#pragma once

#include <phosphor-logging/log.hpp>

namespace phosphor
{
namespace sensor
{
namespace monitoring
{
namespace action
{

using namespace phosphor::logging;

/**
 * @brief An action to log an error with the given message
 *
 * @param[in] msg - The message to log
 *
 * @return Lambda function
 *     A lambda function to perform the log_error function
 */
inline auto log_error(const char* msg)
{
    return [=](auto&, auto&)
    {
        log<level::ERR>(msg);
    };
}

} // namespace action
} // namespace monitoring
} // namespace sensor
} // namespace phosphor
