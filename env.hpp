#pragma once

class SensorSet;

std::string getEnv(
    const char* prefix, const SensorSet::key_type& sensor);

/** @brief Get the label for the sensor with a level of indirection.
 *
 *  Read the sensor number from the <path>/<item><X>_label file.<item> & <X> is
 *  populated from the sensor key. The sensor label is read from the environment
 *  variable <prefix>_<item><sensorNum>.
 *
 *  @param[in] prefix - Prefix of the environment variable.
 *  @param[in] path - Directory path of the label file.
 *  @param[in] sensor - Sensor details.
 *
 *  @return On success return the sensor label, in case of failure return empty
 *          string.
 */
std::string getIndirectLabelEnv(const char* prefix,
                                std::string path,
                                const SensorSet::key_type& sensor);
