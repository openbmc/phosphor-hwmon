#include <experimental/filesystem>

#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Sensor/Device/error.hpp>

#include "sensor.hpp"
#include "sensorset.hpp"
#include "hwmon.hpp"
#include "sysfs.hpp"

namespace sensor
{

Sensor::Sensor(const SensorSet::key_type& sensor) :
    _sensor(sensor)
{
}

std::shared_ptr<StatusObject> addStatus(
        const SensorSet::key_type& sensor,
        const hwmonio::HwmonIO& ioAccess,
        const std::string& devPath,
        ObjectInfo& info)
{
    namespace fs = std::experimental::filesystem;

    std::shared_ptr<StatusObject> iface = nullptr;
    static constexpr bool deferSignals = true;
    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& objPath = std::get<std::string>(info);
    auto& obj = std::get<Object>(info);

    // Check if fault sysfs file exists
    std::string faultName = sensor.first;
    std::string faultID = sensor.second;
    std::string entry = hwmon::entry::fault;

    auto sysfsFullPath = sysfs::make_sysfs_path(ioAccess.path(),
                                                faultName,
                                                faultID,
                                                entry);
    if (fs::exists(sysfsFullPath))
    {
        bool functional = true;
        uint32_t fault = 0;
        try
        {
            fault = ioAccess.read(faultName,
                                  faultID,
                                  entry,
                                  hwmonio::retries,
                                  hwmonio::delay);
            if (fault != 0)
            {
                functional = false;
            }
        }
        catch (const std::system_error& e)
        {
            using namespace phosphor::logging;
            using namespace sdbusplus::xyz::openbmc_project::
                Sensor::Device::Error;
            using metadata = xyz::openbmc_project::Sensor::
                Device::ReadFailure;

            report<ReadFailure>(
                    metadata::CALLOUT_ERRNO(e.code().value()),
                    metadata::CALLOUT_DEVICE_PATH(devPath.c_str()));

            log<level::INFO>("Logging failing sysfs file",
                    phosphor::logging::entry(
                            "FILE=%s", sysfsFullPath.c_str()));
        }

        iface = std::make_shared<StatusObject>(
                bus,
                objPath.c_str(),
                deferSignals);
        // Set functional property
        iface->functional(functional);

        obj[InterfaceType::STATUS] = iface;
    }

    return iface;
}

} // namespace sensor
