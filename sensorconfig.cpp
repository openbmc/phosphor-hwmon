#include "sensorconfig.hpp"

#include "env.hpp"

#include <limits>
#include <memory>
#include <string>
#include <unordered_set>

namespace conf
{

using dbl = std::numeric_limits<double>;

std::unique_ptr<DeviceConfig> DeviceConfig::buildFromEnv()
{
    auto devConfig = std::make_unique<DeviceConfig>();

    auto interval = env::getEnv("INTERVAL");
    if (!interval.empty())
    {
        devConfig->interval = std::strtoull(interval.c_str(), NULL, 10);
    }

    return devConfig;
}

std::unique_ptr<SensorConfig>
    SensorConfig::buildFromEnv(const std::string& sensorName)
{
    // TODO: finish
    return nullptr;
}

std::unique_ptr<SensorConfig>
    FanConfig::buildFanFromEnv(const std::string& fanName)
{
    // TODO: finish
    return nullptr;
}

} // namespace conf

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
