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
#include "config.h"

#include "mainloop.hpp"

#include "env.hpp"
#include "fan_pwm.hpp"
#include "fan_speed.hpp"
#include "hwmon.hpp"
#include "hwmonio.hpp"
#include "sensor.hpp"
#include "sensorset.hpp"
#include "sysfs.hpp"
#include "targets.hpp"
#include "thresholds.hpp"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <phosphor-logging/elog-errors.hpp>
#include <sstream>
#include <string>
#include <unordered_set>
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
decltype(
    Thresholds<WarningObject>::alarmLo) Thresholds<WarningObject>::alarmLo =
    &WarningObject::warningAlarmLow;
decltype(
    Thresholds<WarningObject>::alarmHi) Thresholds<WarningObject>::alarmHi =
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
decltype(
    Thresholds<CriticalObject>::alarmLo) Thresholds<CriticalObject>::alarmLo =
    &CriticalObject::criticalAlarmLow;
decltype(
    Thresholds<CriticalObject>::alarmHi) Thresholds<CriticalObject>::alarmHi =
    &CriticalObject::criticalAlarmHigh;

std::string MainLoop::getID(SensorSet::container_t::const_reference sensor)
{
    std::string id;

    /*
     * Check if the value of the MODE_<item><X> env variable for the sensor
     * is set. If it is, then read the from the <item><X>_<mode>
     * file. The name of the DBUS object would be the value of the env
     * variable LABEL_<item><mode value>. If the MODE_<item><X> env variable
     * doesn't exist, then the name of DBUS object is the value of the env
     * variable LABEL_<item><X>.
     *
     * For example, if MODE_temp1 = "label", then code reads the temp1_label
     * file.  If it has a 5 in it, then it will use the following entry to
     * name the object: LABEL_temp5 = "My DBus object name".
     *
     */
    auto mode = env::getEnv("MODE", sensor.first);
    if (!mode.empty())
    {
        id = env::getIndirectID(_hwmonRoot + '/' + _instance + '/', mode,
                                sensor.first);

        if (id.empty())
        {
            return id;
        }
    }

    // Use the ID we looked up above if there was one,
    // otherwise use the standard one.
    id = (id.empty()) ? sensor.first.second : id;

    return id;
}

SensorIdentifiers
    MainLoop::getIdentifiers(SensorSet::container_t::const_reference sensor)
{
    std::string id = getID(sensor);
    std::string label;

    if (!id.empty())
    {
        // Ignore inputs without a label.
        label = env::getEnv("LABEL", sensor.first.first, id);
    }

    return std::make_tuple(std::move(id), std::move(label));
}

/**
 * Reads the environment parameters of a sensor and creates an object with
 * atleast the `Value` interface, otherwise returns without creating the object.
 * If the `Value` interface is successfully created, by reading the sensor's
 * corresponding sysfs file's value, the additional interfaces for the sensor
 * are created and the InterfacesAdded signal is emitted. The object's state
 * data is then returned for sensor state monitoring within the main loop.
 */
