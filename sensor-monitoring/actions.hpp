#pragma once

namespace phosphor
{
namespace sensor
{
namespace monitoring
{
namespace action
{

inline auto log_error(const char* msg)
{
    return [=](auto&, auto& mon)
    {
        mon.log_error(msg);
    };
}

} // namespace action
} // namespace monitoring
} // namespace sensor
} // namespace phosphor
