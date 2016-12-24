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
#include <algorithm>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <utility>
#include <experimental/any>
#include <vector>
#include "sensorset.hpp"
#include "sensorcache.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"
#include "mainloop.hpp"
#include "sensors.hpp"
#include "interface.hpp"

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
      _root(root)
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
    static constexpr auto unitMap =
    {
        std::make_pair("temp", ValueInterface::Unit::DegreesC),
        std::make_pair("fan", ValueInterface::Unit::RPMS),
        std::make_pair("in", ValueInterface::Unit::Volts),
        std::make_pair("power", ValueInterface::Unit::Volts),
    };

    // Check sysfs for available sensors.
    auto sensors = std::make_unique<SensorSet>(_path);
    auto sensor_cache = std::make_unique<SensorCache>();
    auto lib = libsensors::loadDefault();
    std::vector<std::vector<std::experimental::any>> refs;
    std::vector<std::pair<
    libsensors::Attribute,
               std::experimental::any>> pollMe;

    {
        auto chips = lib.chips();

        // Narrow the scope to just our hwmon instance.
        auto chip = std::find_if(
                        chips.begin(),
                        chips.end(),
                        [&](const auto & e)
        {
            return _path == e.path();
        });

        if (chip == chips.cend())
        {
            std::string msg
            {
                "Did not find any chips in hwmon instance: " + _path};
            throw std::runtime_error(msg);
        }

        for (auto& sensor : chip->sensors())
        {
            // Find supported interfaces and
            // create a dbus object.
            auto sensorType = sensor.type();

            std::string objectPath{_root};
            objectPath.append("/");
            objectPath.append(sensorType);
            objectPath.append("/");
            objectPath.append(sensor.label());
            std::vector<std::experimental::any> interfaces;

            auto attrs = sensor.attributes();

            // Create a value interface if the input
            // attribute exists.
            auto input = std::find_if(
                             attrs.begin(),
                             attrs.end(),
                             [](const auto & attr)
            {
                return attr.type() == "input";
            });

            if (input != attrs.cend())
            {
                // Create the interface.
                interfaces.emplace_back(
                    std::make_shared<ValueObject>(
                        _bus, objectPath.c_str()));

                // Set the initial values.
                auto& iface = *std::experimental::any_cast<std::shared_ptr<ValueObject>>
                              (interfaces.back());
                iface.value(input->read());
                iface.scale(input->scale());

                const auto& unit = std::find_if(
                                       unitMap.begin(),
                                       unitMap.end(),
                                       [&](const auto & mapEntry)
                {
                    return sensorType == mapEntry.first;
                });
                if (unit != unitMap.end())
                {
                    iface.unit(unit->second);
                }

                // Input attributes are polled, so add
                // to the polling loop.
                pollMe.emplace_back(
                    *input,
                    std::experimental::any(
                        interfaces.back()));
            }

            // Create the dbus object.
            if (!interfaces.empty())
            {
                refs.emplace_back(std::move(interfaces));
            }
        }
    }

    {
        struct Free
        {
            void operator()(char* ptr) const
            {
                free(ptr);
            }
        };

        auto copy = std::unique_ptr<char, Free>(strdup(_path.c_str()));
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
        for (auto& i : *sensors)
        {
            if (i.second.find(hwmon::entry::input) != i.second.end())
            {
                // Read value from sensor.
                int value = 0;
                read_sysfs(make_sysfs_path(_path,
                                           i.first.first, i.first.second,
                                           hwmon::entry::input),
                           value);

                // Update sensor cache.
                if (sensor_cache->update(i.first, value))
                {
                    // TODO: Issue#4 - dbus event here.
                    std::cout << i.first.first << i.first.second << " = "
                              << value << std::endl;
                }
            }
        }

        for (auto& i : pollMe)
        {
            auto attrType = i.first.type();
            auto val = i.first.read();

            if (attrType == "input")
            {
                auto& iface = *std::experimental::any_cast <
                              std::shared_ptr<ValueObject >> (i.second);
                iface.value(val);
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
