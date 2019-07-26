#pragma once

#include <limits>
#include <memory>
#include <string>
#include <unordered_set>

namespace conf
{

using dbl = std::numeric_limits<double>;

static constexpr auto default_interval = 1000000;

/** @class DeviceConfig
 *  @brief Device level configurations.
 */
struct DeviceConfig
{
    uint64_t interval = default_interval;
    std::unordered_set<int> rmRCs;

    static std::unique_ptr<DeviceConfig> buildFromEnv();
};

/** @class SensorConfig
 *  @brief Sensor level configurations.
 */
struct SensorConfig
{
    std::string label;
    double warnLo = dbl::quiet_NaN();
    double warnHi = dbl::quiet_NaN();
    double critLo = dbl::quiet_NaN();
    double critHi = dbl::quiet_NaN();
    double minVal = dbl::quiet_NaN();
    double maxVal = dbl::quiet_NaN();
    double gain = 1.0;
    double offset = 0.0;
    std::unordered_set<int> rmRCs;
    std::string mode;

    static std::unique_ptr<SensorConfig>
        buildFromEnv(const std::string& sensorName);
    bool overrideLabelWithEnvNamed(const std::string& envName);
};

struct FanConfig : public SensorConfig
{
    std::string targetMode;
    std::string enable;
    int pwmTarget;

    static std::unique_ptr<SensorConfig>
        buildFanFromEnv(const std::string& fanName);
};

} // namespace conf
