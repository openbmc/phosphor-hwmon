#pragma once

#include <memory>
#include <unordered_set>

#include "interface.hpp"
#include "sensorset.hpp"
#include "mainloop.hpp"

namespace sensor
{
// The gain and offset to adjust a value
struct valueAdjust
{
    double gain = 1.0;
    int offset = 0;
    std::unordered_set<int> rmRCs;
};

/**
 * @brief Adjust a value given the gain and offset.
 *
 * @param[in] sensor - Sensor identification data
 * @param[in] value - The raw value
 *
 * @return The adjusted sensor value.
 */
int64_t adjustValue(const SensorSet::key_type& sensor, int64_t value);

/**
 * @brief Add remove return-codes for a sensor given a string list.
 *
 * @param[in] sensor - Sensor identification data
 * @param[in] rcList - Comma-separated list of return-codes.
 */
void addRemoveRCs(
        const SensorSet::key_type& sensor,
        const std::string& rcList);

/**
 * @brief Add Value interface and properties for sensor.
 *
 * @param[in] sensor - Sensor identification data
 * @param[in] retryIO - The retry values for this sensor
 * @param[in] ioAccess - I/O access to sysfs
 * @param[in] info - Sensor object information
 *
 * @return - Shared pointer to the value object
 */
std::shared_ptr<ValueObject> addValue(
        const SensorSet::key_type& sensor,
        const RetryIO& retryIO,
        hwmonio::HwmonIO& ioAccess,
        ObjectInfo& info);

/**
 * @brief Add status interface and functional property for sensor
 * @details When a sensor has an associated fault file, the OperationalStatus
 * interface is added along with setting the Functional property to the
 * corresponding value found in the fault file.
 *
 * @param[in] sensor - Sensor identification data
 * @param[in] ioAccess - I/O access to sysfs
 * @param[in] devPath - Device path
 * @param[in] info - Sensor object information
 *
 * @return - Shared pointer to the status object
 */
std::shared_ptr<StatusObject> addStatus(
        const SensorSet::key_type& sensor,
        const hwmonio::HwmonIO& ioAccess,
        const std::string& devPath,
        ObjectInfo& info);

} // namespace sensor
