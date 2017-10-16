#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Control/Device/error.hpp>
#include "sensorset.hpp"
#include "env.hpp"
#include "fan_speed.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"

using namespace phosphor::logging;

namespace hwmon
{

uint64_t FanSpeed::target(uint64_t value)
{
    auto curValue = FanSpeedObject::target();

    if (curValue != value)
    {
        //Write target out to sysfs
        try
        {
            ioAccess.write(
                    value,
                    type,
                    id,
                    entry::target,
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
                    entry::target);

            log<level::INFO>("Logging failing sysfs file",
                    phosphor::logging::entry("FILE=%s", file.c_str()));

            exit(EXIT_FAILURE);
        }
    }

    return FanSpeedObject::target(value);
}


void FanSpeed::enable()
{
    auto enable = getEnv("ENABLE", type, id);
    if (!enable.empty())
    {
        auto val = std::stoul(enable);

        try
        {
            ioAccess.write(
                    val,
                    type::pwm,
                    id,
                    entry::enable,
                    sysfs::hwmonio::retries,
                    sysfs::hwmonio::delay);
        }
        catch (const std::system_error& e)
        {
            using namespace sdbusplus::xyz::openbmc_project::Control::
                Device::Error;
            phosphor::logging::report<WriteFailure>(
                    xyz::openbmc_project::Control::Device::
                        WriteFailure::CALLOUT_ERRNO(e.code().value()),
                    xyz::openbmc_project::Control::Device::
                        WriteFailure::CALLOUT_DEVICE_PATH(devPath.c_str()));

            auto fullPath = sysfs::make_sysfs_path(
                    ioAccess.path(),
                    type::pwm,
                    id,
                    entry::enable);

            log<level::INFO>("Logging failing sysfs file",
                    phosphor::logging::entry("FILE=%s", fullPath.c_str()));

            exit(EXIT_FAILURE);
        }
    }
}


} // namespace hwmon
