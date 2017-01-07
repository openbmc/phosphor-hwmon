/**
 * Copyright © 2016 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include "sensorset.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"
#include "mainloop.hpp"
#include "util.hpp"
#include "env.hpp"
#include "thresholds.hpp"

using namespace std::literals::chrono_literals;

static constexpr auto typeAttrMap =
{
    // 1 - hwmon class
    // 2 - unit
    // 3 - sysfs scaling factor
    std::make_tuple(
        hwmon::type::ctemp,
        ValueInterface::Unit::DegreesC,
        -3),
    std::make_tuple(
        hwmon::type::cfan,
        ValueInterface::Unit::RPMS,
        0),
    std::make_tuple(
        hwmon::type::cvolt,
        ValueInterface::Unit::Volts,
        -3),
};

using AttributeIterator = decltype(*typeAttrMap.begin());
using Attributes
    = std::remove_cv<std::remove_reference<AttributeIterator>::type>::type;

auto getAttributes(const std::string& type, Attributes& attributes)
{
    auto a = std::find_if(
                 typeAttrMap.begin(),
                 typeAttrMap.end(),
                 [&](const auto & e)
    {
        return type == std::get<0>(e);
    });
    if (a == typeAttrMap.end())
    {
        return false;
    }

    attributes = *a;
    return true;
}

auto addValue(const SensorSet::key_type& sensor,
              const std::string& sysfsRoot, ObjectInfo& info)
{
    // Get the initial value for the value interface.
    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& obj = std::get<Object>(info);
    auto& objPath = std::get<std::string>(info);

    auto sysfsPath = make_sysfs_path(
                         sysfsRoot,
                         sensor.first,
                         sensor.second,
                         hwmon::entry::input);
    int val = 0;
    read_sysfs(sysfsPath, val);

    auto iface = std::make_shared<ValueObject>(bus, objPath.c_str());
    iface->value(val);

    Attributes attrs;
    if (getAttributes(sensor.first, attrs))
    {
        iface->unit(std::get<1>(attrs));
        iface->scale(std::get<2>(attrs));
    }

    obj[InterfaceType::VALUE] = iface;
    return iface;
}

MainLoop::MainLoop(
    sdbusplus::bus::bus&& bus,
    const std::string& path,
    const char* prefix,
    const char* root)
    : _bus(std::move(bus)),
      _manager(sdbusplus::server::manager::manager(_bus, root)),
      _shutdown(false),
      _path(path),
      _prefix(prefix),
      _root(root),
      state()
{
    if (_path.back() == '/')
    {
        _path.pop_back();
    }
}

void MainLoop::shutdown() noexcept
{
    _shutdown = true;
}

void MainLoop::run()
{
    // Check sysfs for available sensors.
    auto sensors = std::make_unique<SensorSet>(_path);

    for (auto& i : *sensors)
    {
        // Get sensor configuration from the environment.

        // Ignore inputs without a label.
        auto label = getEnv("LABEL", i.first);
        if (label.empty())
        {
            continue;
        }

        std::string objectPath{_root};
        objectPath.append("/");
        objectPath.append(i.first.first);
        objectPath.append("/");
        objectPath.append(label);

        ObjectInfo info(&_bus, std::move(objectPath), Object());
        auto valueInterface = addValue(i.first, _path, info);
        auto sensorValue = valueInterface->value();
        addThreshold<WarningObject>(i.first, sensorValue, info);
        addThreshold<CriticalObject>(i.first, sensorValue, info);

        auto value = std::make_tuple(
                         std::move(i.second),
                         std::move(label),
                         std::move(info));

        state[std::move(i.first)] = std::move(value);
    }

    {
        auto copy = std::unique_ptr<char, Free<char>>(strdup(_path.c_str()));
        auto busname = std::string(_prefix) + '.' + basename(copy.get());
        _bus.request_name(busname.c_str());
    }

    // TODO: Issue#3 - Need to make calls to the dbus sensor cache here to
    //       ensure the objects all exist?

    // Polling loop.
    while (!_shutdown)
    {
        // Iterate through all the sensors.
        for (auto& i : state)
        {
            auto& attrs = std::get<0>(i.second);
            if (attrs.find(hwmon::entry::input) != attrs.end())
            {
                // Read value from sensor.
                int value = 0;
                read_sysfs(make_sysfs_path(_path,
                                           i.first.first, i.first.second,
                                           hwmon::entry::input),
                           value);

                auto& objInfo = std::get<ObjectInfo>(i.second);
                auto& obj = std::get<Object>(objInfo);

                for (auto& iface : obj)
                {
                    auto valueIface = std::shared_ptr<ValueObject>();
                    auto warnIface = std::shared_ptr<WarningObject>();
                    auto critIface = std::shared_ptr<CriticalObject>();

                    switch (iface.first)
                    {
                        case InterfaceType::VALUE:
                            valueIface = std::experimental::any_cast<std::shared_ptr<ValueObject>>
                                         (iface.second);
                            valueIface->value(value);
                            break;
                        case InterfaceType::WARN:
                            checkThresholds<WarningObject>(iface.second, value);
                            break;
                        case InterfaceType::CRIT:
                            checkThresholds<CriticalObject>(iface.second, value);
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        // Respond to DBus
        _bus.process_discard();

        // Sleep until next interval.
        // TODO: Issue#5 - Make this configurable.
        // TODO: Issue#6 - Optionally look at polling interval sysfs entry.
        _bus.wait((1000000us).count());

        // TODO: Issue#7 - Should probably periodically check the SensorSet
        //       for new entries.
    }
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
