#pragma once

/** @class Targets
 *  @brief Target type traits.
 *
 *  @tparam T - The target type.
 */
template <typename T>
struct Targets
{
    static void fail()
    {
        static_assert(sizeof(Targets) == -1, "Unsupported Target type");
    }
};

/**@brief Targets specialization for fan speed. */
template <>
struct Targets<FanSpeedObject>
{
    static constexpr InterfaceType type = InterfaceType::FAN_SPEED;
    static constexpr int64_t (FanSpeedObject::*setTarget)(int64_t) =
        &FanSpeedObject::target;
    static constexpr int64_t (FanSpeedObject::*getTarget)() const =
        &FanSpeedObject::target;
};

/** @brief addTarget
 *
 *  Creates the target type interface and set the target value to the given
 *  initial target value.
 *
 *  @tparam T - The target type
 *
 *  @param[in] target - The initial target value
 *  @param[in] info - The sdbusplus server connection and interfaces
 */
template <typename T>
void addTarget(int64_t target, ObjectInfo& info)
{
    static constexpr bool deferSignals = true;

    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& obj = std::get<Object>(info);
    auto& objPath = std::get<std::string>(info);

    auto iface = std::make_shared<T>(bus, objPath.c_str(), deferSignals);
    iface->target(target);

    obj[Targets<T>::type] = iface;
    iface->emit_object_added();
}
