#pragma once

#include "env.hpp"
#include "interface.hpp"

#include <cmath>

/** @class Thresholds
 *  @brief Threshold type traits.
 *
 *  @tparam T - The threshold type.
 */
template <typename T>
struct Thresholds
{
    static void fail()
    {
        static_assert(sizeof(Thresholds) == -1, "Unsupported Threshold type");
    }
};

/**@brief Thresholds specialization for warning thresholds. */
template <>
struct Thresholds<WarningObject>
{
    static constexpr InterfaceType type = InterfaceType::WARN;
    static constexpr const char* envLo = "WARNLO";
    static constexpr const char* envHi = "WARNHI";
    static SensorValueType (WarningObject::*const setLo)(SensorValueType);
    static SensorValueType (WarningObject::*const setHi)(SensorValueType);
    static SensorValueType (WarningObject::*const getLo)() const;
    static SensorValueType (WarningObject::*const getHi)() const;
    static bool (WarningObject::*const alarmLo)(bool);
    static bool (WarningObject::*const alarmHi)(bool);
    static bool (WarningObject::*const getalarmLo)() const;
    static bool (WarningObject::*const getalarmHi)() const;
    static void (WarningObject::*const assertLosignal)(SensorValueType);
    static void (WarningObject::*const assertHisignal)(SensorValueType);
    static void (WarningObject::*const deassertLosignal)(SensorValueType);
    static void (WarningObject::*const deassertHisignal)(SensorValueType);
};

/**@brief Thresholds specialization for critical thresholds. */
template <>
struct Thresholds<CriticalObject>
{
    static constexpr InterfaceType type = InterfaceType::CRIT;
    static constexpr const char* envLo = "CRITLO";
    static constexpr const char* envHi = "CRITHI";
    static SensorValueType (CriticalObject::*const setLo)(SensorValueType);
    static SensorValueType (CriticalObject::*const setHi)(SensorValueType);
    static SensorValueType (CriticalObject::*const getLo)() const;
    static SensorValueType (CriticalObject::*const getHi)() const;
    static bool (CriticalObject::*const alarmLo)(bool);
    static bool (CriticalObject::*const alarmHi)(bool);
    static bool (CriticalObject::*const getalarmLo)() const;
    static bool (CriticalObject::*const getalarmHi)() const;
    static void (CriticalObject::*const assertLosignal)(SensorValueType);
    static void (CriticalObject::*const assertHisignal)(SensorValueType);
    static void (CriticalObject::*const deassertLosignal)(SensorValueType);
    static void (CriticalObject::*const deassertHisignal)(SensorValueType);
};

/** @brief checkThresholds
 *
 *  Compare a sensor reading to threshold values and set the
 *  appropriate alarm property if bounds are exceeded.
 *
 *  @tparam T - The threshold type.
 *
 *  @param[in] iface - An sdbusplus server threshold instance.
 *  @param[in] value - The sensor reading to compare to thresholds.
 */
template <typename T>
void checkThresholds(std::any& iface, SensorValueType value)
{
    auto realIface = std::any_cast<std::shared_ptr<T>>(iface);
    auto lo = (*realIface.*Thresholds<T>::getLo)();
    auto hi = (*realIface.*Thresholds<T>::getHi)();
    auto alarmlostate = (*realIface.*Thresholds<T>::getalarmLo)();
    auto alarmhistate = (*realIface.*Thresholds<T>::getalarmHi)();
    (*realIface.*Thresholds<T>::alarmLo)(value <= lo);
    (*realIface.*Thresholds<T>::alarmHi)(value >= hi);
    if (alarmlostate != (value <= lo))
    {
        if (value <= lo)
            (*realIface.*Thresholds<T>::assertLosignal)(value);
        else
            (*realIface.*Thresholds<T>::deassertLosignal)(value);
    }
    if (alarmhistate != (value >= hi))
    {
        if (value >= hi)
            (*realIface.*Thresholds<T>::assertHisignal)(value);
        else
            (*realIface.*Thresholds<T>::deassertHisignal)(value);
    }
}

/** @brief addThreshold
 *
 *  Look for a configured threshold value in the environment and
 *  create an sdbusplus server threshold if found.
 *
 *  @tparam T - The threshold type.
 *
 *  @param[in] sensorType - sensor type, like 'temp'
 *  @param[in] sensorID - sensor ID, like '5'
 *  @param[in] value - The sensor reading.
 *  @param[in] info - The sdbusplus server connection and interfaces.
 */
template <typename T>
auto addThreshold(const std::string& sensorType, const std::string& sensorID,
                  SensorValueType value, ObjectInfo& info, int64_t scale)
{
    auto& objPath = std::get<std::string>(info);
    auto& obj = std::get<InterfaceMap>(info);
    std::shared_ptr<T> iface;

    auto tLo = env::getEnv(Thresholds<T>::envLo, sensorType, sensorID);
    auto tHi = env::getEnv(Thresholds<T>::envHi, sensorType, sensorID);
    if (!tLo.empty() && !tHi.empty())
    {
        static constexpr bool deferSignals = true;
        auto& bus = *std::get<sdbusplus::bus::bus*>(info);

        iface = std::make_shared<T>(bus, objPath.c_str(), deferSignals);
        auto lo = stod(tLo) * std::pow(10, scale);
        auto hi = stod(tHi) * std::pow(10, scale);
        (*iface.*Thresholds<T>::setLo)(lo);
        (*iface.*Thresholds<T>::setHi)(hi);
        auto alarmlostate = (*iface.*Thresholds<T>::getalarmLo)();
        auto alarmhistate = (*iface.*Thresholds<T>::getalarmHi)();
        (*iface.*Thresholds<T>::alarmLo)(value <= lo);
        (*iface.*Thresholds<T>::alarmHi)(value >= hi);
        if (alarmlostate != (value <= lo))
        {
            if (value <= lo)
                (*iface.*Thresholds<T>::assertLosignal)(value);
            else
                (*iface.*Thresholds<T>::deassertLosignal)(value);
        }
        if (alarmhistate != (value >= hi))
        {
            if (value >= hi)
                (*iface.*Thresholds<T>::assertHisignal)(value);
            else
                (*iface.*Thresholds<T>::deassertHisignal)(value);
        }
        auto type = Thresholds<T>::type;
        obj[type] = iface;
    }

    return iface;
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
