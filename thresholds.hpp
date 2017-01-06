#pragma once

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
    static constexpr int64_t (WarningObject::*setLo)(int64_t) =
        &WarningObject::warningLow;
    static constexpr int64_t (WarningObject::*setHi)(int64_t) =
        &WarningObject::warningHigh;
    static constexpr int64_t (WarningObject::*getLo)() const =
        &WarningObject::warningLow;
    static constexpr int64_t (WarningObject::*getHi)() const =
        &WarningObject::warningHigh;
    static constexpr bool (WarningObject::*alarmLo)(bool) =
        &WarningObject::warningAlarmLow;
    static constexpr bool (WarningObject::*alarmHi)(bool) =
        &WarningObject::warningAlarmHigh;
};

/**@brief Thresholds specialization for critical thresholds. */
template <>
struct Thresholds<CriticalObject>
{
    static constexpr InterfaceType type = InterfaceType::CRIT;
    static constexpr const char* envLo = "CRITLO";
    static constexpr const char* envHi = "CRITHI";
    static constexpr int64_t (CriticalObject::*setLo)(int64_t) =
        &CriticalObject::criticalLow;
    static constexpr int64_t (CriticalObject::*setHi)(int64_t) =
        &CriticalObject::criticalHigh;
    static constexpr int64_t (CriticalObject::*getLo)() const =
        &CriticalObject::criticalLow;
    static constexpr int64_t (CriticalObject::*getHi)() const =
        &CriticalObject::criticalHigh;
    static constexpr bool (CriticalObject::*alarmLo)(bool) =
        &CriticalObject::criticalAlarmLow;
    static constexpr bool (CriticalObject::*alarmHi)(bool) =
        &CriticalObject::criticalAlarmHigh;
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
void checkThresholds(std::experimental::any& iface, int64_t value)
{
    auto realIface = std::experimental::any_cast<std::shared_ptr<T>>
                     (iface);
    auto lo = (*realIface.*Thresholds<T>::getLo)();
    auto hi = (*realIface.*Thresholds<T>::getHi)();
    (*realIface.*Thresholds<T>::alarmLo)(value < lo);
    (*realIface.*Thresholds<T>::alarmHi)(value > hi);
}

/** @brief addThreshold
 *
 *  Look for a configured threshold value in the environment and
 *  create an sdbusplus server threshold if found.
 *
 *  @tparam T - The threshold type.
 *
 *  @param[in] sensor - A sensor type and name.
 *  @param[in] value - The sensor reading.
 *  @param[in] info - The sdbusplus server connection and interfaces.
 */
template <typename T>
auto addThreshold(const SensorSet::key_type& sensor,
                  int64_t value, ObjectInfo& info)
{
    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& objPath = std::get<std::string>(info);
    auto& obj = std::get<Object>(info);
    std::shared_ptr<T> iface;
    auto tLo = getEnv(Thresholds<T>::envLo, sensor);
    auto tHi = getEnv(Thresholds<T>::envHi, sensor);
    if (!tLo.empty() && !tHi.empty())
    {
        iface = std::make_shared<T>(bus, objPath.c_str());
        auto lo = stoi(tLo);
        auto hi = stoi(tHi);
        (*iface.*Thresholds<T>::setLo)(lo);
        (*iface.*Thresholds<T>::setHi)(hi);
        (*iface.*Thresholds<T>::alarmLo)(value < lo);
        (*iface.*Thresholds<T>::alarmHi)(value > hi);
        auto type = Thresholds<T>::type;
        obj[type] = iface;
    }

    return iface;
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
