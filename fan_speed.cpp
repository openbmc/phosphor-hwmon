#include "fan_speed.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"
#include <experimental/filesystem>

namespace hwmon
{

uint64_t FanSpeed::target(uint64_t value)
{
    auto curValue = FanSpeedObject::target();

    if (curValue != value)
    {
        // Write target out to sysfs
        curValue = sysfs::writeSysfsWithCallout(value,
                                                sysfsRoot,
                                                instance,
                                                type,
                                                id,
                                                entry::target);
    }

    return FanSpeedObject::target(value);
}


void FanSpeed::enable()
{
    namespace fs = std::experimental::filesystem;

    auto path = sysfsRoot + "/" + instance;
    auto fullPath = sysfs::make_sysfs_path(path,
                                           type::pwm,
                                           id,
                                           entry::enable);

    if (fs::exists(fullPath))
    {
        // This class always uses RPM mode
        sysfs::writeSysfsWithCallout(enable::rpmMode,
                                     sysfsRoot,
                                     instance,
                                     type::pwm,
                                     id,
                                     entry::enable);
    }
}


} // namespace hwmon
