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
#include <algorithm>
#include "sensorset.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"
#include "mainloop.hpp"
#include "util.hpp"

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
        // Ignore inputs without a label.
        std::string envKey = "LABEL_" + i.first.first + i.first.second;
        std::string label;

        auto env = getenv(envKey.c_str());

        if (env)
        {
            label.assign(env);
        }

        if (label.empty())
        {
            continue;
        }

        // Get the initial value for the value interface.
        auto sysfsPath = make_sysfs_path(
                             _path,
                             i.first.first,
                             i.first.second,
                             hwmon::entry::input);
        int val = 0;
        read_sysfs(sysfsPath, val);

        std::string objectPath{_root};
        objectPath.append("/");
        objectPath.append(i.first.first);
        objectPath.append("/");
        objectPath.append(label);

        ObjectInfo info(&_bus, std::move(objectPath), Object());
        auto& o = std::get<Object>(info);

        auto iface = std::make_shared<ValueObject>(_bus, objectPath.c_str());
        iface->value(val);

        const auto& attrs = std::find_if(
                                typeAttrMap.begin(),
                                typeAttrMap.end(),
                                [&](const auto & e)
        {
            return i.first.first == std::get<0>(e);
        });
        if (attrs != typeAttrMap.end())
        {
            iface->unit(std::get<1>(*attrs));
            iface->scale(std::get<2>(*attrs));
        }

        o.emplace(InterfaceType::VALUE, iface);

        auto value = std::make_tuple(
                         std::move(i.second),
                         std::move(label),
                         std::move(info));

        state[std::move(i.first)] = std::move(value);
    }

    {
        auto copy = std::unique_ptr<char, Free<char>>(strdup(_path.c_str()));
        auto busname = static_cast<std::string>(basename(copy.get()));
        busname.insert(0, ".");
        busname.insert(0, _prefix);
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
                auto iface = obj.find(InterfaceType::VALUE);

                if (iface != obj.end())
                {
                    auto realIface = std::experimental::any_cast<std::shared_ptr<ValueObject>>
                                     (iface->second);
                    realIface->value(value);
                }
            }
        }

        // Respond to DBus
        _bus.process_discard();

        // Sleep until next interval.
        // TODO: Issue#5 - Make this configurable.
        // TODO: Issue#6 - Optionally look at polling interval sysfs entry.
        _bus.wait(10000000);

        // TODO: Issue#7 - Should probably periodically check the SensorSet
        //       for new entries.
    }
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
