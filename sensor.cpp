#include "config.h"

#include "sensor.hpp"

#include "env.hpp"
#include "gpio_handle.hpp"
#include "hwmon.hpp"
#include "sensorset.hpp"
#include "sysfs.hpp"

#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Common/error.hpp>
#include <xyz/openbmc_project/Sensor/Device/error.hpp>

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>
#include <future>
#include <thread>

namespace sensor
{

using namespace phosphor::logging;
using namespace sdbusplus::xyz::openbmc_project::Common::Error;

// todo: this can be simplified once we move to the double interface
Sensor::Sensor(const SensorSet::key_type& sensor,
               const hwmonio::HwmonIOInterface* ioAccess,
               const std::string& devPath) :
    _sensor(sensor), _ioAccess(ioAccess), _devPath(devPath), _scale(0),
    _hasFaultFile(false)
{
    auto chip = env::getEnv("GPIOCHIP", sensor);
    auto access = env::getEnv("GPIO", sensor);
    if (!access.empty() && !chip.empty())
    {
        _handle = gpio::BuildGpioHandle(chip, access);

        if (!_handle)
        {
            log<level::ERR>("Unable to set up gpio locking");
            elog<InternalFailure>();
        }
    }

    auto gain = env::getEnv("GAIN", sensor);
    if (!gain.empty())
    {
        _sensorAdjusts.gain = std::stod(gain);
    }

    auto offset = env::getEnv("OFFSET", sensor);
    if (!offset.empty())
    {
        _sensorAdjusts.offset = std::stoi(offset);
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
            _sensorAdjusts.rmRCs.insert(std::stoi(rmRC));
        }
        catch (const std::logic_error& le)
        {
            // Unable to convert to int, continue to next token
            std::string name = _sensor.first + "_" + _sensor.second;
            log<level::INFO>("Unable to convert sensor removal return code",
                             entry("SENSOR=%s", name.c_str()),
                             entry("RC=%s", rmRC),
                             entry("EXCEPTION=%s", le.what()));
        }
        rmRC = std::strtok(nullptr, ", ");
    }
}

SensorValueType Sensor::adjustValue(SensorValueType value)
{
// Because read doesn't have an out pointer to store errors.
// let's assume negative values are errors if they have this
// set.
#if NEGATIVE_ERRNO_ON_FAIL
    if (value < 0)
    {
        return value;
    }
#endif

    // Adjust based on gain and offset
    value = static_cast<decltype(value)>(
        static_cast<double>(value) * _sensorAdjusts.gain +
        _sensorAdjusts.offset);

    if constexpr (std::is_same<SensorValueType, double>::value)
    {
        value *= std::pow(10, _scale);
    }

    return value;
}

std::shared_ptr<ValueObject> Sensor::addValue(
    const RetryIO& retryIO, ObjectInfo& info, TimedoutMap& timedoutMap)
{
    // Get the initial value for the value interface.
    auto& bus = *std::get<sdbusplus::bus_t*>(info);
    auto& obj = std::get<InterfaceMap>(info);
    auto& objPath = std::get<std::string>(info);

    SensorValueType val = 0;

    auto& statusIface = std::any_cast<std::shared_ptr<StatusObject>&>(
        obj[InterfaceType::STATUS]);
    // As long as addStatus is called before addValue, statusIface
    // should never be nullptr
    assert(statusIface);

    // Only read the input value if the status is functional
    if (statusIface->functional())
    {
#if UPDATE_FUNCTIONAL_ON_FAIL
        try
#endif
        {
            // RAII object for GPIO unlock / lock
            auto locker = gpioUnlock(getGpio());

            // For sensors with attribute ASYNC_READ_TIMEOUT,
            // spawn a thread with timeout
            auto asyncReadTimeout = env::getEnv("ASYNC_READ_TIMEOUT", _sensor);
            if (!asyncReadTimeout.empty())
            {
                std::chrono::milliseconds asyncTimeout{
                    std::stoi(asyncReadTimeout)};
                val = asyncRead(_sensor, _ioAccess, asyncTimeout, timedoutMap,
                                _sensor.first, _sensor.second,
                                hwmon::entry::cinput, std::get<size_t>(retryIO),
                                std::get<std::chrono::milliseconds>(retryIO));
            }
            else
            {
                // Retry for up to a second if device is busy
                // or has a transient error.
                val = _ioAccess->read(
                    _sensor.first, _sensor.second, hwmon::entry::cinput,
                    std::get<size_t>(retryIO),
                    std::get<std::chrono::milliseconds>(retryIO));
            }
        }
#if UPDATE_FUNCTIONAL_ON_FAIL
        catch (const std::system_error& e)
        {
            // Catch the exception here and update the functional property.
            // By catching the exception, it will not propagate it up the stack
            // and thus the code will skip the "Remove RCs" check in
            // MainLoop::getObject and will not exit on failure.
            statusIface->functional(false);
        }
#endif
    }

    auto iface = std::make_shared<ValueObject>(bus, objPath.c_str(),
                                               ValueObject::action::defer_emit);

    hwmon::Attributes attrs;
    if (hwmon::getAttributes(_sensor.first, attrs))
    {
        iface->unit(hwmon::getUnit(attrs));

        _scale = hwmon::getScale(attrs);
    }

    val = adjustValue(val);
    iface->value(val);

    auto maxValue = env::getEnv("MAXVALUE", _sensor);
    if (!maxValue.empty())
    {
        iface->maxValue(std::stoll(maxValue));
    }
    auto minValue = env::getEnv("MINVALUE", _sensor);
    if (!minValue.empty())
    {
        iface->minValue(std::stoll(minValue));
    }

    obj[InterfaceType::VALUE] = iface;
    return iface;
}

