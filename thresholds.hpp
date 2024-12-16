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
    static SensorValueType (WarningObject::* const setLo)(SensorValueType);
    static SensorValueType (WarningObject::* const setHi)(SensorValueType);
    static SensorValueType (WarningObject::* const getLo)() const;
    static SensorValueType (WarningObject::* const getHi)() const;
    static bool (WarningObject::* const alarmLo)(bool);
    static bool (WarningObject::* const alarmHi)(bool);
    static bool (WarningObject::* const getAlarmLow)() const;
    static bool (WarningObject::* const getAlarmHigh)() const;
    static void (WarningObject::* const assertLowSignal)(SensorValueType);
    static void (WarningObject::* const assertHighSignal)(SensorValueType);
    static void (WarningObject::* const deassertLowSignal)(SensorValueType);
    static void (WarningObject::* const deassertHighSignal)(SensorValueType);
};

/**@brief Thresholds specialization for critical thresholds. */
template <>
struct Thresholds<CriticalObject>
{
    static constexpr InterfaceType type = InterfaceType::CRIT;
    static constexpr const char* envLo = "CRITLO";
    static constexpr const char* envHi = "CRITHI";
    static SensorValueType (CriticalObject::* const setLo)(SensorValueType);
    static SensorValueType (CriticalObject::* const setHi)(SensorValueType);
    static SensorValueType (CriticalObject::* const getLo)() const;
    static SensorValueType (CriticalObject::* const getHi)() const;
    static bool (CriticalObject::* const alarmLo)(bool);
    static bool (CriticalObject::* const alarmHi)(bool);
    static bool (CriticalObject::* const getAlarmLow)() const;
    static bool (CriticalObject::* const getAlarmHigh)() const;
    static void (CriticalObject::* const assertLowSignal)(SensorValueType);
    static void (CriticalObject::* const assertHighSignal)(SensorValueType);
    static void (CriticalObject::* const deassertLowSignal)(SensorValueType);
    static void (CriticalObject::* const deassertHighSignal)(SensorValueType);
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
    auto alarmLowState = (*realIface.*Thresholds<T>::getAlarmLow)();
    auto alarmHighState = (*realIface.*Thresholds<T>::getAlarmHigh)();
    (*realIface.*Thresholds<T>::alarmLo)(value <= lo);
    (*realIface.*Thresholds<T>::alarmHi)(value >= hi);
    if (alarmLowState != (value <= lo))
    {
        if (value <= lo)
        {
            (*realIface.*Thresholds<T>::assertLowSignal)(value);
        }
        else
        {
            (*realIface.*Thresholds<T>::deassertLowSignal)(value);
        }
    }
    if (alarmHighState != (value >= hi))
    {
        if (value >= hi)
        {
            (*realIface.*Thresholds<T>::assertHighSignal)(value);
        }
        else
        {
            (*realIface.*Thresholds<T>::deassertHighSignal)(value);
        }
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
    if (!tLo.empty() || !tHi.empty())
    {
        auto& bus = *std::get<sdbusplus::bus_t*>(info);

        iface = std::make_shared<T>(bus, objPath.c_str(),
                                    T::action::emit_no_signals);
        if (!tLo.empty())
        {
            auto lo = stod(tLo) * std::pow(10, scale);
            (*iface.*Thresholds<T>::setLo)(lo);
            auto alarmLowState = (*iface.*Thresholds<T>::getAlarmLow)();
            (*iface.*Thresholds<T>::alarmLo)(value <= lo);
            if (alarmLowState != (value <= lo))
            {
                if (value <= lo)
                {
                    (*iface.*Thresholds<T>::assertLowSignal)(value);
                }
                else
                {
                    (*iface.*Thresholds<T>::deassertLowSignal)(value);
                }
            }
        }
        if (!tHi.empty())
        {
            auto hi = stod(tHi) * std::pow(10, scale);
            (*iface.*Thresholds<T>::setHi)(hi);
            auto alarmHighState = (*iface.*Thresholds<T>::getAlarmHigh)();
            (*iface.*Thresholds<T>::alarmHi)(value >= hi);
            if (alarmHighState != (value >= hi))
            {
                if (value >= hi)
                {
                    (*iface.*Thresholds<T>::assertHighSignal)(value);
                }
                else
                {
                    (*iface.*Thresholds<T>::deassertHighSignal)(value);
                }
            }
        }
        auto type = Thresholds<T>::type;
        obj[type] = iface;
    }

    return iface;
}
