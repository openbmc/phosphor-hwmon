#include "fan_speed.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"
#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Control/Device/error.hpp>
#include <xyz/openbmc_project/Sensor/Device/error.hpp>

using namespace phosphor::logging;

namespace hwmon
{

FanSpeed::FanSpeed(const std::string& instancePath,
                   const std::string& devPath,
                   const std::string& id,
                   sdbusplus::bus::bus& bus,
                   const char* objPath,
                   bool defer) :
        FanSpeedObject(bus, objPath, defer),
        id(id),
        ioAccess(instancePath),
        devPath(devPath)
{
    //Read the initial target value to save in the object
    try
    {
        auto val = ioAccess.read(
                type,
                id,
                entry::target,
                sysfs::hwmonio::retries,
                sysfs::hwmonio::delay);

        FanSpeedObject::target(val);
    }
    catch (const std::system_error& e)
    {
        //Log the error, but still construct the object.
        //It will just have an initial value of zero.
        using namespace sdbusplus::xyz::openbmc_project::
                Sensor::Device::Error;
        using metadata = xyz::openbmc_project::Sensor::
                Device::ReadFailure;

        report<ReadFailure>(
                metadata::CALLOUT_ERRNO(e.code().value()),
                metadata::CALLOUT_DEVICE_PATH(devPath.c_str()));

        auto path = sysfs::make_sysfs_path(
                instancePath,
                type,
                id,
                entry::target);

        log<level::INFO>("Logging failing sysfs file",
                phosphor::logging::entry("FILE=%s", path.c_str()));
    }
}

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


} // namespace hwmon
