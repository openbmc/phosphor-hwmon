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
    static int64_t (WarningObject::*const setLo)(int64_t);
    static int64_t (WarningObject::*const setHi)(int64_t);
    static int64_t (WarningObject::*const getLo)() const;
    static int64_t (WarningObject::*const getHi)() const;
    static bool (WarningObject::*const alarmLo)(bool);
    static bool (WarningObject::*const alarmHi)(bool);
};

/**@brief Thresholds specialization for critical thresholds. */
template <>
struct Thresholds<CriticalObject>
{
    static constexpr InterfaceType type = InterfaceType::CRIT;
    static constexpr const char* envLo = "CRITLO";
    static constexpr const char* envHi = "CRITHI";
    static int64_t (CriticalObject::*const setLo)(int64_t);
    static int64_t (CriticalObject::*const setHi)(int64_t);
    static int64_t (CriticalObject::*const getLo)() const;
    static int64_t (CriticalObject::*const getHi)() const;
    static bool (CriticalObject::*const alarmLo)(bool);
    static bool (CriticalObject::*const alarmHi)(bool);
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
 *  @param[in] sensorType - sensor type, like 'temp'
 *  @param[in] sensorID - sensor ID, like '5'
 *  @param[in] value - The sensor reading.
 *  @param[in] info - The sdbusplus server connection and interfaces.
 */
template <typename T>
auto addThreshold(const std::string& sensorType,
                  const std::string& sensorID,
                  int64_t value,
                  ObjectInfo& info)
{
    static constexpr bool deferSignals = true;

    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& objPath = std::get<std::string>(info);
    auto& obj = std::get<Object>(info);
    std::shared_ptr<T> iface;

    auto tLo = getEnv(Thresholds<T>::envLo, sensorType, sensorID);
    auto tHi = getEnv(Thresholds<T>::envHi, sensorType, sensorID);
    if (!tLo.empty() && !tHi.empty())
    {
        iface = std::make_shared<T>(bus, objPath.c_str(), deferSignals);
        auto lo = stoll(tLo);
        auto hi = stoll(tHi);
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
