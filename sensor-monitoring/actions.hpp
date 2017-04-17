#pragma once

namespace phosphor
{
namespace sensor
{
namespace monitoring
{
namespace action
{

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
    return [=](auto&, auto& mon)
    {
        mon.log_error(msg);
    };
}

} // namespace action
} // namespace monitoring
} // namespace sensor
} // namespace phosphor
