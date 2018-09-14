#include "config.h"

#include "sensor.hpp"

#include "env.hpp"
#include "hwmon.hpp"
#include "sensorset.hpp"
#include "sysfs.hpp"

#include <cstring>
#include <experimental/filesystem>
#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Sensor/Device/error.hpp>

namespace sensor
{

using namespace phosphor::logging;

Sensor::Sensor(const SensorSet::key_type& sensor,
               const hwmonio::HwmonIO& ioAccess, const std::string& devPath) :
    sensor(sensor),
    ioAccess(ioAccess), devPath(devPath)
{
    auto chip = env::getEnv("GPIOCHIP", sensor);
    auto access = env::getEnv("GPIO", sensor);
    if (!access.empty() && !chip.empty())
    {
        handle = gpio::GpioHandle::BuildGpioHandle(chip, access);

        if (!handle)
        {
            log<level::ERR>("Unable to set up gpio locking");
        }
    }

    auto gain = env::getEnv("GAIN", sensor);
    if (!gain.empty())
    {
        sensorAdjusts.gain = std::stod(gain);
    }

    auto offset = env::getEnv("OFFSET", sensor);
    if (!offset.empty())
    {
        sensorAdjusts.offset = std::stoi(offset);
    }
    auto senRmRCs = env::getEnv("REMOVERCS", sensor);
    // Add sensor removal return codes defined per sensor
    addRemoveRCs(senRmRCs);
}

void Sensor::addRemoveRCs(const std::string& rcList)
{
    if (rcList.empty())
    {
        return;
    }

    // Convert to a char* for strtok
    std::vector<char> rmRCs(rcList.c_str(), rcList.c_str() + rcList.size() + 1);
    auto rmRC = std::strtok(&rmRCs[0], ", ");
    while (rmRC != nullptr)
    {
        try
        {
            sensorAdjusts.rmRCs.insert(std::stoi(rmRC));
        }
        catch (const std::logic_error& le)
        {
            // Unable to convert to int, continue to next token
            std::string name = sensor.first + "_" + sensor.second;
            log<level::INFO>("Unable to convert sensor removal return code",
                             entry("SENSOR=%s", name.c_str()),
                             entry("RC=%s", rmRC),
                             entry("EXCEPTION=%s", le.what()));
        }
        rmRC = std::strtok(nullptr, ", ");
    }
}

int64_t Sensor::adjustValue(int64_t value)
{
// Because read doesn't have an out pointer to store errors.
// let's assume negative values are errors if they have this
// set.
#ifdef NEGATIVE_ERRNO_ON_FAIL
    if (value < 0)
    {
        return value;
    }
#endif

    // Adjust based on gain and offset
    value = static_cast<decltype(value)>(
        static_cast<double>(value) * sensorAdjusts.gain + sensorAdjusts.offset);

    return value;
}

std::shared_ptr<ValueObject> Sensor::addValue(const RetryIO& retryIO,
                                              ObjectInfo& info)
{
    static constexpr bool deferSignals = true;

    // Get the initial value for the value interface.
    auto& bus = *std::get<sdbusplus::bus::bus*>(info);
    auto& obj = std::get<Object>(info);
    auto& objPath = std::get<std::string>(info);

    int64_t val = 0;
    std::shared_ptr<StatusObject> statusIface = nullptr;
    auto it = obj.find(InterfaceType::STATUS);
    if (it != obj.end())
    {
        statusIface =
            std::experimental::any_cast<std::shared_ptr<StatusObject>>(
                it->second);
    }

    // If there's no fault file or the sensor has a fault file and
    // its status is functional, read the input value.
    if (!statusIface || (statusIface && statusIface->functional()))
    {
        unlockGpio();

        // Retry for up to a second if device is busy
        // or has a transient error.
        val = ioAccess.read(sensor.first, sensor.second, hwmon::entry::cinput,
                            std::get<size_t>(retryIO),
                            std::get<std::chrono::milliseconds>(retryIO));

        lockGpio();
        val = adjustValue(val);
    }

    auto iface =
        std::make_shared<ValueObject>(bus, objPath.c_str(), deferSignals);
    iface->value(val);

    hwmon::Attributes attrs;
    if (hwmon::getAttributes(sensor.first, attrs))
    {
        iface->unit(hwmon::getUnit(attrs));
        iface->scale(hwmon::getScale(attrs));
    }

    auto maxValue = env::getEnv("MAXVALUE", sensor);
    if (!maxValue.empty())
    {
        iface->maxValue(std::stoll(maxValue));
    }
    auto minValue = env::getEnv("MINVALUE", sensor);
    if (!minValue.empty())
    {
        iface->minValue(std::stoll(minValue));
    }

    obj[InterfaceType::VALUE] = iface;
    return iface;
}

std::shared_ptr<StatusObject> Sensor::addStatus(ObjectInfo& info)
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

    auto sysfsFullPath =
        sysfs::make_sysfs_path(ioAccess.path(), faultName, faultID, entry);
    if (fs::exists(sysfsFullPath))
    {
        bool functional = true;
        uint32_t fault = 0;
        try
        {
            fault = ioAccess.read(faultName, faultID, entry, hwmonio::retries,
                                  hwmonio::delay);
            if (fault != 0)
            {
                functional = false;
            }
        }
        catch (const std::system_error& e)
        {
            using namespace sdbusplus::xyz::openbmc_project::Sensor::Device::
                Error;
            using metadata = xyz::openbmc_project::Sensor::Device::ReadFailure;

            report<ReadFailure>(metadata::CALLOUT_ERRNO(e.code().value()),
                                metadata::CALLOUT_DEVICE_PATH(devPath.c_str()));

            log<level::INFO>(
                "Logging failing sysfs file",
                phosphor::logging::entry("FILE=%s", sysfsFullPath.c_str()));
        }

        iface =
            std::make_shared<StatusObject>(bus, objPath.c_str(), deferSignals);
        // Set functional property
        iface->functional(functional);

        obj[InterfaceType::STATUS] = iface;
    }

    return iface;
}

void Sensor::unlockGpio()
{
    if (handle)
    {
        handle->setValue(1);
    }
}

void Sensor::lockGpio()
{
    if (handle)
    {
        handle->setValue(0);
    }
}

} // namespace sensor
