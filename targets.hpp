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
 *  @param[in] hwmonRoot - The root hwmon path
 *  @param[in] instance - The target instance name
 *  @param[in] info - The sdbusplus server connection and interfaces
 */
template <typename T>
void addTarget(const SensorSet::key_type& sensor,
               const std::string& hwmonRoot,
               const std::string& instance,
               ObjectInfo& info)
{
    namespace fs = std::experimental::filesystem;
    static constexpr bool deferSignals = true;

    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& obj = std::get<Object>(info);
    auto& objPath = std::get<std::string>(info);

    // Check if target sysfs file exists
    std::string targetPath = hwmonRoot + '/' + instance;
    auto sysfsFullPath = make_sysfs_path(targetPath,
                                         sensor.first,
                                         sensor.second,
                                         hwmon::entry::target);
    if (fs::exists(sysfsFullPath))
    {
        auto iface = std::make_shared<T>(targetPath,
                                         sysfsFullPath,
                                         bus,
                                         objPath.c_str(),
                                         deferSignals);
        auto type = Targets<T>::type;
        obj[type] = iface;
    }
}
