#include "env.hpp"
#include "fan_pwm.hpp"
#include "hwmon.hpp"
#include "sensorset.hpp"
#include "sysfs.hpp"

#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Control/Device/error.hpp>

#include <experimental/filesystem>
#include <string>

using namespace phosphor::logging;

namespace hwmon
{

uint64_t FanPwm::target(uint64_t value)
{
    auto curValue = FanPwmObject::target();
    using namespace std::literals;

    if (curValue != value)
    {
        std::string empty;
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
            using namespace sdbusplus::xyz::openbmc_project::Control::
                Device::Error;
            report<WriteFailure>(
                    xyz::openbmc_project::Control::Device::
                        WriteFailure::CALLOUT_ERRNO(e.code().value()),
                    xyz::openbmc_project::Control::Device::
                        WriteFailure::CALLOUT_DEVICE_PATH(devPath.c_str()));

            auto file = sysfs::make_sysfs_path(
                    ioAccess.path(),
                    type,
                    id,
                    empty);

            log<level::INFO>("Logging failing sysfs file",
                    phosphor::logging::entry("FILE=%s", file.c_str()));

            exit(EXIT_FAILURE);
        }
    }

    return FanPwmObject::target(value);
}

} // namespace hwmon

