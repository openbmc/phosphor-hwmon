#include "fan_speed.hpp"
#include "sysfs.hpp"

namespace hwmon
{

uint64_t FanSpeed::target(uint64_t value)
{
    auto curValue = FanSpeedObject::target();

    if (curValue != value)
    {
        //Write target out to sysfs
        curValue = writeSysfsWithCallout(value, targetPath, sysfsFullPath);
    }

    return FanSpeedObject::target(value);
}

} // namespace hwmon
