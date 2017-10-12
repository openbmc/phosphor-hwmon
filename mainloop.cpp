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
#include <cstdlib>
#include <string>

#include <phosphor-logging/elog-errors.hpp>
#include "config.h"
#include "sensorset.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"
#include "mainloop.hpp"
#include "env.hpp"
#include "thresholds.hpp"
#include "targets.hpp"
#include "fan_speed.hpp"

#include <xyz/openbmc_project/Sensor/Device/error.hpp>

using namespace phosphor::logging;

// Initialization for Warning Objects
decltype(Thresholds<WarningObject>::setLo) Thresholds<WarningObject>::setLo =
    &WarningObject::warningLow;
decltype(Thresholds<WarningObject>::setHi) Thresholds<WarningObject>::setHi =
    &WarningObject::warningHigh;
decltype(Thresholds<WarningObject>::getLo) Thresholds<WarningObject>::getLo =
    &WarningObject::warningLow;
decltype(Thresholds<WarningObject>::getHi) Thresholds<WarningObject>::getHi =
    &WarningObject::warningHigh;
decltype(Thresholds<WarningObject>::alarmLo) Thresholds<WarningObject>::alarmLo =
    &WarningObject::warningAlarmLow;
decltype(Thresholds<WarningObject>::alarmHi) Thresholds<WarningObject>::alarmHi =
    &WarningObject::warningAlarmHigh;

// Initialization for Critical Objects
decltype(Thresholds<CriticalObject>::setLo) Thresholds<CriticalObject>::setLo =
    &CriticalObject::criticalLow;
decltype(Thresholds<CriticalObject>::setHi) Thresholds<CriticalObject>::setHi =
    &CriticalObject::criticalHigh;
decltype(Thresholds<CriticalObject>::getLo) Thresholds<CriticalObject>::getLo =
    &CriticalObject::criticalLow;
decltype(Thresholds<CriticalObject>::getHi) Thresholds<CriticalObject>::getHi =
    &CriticalObject::criticalHigh;
decltype(Thresholds<CriticalObject>::alarmLo) Thresholds<CriticalObject>::alarmLo =
    &CriticalObject::criticalAlarmLow;
decltype(Thresholds<CriticalObject>::alarmHi) Thresholds<CriticalObject>::alarmHi =
    &CriticalObject::criticalAlarmHigh;

// The gain and offset to adjust a value
struct valueAdjust
{
    double gain = 1.0;
    int offset = 0;
};

// Store the valueAdjust for sensors
std::map<SensorSet::key_type, valueAdjust> sensorAdjusts;

static constexpr auto typeAttrMap =
{
    // 1 - hwmon class
    // 2 - unit
    // 3 - sysfs scaling factor
    std::make_tuple(
        hwmon::type::ctemp,
        ValueInterface::Unit::DegreesC,
        -3,
        "temperature"),
    std::make_tuple(
        hwmon::type::cfan,
        ValueInterface::Unit::RPMS,
        0,
        "fan_tach"),
    std::make_tuple(
        hwmon::type::cvolt,
        ValueInterface::Unit::Volts,
        -3,
        "voltage"),
    std::make_tuple(
        hwmon::type::ccurr,
        ValueInterface::Unit::Amperes,
        -3,
        "current"),
    std::make_tuple(
        hwmon::type::cenergy,
        ValueInterface::Unit::Joules,
        -6,
        "energy"),
    std::make_tuple(
        hwmon::type::cpower,
        ValueInterface::Unit::Watts,
        -6,
        "power"),
};

auto getHwmonType(decltype(typeAttrMap)::const_reference attrs)
{
    return std::get<0>(attrs);
}

auto getUnit(decltype(typeAttrMap)::const_reference attrs)
{
    return std::get<1>(attrs);
}

auto getScale(decltype(typeAttrMap)::const_reference attrs)
{
    return std::get<2>(attrs);
}

auto getNamespace(decltype(typeAttrMap)::const_reference attrs)
{
    return std::get<3>(attrs);
}

using AttributeIterator = decltype(*typeAttrMap.begin());
using Attributes
    = std::remove_cv<std::remove_reference<AttributeIterator>::type>::type;

