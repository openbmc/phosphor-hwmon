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

using Value = sdbusplus::message::variant<bool, int64_t, std::string>;

using Group = std::vector<std::tuple<std::string, Value>>;

using Condition = std::function<bool(sdbusplus::bus::bus&,
                                     sdbusplus::message::message&,
                                     Monitor&)>;

using Action = std::function<void(sdbusplus::bus::bus&,
                                  Monitor&)>;

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
