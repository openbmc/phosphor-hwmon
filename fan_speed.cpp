#include "fan_speed.hpp"
#include "sysfs.hpp"

namespace hwmon
{

int64_t FanSpeed::target(int64_t value)
{
    if (_target != value)
    {
        //Write target out to sysfs
        _target = writeSysfsWithCallout(value, targetPath, sysfsFullPath);
    }

    return _target;
}

} // namespace hwmon