auto getAttributes(const std::string& type, Attributes& attributes)
{
    // *INDENT-OFF*
    auto a = std::find_if(
                typeAttrMap.begin(),
                typeAttrMap.end(),
                [&](const auto & e)
                {
                   return type == getHwmonType(e);
                });
    // *INDENT-ON*

    if (a == typeAttrMap.end())
    {
        return false;
    }

    attributes = *a;
    return true;
}

int adjustValue(const SensorSet::key_type& sensor, int value)
{
    const auto& it = sensorAdjusts.find(sensor);
    if (it != sensorAdjusts.end())
    {
        // Adjust based on gain and offset
        value = static_cast<decltype(value)>(
                    static_cast<double>(value) * it->second.gain
                        + it->second.offset);
    }
    return value;
}

auto addValue(const SensorSet::key_type& sensor,
              const std::string& devPath,
              sysfs::hwmonio::HwmonIO& ioAccess,
              ObjectInfo& info)
{
    static constexpr bool deferSignals = true;

    // Get the initial value for the value interface.
    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& obj = std::get<Object>(info);
    auto& objPath = std::get<std::string>(info);

    auto val = 0;
    try
    {
        // Retry for up to a second if device is busy
        // or has a transient error.
        val = ioAccess.read(
                sensor.first,
                sensor.second,
                hwmon::entry::cinput,
                sysfs::hwmonio::retries,
                sysfs::hwmonio::delay);
    }
    catch (const std::system_error& e)
    {
        using namespace sdbusplus::xyz::openbmc_project::Sensor::Device::Error;
        report<ReadFailure>(
            xyz::openbmc_project::Sensor::Device::
                ReadFailure::CALLOUT_ERRNO(e.code().value()),
            xyz::openbmc_project::Sensor::Device::
                ReadFailure::CALLOUT_DEVICE_PATH(devPath.c_str()));

        auto file = sysfs::make_sysfs_path(
                ioAccess.path(),
                sensor.first,
                sensor.second,
                hwmon::entry::cinput);

        log<level::INFO>("Logging failing sysfs file",
                entry("FILE=%s", file.c_str()));

        return static_cast<std::shared_ptr<ValueObject>>(nullptr);
    }

    auto gain = getEnv("GAIN", sensor);
    if (!gain.empty())
    {
        sensorAdjusts[sensor].gain = std::stod(gain);
    }

    auto offset = getEnv("OFFSET", sensor);
    if (!offset.empty())
    {
        sensorAdjusts[sensor].offset = std::stoi(offset);
    }

    val = adjustValue(sensor, val);

    auto iface = std::make_shared<ValueObject>(bus, objPath.c_str(), deferSignals);
    iface->value(val);

    Attributes attrs;
    if (getAttributes(sensor.first, attrs))
    {
        iface->unit(getUnit(attrs));
        iface->scale(getScale(attrs));
    }

    obj[InterfaceType::VALUE] = iface;
    return iface;
}

MainLoop::MainLoop(
    sdbusplus::bus::bus&& bus,
    const std::string& path,
    const std::string& devPath,
    const char* prefix,
    const char* root)
    : _bus(std::move(bus)),
      _manager(_bus, root),
      _shutdown(false),
      _hwmonRoot(),
      _instance(),
      _devPath(devPath),
      _prefix(prefix),
      _root(root),
      state(),
      ioAccess(path)
{
    std::string p = path;
    while (!p.empty() && p.back() == '/')
    {
        p.pop_back();
    }

    auto n = p.rfind('/');
    if (n != std::string::npos)
    {
        _instance.assign(p.substr(n + 1));
        _hwmonRoot.assign(p.substr(0, n));
    }

    assert(!_instance.empty());
    assert(!_hwmonRoot.empty());
}

void MainLoop::shutdown() noexcept
{
    _shutdown = true;
}

