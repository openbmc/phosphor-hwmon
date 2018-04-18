#pragma once

#include "sensorset.hpp"
#include "mainloop.hpp"

namespace sensor
{

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
