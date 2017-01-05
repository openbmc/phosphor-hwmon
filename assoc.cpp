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
#include <string>
#include <cstdlib>
#include <algorithm>
#include "assoc.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"

static constexpr auto typeAttrMap =
{
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

ObjectSet::ObjectSet(
    const std::string& dbusRoot,
    const std::string& sysfsRoot,
    sdbusplus::bus::bus& bus,
    SensorSet&& o)
{
    for (auto& i : o)
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
                             sysfsRoot,
                             i.first.first,
                             i.first.second,
                             hwmon::entry::input);
        int val = 0;
        read_sysfs(sysfsPath, val);

        Object o;
        std::string objectPath{dbusRoot};

        objectPath.append("/");
        objectPath.append(i.first.first);
        objectPath.append("/");
        objectPath.append(label);

        auto iface = std::make_shared<ValueObject>(bus, objectPath.c_str());
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
                         std::move(o));
        container[std::move(i.first)] = std::move(value);
    }
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