optional_ns::optional<ObjectStateData>
    MainLoop::getObject(SensorSet::container_t::const_reference sensor)
{
    auto properties = getIdentifiers(sensor);
    if (std::get<sensorID>(properties).empty() ||
        std::get<sensorLabel>(properties).empty())
    {
        return {};
    }

    hwmon::Attributes attrs;
    if (!hwmon::getAttributes(sensor.first.first, attrs))
    {
        return {};
    }

    auto sensorObj =
        std::make_unique<sensor::Sensor>(sensor.first, ioAccess, _devPath);

    // Get list of return codes for removing sensors on device
    auto devRmRCs = env::getEnv("REMOVERCS");
    // Add sensor removal return codes defined at the device level
    sensorObj->addRemoveRCs(devRmRCs);

    std::string objectPath{_root};
    objectPath.append(1, '/');
    objectPath.append(hwmon::getNamespace(attrs));
    objectPath.append(1, '/');
    objectPath.append(std::get<sensorLabel>(properties));

    ObjectInfo info(&_bus, std::move(objectPath), Object());
    RetryIO retryIO(hwmonio::retries, hwmonio::delay);
    if (rmSensors.find(sensor.first) != rmSensors.end())
    {
        // When adding a sensor that was purposely removed,
        // don't retry on errors when reading its value
        std::get<size_t>(retryIO) = 0;
    }
    auto valueInterface = static_cast<std::shared_ptr<ValueObject>>(nullptr);
    try
    {
        // Add status interface based on _fault file being present
        sensorObj->addStatus(info);
        valueInterface = sensorObj->addValue(retryIO, info);
    }
    catch (const std::system_error& e)
    {
        auto file =
            sysfs::make_sysfs_path(ioAccess.path(), sensor.first.first,
                                   sensor.first.second, hwmon::entry::cinput);
#ifndef REMOVE_ON_FAIL
        // Check sensorAdjusts for sensor removal RCs
        auto& sAdjusts = sensorObj->getAdjusts();
        if (sAdjusts.rmRCs.count(e.code().value()) > 0)
        {
            // Return code found in sensor return code removal list
            if (rmSensors.find(sensor.first) == rmSensors.end())
            {
                // Trace for sensor not already removed from dbus
                log<level::INFO>("Sensor not added to dbus for read fail",
                                 entry("FILE=%s", file.c_str()),
                                 entry("RC=%d", e.code().value()));
                rmSensors[std::move(sensor.first)] = std::move(sensor.second);
            }
            return {};
        }
#endif
        using namespace sdbusplus::xyz::openbmc_project::Sensor::Device::Error;
        report<ReadFailure>(
            xyz::openbmc_project::Sensor::Device::ReadFailure::CALLOUT_ERRNO(
                e.code().value()),
            xyz::openbmc_project::Sensor::Device::ReadFailure::
                CALLOUT_DEVICE_PATH(_devPath.c_str()));

        log<level::INFO>("Logging failing sysfs file",
                         entry("FILE=%s", file.c_str()));
#ifdef REMOVE_ON_FAIL
        return {}; /* skip adding this sensor for now. */
#else
        exit(EXIT_FAILURE);
#endif
    }
    auto sensorValue = valueInterface->value();
    int64_t scale = 0;
    // scale the thresholds only if we're using doubles
    if constexpr (std::is_same<SensorValueType, double>::value)
    {
        scale = sensorObj->getScale();
    }
    addThreshold<WarningObject>(sensor.first.first,
                                std::get<sensorID>(properties), sensorValue,
                                info, scale);
    addThreshold<CriticalObject>(sensor.first.first,
                                 std::get<sensorID>(properties), sensorValue,
                                 info, scale);

    auto target =
        addTarget<hwmon::FanSpeed>(sensor.first, ioAccess, _devPath, info);
    if (target)
    {
        target->enable();
    }
    addTarget<hwmon::FanPwm>(sensor.first, ioAccess, _devPath, info);

    // All the interfaces have been created.  Go ahead
    // and emit InterfacesAdded.
    valueInterface->emit_object_added();

    // Save sensor object specifications
    sensorObjects[sensor.first] = std::move(sensorObj);

    return std::make_pair(std::move(std::get<sensorLabel>(properties)),
                          std::move(info));
}

MainLoop::MainLoop(sdbusplus::bus::bus&& bus, const std::string& param,
                   const std::string& path, const std::string& devPath,
                   const char* prefix, const char* root) :
    _bus(std::move(bus)),
    _manager(_bus, root), _pathParam(param), _hwmonRoot(), _instance(),
    _devPath(devPath), _prefix(prefix), _root(root), state(), ioAccess(path)
{
    // Strip off any trailing slashes.
    std::string p = path;
    while (!p.empty() && p.back() == '/')
    {
        p.pop_back();
    }

    // Given the furthest right /, set instance to
    // the basename, and hwmonRoot to the leading path.
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
    timer->state(phosphor::hwmon::timer::OFF);
    sd_event_exit(loop, 0);
    loop = nullptr;
}

void MainLoop::run()
{
    init();

    sd_event_default(&loop);

    std::function<void()> callback(std::bind(&MainLoop::read, this));
    try
    {
        timer = std::make_unique<phosphor::hwmon::Timer>(
            loop, callback, std::chrono::microseconds(_interval),
            phosphor::hwmon::timer::ON);

        // TODO: Issue#6 - Optionally look at polling interval sysfs entry.

        // TODO: Issue#7 - Should probably periodically check the SensorSet
        //       for new entries.

        _bus.attach_event(loop, SD_EVENT_PRIORITY_IMPORTANT);
        sd_event_loop(loop);
    }
    catch (const std::system_error& e)
    {
        log<level::ERR>("Error in sysfs polling loop",
                        entry("ERROR=%s", e.what()));
        throw;
    }
}

void MainLoop::init()
{
    // Check sysfs for available sensors.
    auto sensors = std::make_unique<SensorSet>(_hwmonRoot + '/' + _instance);

    for (auto& i : *sensors)
    {
        auto object = getObject(i);
        if (object)
        {
            // Construct the SensorSet value
            // std::tuple<SensorSet::mapped_type,
            //            std::string(Sensor Label),
            //            ObjectInfo>
            auto value =
                std::make_tuple(std::move(i.second), std::move((*object).first),
                                std::move((*object).second));

            state[std::move(i.first)] = std::move(value);
        }
    }

    /* If there are no sensors specified by labels, exit. */
    if (0 == state.size())
    {
        exit(0);
    }

    {
        std::stringstream ss;
        ss << _prefix << "-"
           << std::to_string(std::hash<std::string>{}(_devPath + _pathParam))
           << ".Hwmon1";

        _bus.request_name(ss.str().c_str());
    }

    {
        auto interval = env::getEnv("INTERVAL");
        if (!interval.empty())
        {
            _interval = std::strtoull(interval.c_str(), NULL, 10);
        }
    }
}