std::shared_ptr<StatusObject> Sensor::addStatus(ObjectInfo& info)
{
    namespace fs = std::filesystem;

    std::shared_ptr<StatusObject> iface = nullptr;
    auto& objPath = std::get<std::string>(info);
    auto& obj = std::get<InterfaceMap>(info);

    // Check if fault sysfs file exists
    std::string faultName = _sensor.first;
    std::string faultID = _sensor.second;
    std::string entry = hwmon::entry::fault;

    bool functional = true;
    auto sysfsFullPath =
        sysfs::make_sysfs_path(_ioAccess->path(), faultName, faultID, entry);
    if (fs::exists(sysfsFullPath))
    {
        _hasFaultFile = true;
        try
        {
            uint32_t fault = _ioAccess->read(faultName, faultID, entry,
                                             hwmonio::retries, hwmonio::delay);
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

            report<ReadFailure>(
                metadata::CALLOUT_ERRNO(e.code().value()),
                metadata::CALLOUT_DEVICE_PATH(_devPath.c_str()));

            log<level::INFO>(std::format("Failing sysfs file: {} errno {}",
                                         sysfsFullPath, e.code().value())
                                 .c_str());
        }
    }

    auto& bus = *std::get<sdbusplus::bus_t*>(info);

    iface = std::make_shared<StatusObject>(
        bus, objPath.c_str(), StatusObject::action::emit_no_signals);
    // Set functional property
    iface->functional(functional);

    obj[InterfaceType::STATUS] = iface;

    return iface;
}

std::shared_ptr<AccuracyObject> Sensor::addAccuracy(ObjectInfo& info,
                                                    double accuracy)
{
    auto& objPath = std::get<std::string>(info);
    auto& obj = std::get<InterfaceMap>(info);

    auto& bus = *std::get<sdbusplus::bus_t*>(info);
    auto iface = std::make_shared<AccuracyObject>(
        bus, objPath.c_str(), AccuracyObject::action::emit_no_signals);

    iface->accuracy(accuracy);
    obj[InterfaceType::ACCURACY] = iface;

    return iface;
}

std::shared_ptr<PriorityObject> Sensor::addPriority(ObjectInfo& info,
                                                    size_t priority)
{
    auto& objPath = std::get<std::string>(info);
    auto& obj = std::get<InterfaceMap>(info);

    auto& bus = *std::get<sdbusplus::bus_t*>(info);
    auto iface = std::make_shared<PriorityObject>(
        bus, objPath.c_str(), PriorityObject::action::emit_no_signals);

    iface->priority(priority);
    obj[InterfaceType::PRIORITY] = iface;

    return iface;
}

void gpioLock(const gpioplus::HandleInterface*&& handle)
{
    handle->setValues({0});
}

std::optional<GpioLocker> gpioUnlock(const gpioplus::HandleInterface* handle)
{
    if (handle == nullptr)
    {
        return std::nullopt;
    }

    handle->setValues({1});
    // Default pause needed to guarantee sensors are ready
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return GpioLocker(std::move(handle));
}

SensorValueType asyncRead(
    const SensorSet::key_type& sensorSetKey,
    const hwmonio::HwmonIOInterface* ioAccess,
    std::chrono::milliseconds asyncTimeout, TimedoutMap& timedoutMap,
    const std::string& type, const std::string& id, const std::string& sensor,
    const size_t retries, const std::chrono::milliseconds delay)
{
    // Default async read timeout
    bool valueIsValid = false;
    std::future<int64_t> asyncThread;

    auto asyncIter = timedoutMap.find(sensorSetKey);
    if (asyncIter == timedoutMap.end())
    {
        // If sensor not found in timedoutMap, spawn an async thread
        asyncThread =
            std::async(std::launch::async, &hwmonio::HwmonIOInterface::read,
                       ioAccess, type, id, sensor, retries, delay);
        valueIsValid = true;
    }
    else
    {
        // If we already have the async thread in the timedoutMap, it means this
        // sensor has already timed out in the previous reads. No need to wait
        // on subsequent reads - proceed to check the future_status to see when
        // the async thread finishes
        asyncTimeout = std::chrono::seconds(0);
        asyncThread = std::move(asyncIter->second);
    }

    // TODO: This is still not a true asynchronous read as it still blocks the
    // main thread for asyncTimeout amount of time. To make this completely
    // asynchronous, schedule a read and register a callback to update the
    // sensor value
    std::future_status status = asyncThread.wait_for(asyncTimeout);
    switch (status)
    {
        case std::future_status::ready:
            // Read has finished
            if (valueIsValid)
            {
                return asyncThread.get();
                // Good sensor reads should skip the code below
            }
            // Async read thread has completed but had previously timed out (was
            // found in the timedoutMap). Erase from timedoutMap and throw to
            // allow retry in the next read cycle. Not returning the read value
            // as the sensor reading may be bad / corrupted if it took so long.
            timedoutMap.erase(sensorSetKey);
            throw AsyncSensorReadTimeOut();
        default:
            // Read timed out so add the thread to the timedoutMap (if the entry
            // already exists, operator[] updates it).
            //
            // Keeping the timed out futures in a map is required to prevent
            // their destructor from being called when returning from this
            // stack. The destructor will otherwise block until the read
            // completes due to the limitation of std::async.
            timedoutMap[sensorSetKey] = std::move(asyncThread);
            throw AsyncSensorReadTimeOut();
    }
}

} // namespace sensor
