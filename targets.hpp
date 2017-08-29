#pragma once

#include <experimental/filesystem>
#include "fan_speed.hpp"

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
struct Targets<hwmon::FanSpeed>
{
    static constexpr InterfaceType type = InterfaceType::FAN_SPEED;
};

/** @brief addTarget
 *
 *  Creates the target type interface
 *
 *  @tparam T - The target type
 *
 *  @param[in] sensor - A sensor type and name
 *  @param[in] instance - The target instance path
 *  @param[in] info - The sdbusplus server connection and interfaces
 *
 *  @return A shared pointer to the target interface object
 *          Will be empty if no interface was created
 */
template <typename T>
std::shared_ptr<T> addTarget(const SensorSet::key_type& sensor,
                             const std::string& instancePath,
                             const std::string& devPath,
                             ObjectInfo& info)
{
    std::shared_ptr<T> target;
    namespace fs = std::experimental::filesystem;
    static constexpr bool deferSignals = true;

    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& obj = std::get<Object>(info);
    auto& objPath = std::get<std::string>(info);

    // Check if target sysfs file exists
    auto sysfsFullPath = sysfs::make_sysfs_path(instancePath,
                                                sensor.first,
                                                sensor.second,
                                                hwmon::entry::target);
    if (fs::exists(sysfsFullPath))
    {
        target = std::make_shared<T>(instancePath,
                                     devPath,
                                     sensor.second,
                                     bus,
                                     objPath.c_str(),
                                     deferSignals);
        auto type = Targets<T>::type;
        obj[type] = target;
    }

    return target;
}
