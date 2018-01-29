#include "fan_pwm.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"

#include <experimental/filesystem>
#include <string>

namespace hwmon
{

uint64_t FanPwm::target(uint64_t value)
{
    auto curValue = FanPwmObject::target();
    using namespace std::literals;
    std::string empty = ""s;

    if (curValue != value)
    {
        //Write target out to sysfs
        try {
            ioAccess.write(
                value,
                type,
                id,
                empty,
                sysfs::hwmonio::retries,
                sysfs::hwmonio::delay);
        }
        catch (const std::system_error& e)
        {
            // Don't care.
        }
    }

    return FanPwmObject::target(value);
}

} // namespace hwmon

