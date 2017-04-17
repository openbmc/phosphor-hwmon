#pragma once

#include <functional>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

class Monitor;

/** @brief The possible sensor value types */
using Value = int64_t;

/** @brief A list of what constructs a unique sensor and its value */
using Group = std::vector<std::tuple<std::string, Value>>;

/** @brief A conditional function type for sensor(s) conditions */
using Condition = std::function<bool(sdbusplus::bus::bus&,
                                     sdbusplus::message::message&,
                                     Monitor&)>;

/** @brief A void function type for actions based condition(s) */
using Action = std::function<void(sdbusplus::bus::bus&,
                                  Monitor&)>;

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
