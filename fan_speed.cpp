#include "fan_speed.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"
#include <experimental/filesystem>
#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Control/Device/error.hpp>

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
                    entry::target);
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
            exit(EXIT_FAILURE);
        }
    }

    return FanSpeedObject::target(value);
}


void FanSpeed::enable()
{
    namespace fs = std::experimental::filesystem;

    auto fullPath = sysfs::make_sysfs_path(ioAccess.path(),
                                           type::pwm,
                                           id,
                                           entry::enable);

    if (fs::exists(fullPath))
    {
        //This class always uses RPM mode
        try
        {
            ioAccess.write(
                    enable::rpmMode,
                    type::pwm,
                    id,
                    entry::enable);
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
            exit(EXIT_FAILURE);
        }
    }
}


} // namespace hwmon
