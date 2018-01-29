#pragma once

#include <string>

#include "sensorset.hpp"

/** @brief Reads an environment variable
 *
 *  Reads <prefix>_<sensor.first><sensor.second>
 *
 *  @param[in] prefix - the variable prefix
 *  @param[in] sensor - Sensor details
 *
 *  @return string - the env var value
 */
std::string getEnv(
    const char* prefix, const SensorSet::key_type& sensor);

/** @brief Reads an environment variable, and takes type and id separately
 *
 *  @param[in] prefix - the variable prefix
 *  @param[in] type - sensor type, like 'temp'
 *  @param[in] id - sensor ID, like '5'
 *
 *  @return string - the env var value
 */
std::string getEnv(
    const char* prefix,
    const std::string& type,
    const std::string& id);

/** @brief Gets the ID for the sensor with a level of indirection
 *
 *  Read the sensor number/ID from the <path>/<item><X>_label file.
 *  <item> & <X> are populated from the sensor key.
 *
 *  @param[in] path - Directory path of the label file
 *  @param[in] sensor - Sensor details
 */
std::string getIndirectID(
    std::string path,
    const SensorSet::key_type& sensor);