void MainLoop::read()
{
    // TODO: Issue#3 - Need to make calls to the dbus sensor cache here to
    //       ensure the objects all exist?

    // Iterate through all the sensors.
    for (auto& i : state)
    {
        auto& attrs = std::get<0>(i.second);
        if (attrs.find(hwmon::entry::input) != attrs.end())
        {
            // Read value from sensor.
            std::string input = hwmon::entry::cinput;
            if (i.first.first == "pwm")
            {
                input = "";
            }

            try
            {
                int64_t value;
                auto& objInfo = std::get<ObjectInfo>(i.second);
                auto& obj = std::get<Object>(objInfo);

                auto it = obj.find(InterfaceType::STATUS);
                if (it != obj.end())
                {
                    auto fault = ioAccess.read(
                        i.first.first, i.first.second, hwmon::entry::fault,
                        hwmonio::retries, hwmonio::delay);
                    auto statusIface = std::experimental::any_cast<
                        std::shared_ptr<StatusObject>>(it->second);
                    if (!statusIface->functional((fault == 0) ? true : false))
                    {
                        continue;
                    }
                }

                // Retry for up to a second if device is busy
                // or has a transient error.
                std::unique_ptr<sensor::Sensor>& sensor =
                    sensorObjects[i.first];

                sensor->unlockGpio();

                value = ioAccess.read(i.first.first, i.first.second, input,
                                      hwmonio::retries, hwmonio::delay);

                sensor->lockGpio();

                value = sensor->adjustValue(value);

                for (auto& iface : obj)
                {
                    auto valueIface = std::shared_ptr<ValueObject>();
                    auto warnIface = std::shared_ptr<WarningObject>();
                    auto critIface = std::shared_ptr<CriticalObject>();

                    switch (iface.first)
                    {
                        case InterfaceType::VALUE:
                            valueIface = std::experimental::any_cast<
                                std::shared_ptr<ValueObject>>(iface.second);
                            valueIface->value(value);
                            break;
                        case InterfaceType::WARN:
                            checkThresholds<WarningObject>(iface.second, value);
                            break;
                        case InterfaceType::CRIT:
                            checkThresholds<CriticalObject>(iface.second,
                                                            value);
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (const std::system_error& e)
            {
                auto file = sysfs::make_sysfs_path(
                    ioAccess.path(), i.first.first, i.first.second,
                    hwmon::entry::cinput);
#ifndef REMOVE_ON_FAIL
                // Check sensorAdjusts for sensor removal RCs
                auto& sAdjusts = sensorObjects[i.first]->getAdjusts();
                if (sAdjusts.rmRCs.count(e.code().value()) > 0)
                {
                    // Return code found in sensor return code removal list
                    if (rmSensors.find(i.first) == rmSensors.end())
                    {
                        // Trace for sensor not already removed from dbus
                        log<level::INFO>(
                            "Remove sensor from dbus for read fail",
                            entry("FILE=%s", file.c_str()),
                            entry("RC=%d", e.code().value()));
                        // Mark this sensor to be removed from dbus
                        rmSensors[i.first] = std::get<0>(i.second);
                    }
                    continue;
                }
#endif
                using namespace sdbusplus::xyz::openbmc_project::Sensor::
                    Device::Error;
                report<ReadFailure>(
                    xyz::openbmc_project::Sensor::Device::ReadFailure::
                        CALLOUT_ERRNO(e.code().value()),
                    xyz::openbmc_project::Sensor::Device::ReadFailure::
                        CALLOUT_DEVICE_PATH(_devPath.c_str()));

                log<level::INFO>("Logging failing sysfs file",
                                 entry("FILE=%s", file.c_str()));

#ifdef REMOVE_ON_FAIL
                rmSensors[i.first] = std::get<0>(i.second);
#else
                exit(EXIT_FAILURE);
#endif
            }
        }
    }

    // Remove any sensors marked for removal
    for (auto& i : rmSensors)
    {
        state.erase(i.first);
    }

#ifndef REMOVE_ON_FAIL
    // Attempt to add any sensors that were removed
    auto it = rmSensors.begin();
    while (it != rmSensors.end())
    {
        if (state.find(it->first) == state.end())
        {
            SensorSet::container_t::value_type ssValueType =
                std::make_pair(it->first, it->second);
            auto object = getObject(ssValueType);
            if (object)
            {
                // Construct the SensorSet value
                // std::tuple<SensorSet::mapped_type,
                //            std::string(Sensor Label),
                //            ObjectInfo>
                auto value = std::make_tuple(std::move(ssValueType.second),
                                             std::move((*object).first),
                                             std::move((*object).second));

                state[std::move(ssValueType.first)] = std::move(value);

                // Sensor object added, erase entry from removal list
                auto file = sysfs::make_sysfs_path(
                    ioAccess.path(), it->first.first, it->first.second,
                    hwmon::entry::cinput);
                log<level::INFO>("Added sensor to dbus after successful read",
                                 entry("FILE=%s", file.c_str()));
                it = rmSensors.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            // Sanity check to remove sensors that were re-added
            it = rmSensors.erase(it);
        }
    }
#endif
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