void MainLoop::run()
{
    // Check sysfs for available sensors.
    auto sensors = std::make_unique<SensorSet>(_hwmonRoot + '/' + _instance);

    for (auto& i : *sensors)
    {
        std::string label;
        std::string id;

        /*
         * Check if the value of the MODE_<item><X> env variable for the sensor
         * is "label", then read the sensor number from the <item><X>_label
         * file. The name of the DBUS object would be the value of the env
         * variable LABEL_<item><sensorNum>. If the MODE_<item><X> env variable
         * doesn't exist, then the name of DBUS object is the value of the env
         * variable LABEL_<item><X>.
         */
        auto mode = getEnv("MODE", i.first);
        if (!mode.compare(hwmon::entry::label))
        {
            id = getIndirectID(
                    _hwmonRoot + '/' + _instance + '/', i.first);

            if (id.empty())
            {
                continue;
            }
        }

        //In this loop, use the ID we looked up above if
        //there was one, otherwise use the standard one.
        id = (id.empty()) ? i.first.second : id;

        // Ignore inputs without a label.
        label = getEnv("LABEL", i.first.first, id);
        if (label.empty())
        {
            continue;
        }

        Attributes attrs;
        if (!getAttributes(i.first.first, attrs))
        {
            continue;
        }

        std::string objectPath{_root};
        objectPath.append(1, '/');
        objectPath.append(getNamespace(attrs));
        objectPath.append(1, '/');
        objectPath.append(label);

        ObjectInfo info(&_bus, std::move(objectPath), Object());
        auto valueInterface = addValue(i.first, _devPath, ioAccess, info);
        if (!valueInterface)
        {
#ifdef REMOVE_ON_FAIL
            continue; /* skip adding this sensor for now. */
#else
            exit(EXIT_FAILURE);
#endif
        }
        auto sensorValue = valueInterface->value();
        addThreshold<WarningObject>(i.first, sensorValue, info);
        addThreshold<CriticalObject>(i.first, sensorValue, info);

        auto target = addTarget<hwmon::FanSpeed>(
                i.first, ioAccess.path(), _devPath, info);

        if (target)
        {
            target->enable();
        }

        // All the interfaces have been created.  Go ahead
        // and emit InterfacesAdded.
        valueInterface->emit_object_added();

        auto value = std::make_tuple(
                         std::move(i.second),
                         std::move(label),
                         std::move(info));

        state[std::move(i.first)] = std::move(value);
    }

    /* If there are no sensors specified by labels, exit. */
    if (0 == state.size())
    {
        return;
    }

    {
        std::string busname{_prefix};
        busname.append(1, '.');
        busname.append(_instance);
        _bus.request_name(busname.c_str());
    }

    {
        auto interval = getenv("INTERVAL");
        if (interval)
        {
            _interval = strtoull(interval, NULL, 10);
        }
    }

    // TODO: Issue#3 - Need to make calls to the dbus sensor cache here to
    //       ensure the objects all exist?

    // Polling loop.
    while (!_shutdown)
    {
#ifdef REMOVE_ON_FAIL
        std::vector<SensorSet::key_type> destroy;
#endif
        // Iterate through all the sensors.
        for (auto& i : state)
        {
            auto& attrs = std::get<0>(i.second);
            if (attrs.find(hwmon::entry::input) != attrs.end())
            {
                // Read value from sensor.
                int value;
                try
                {
                    // Retry for up to a second if device is busy
                    // or has a transient error.

                    value = ioAccess.read(
                            i.first.first,
                            i.first.second,
                            hwmon::entry::cinput,
                            sysfs::hwmonio::retries,
                            sysfs::hwmonio::delay);

                    value = adjustValue(i.first, value);

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
                catch (const std::system_error& e)
                {
                    using namespace sdbusplus::xyz::openbmc_project::
                        Sensor::Device::Error;
                    report<ReadFailure>(
                            xyz::openbmc_project::Sensor::Device::
                                ReadFailure::CALLOUT_ERRNO(e.code().value()),
                            xyz::openbmc_project::Sensor::Device::
                                ReadFailure::CALLOUT_DEVICE_PATH(
                                    _devPath.c_str()));

                    auto file = sysfs::make_sysfs_path(
                            ioAccess.path(),
                            i.first.first,
                            i.first.second,
                            hwmon::entry::cinput);

                    log<level::INFO>("Logging failing sysfs file",
                            entry("FILE=%s", file.c_str()));

#ifdef REMOVE_ON_FAIL
                    destroy.push_back(i.first);
#else
                    exit(EXIT_FAILURE);
#endif
                }
            }
        }

#ifdef REMOVE_ON_FAIL
        for (auto& i : destroy)
        {
            state.erase(i);
        }
#endif

        // Respond to DBus
        _bus.process_discard();

        // Sleep until next interval.
        // TODO: Issue#6 - Optionally look at polling interval sysfs entry.
        _bus.wait(_interval);

        // TODO: Issue#7 - Should probably periodically check the SensorSet
        //       for new entries.
    }
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
