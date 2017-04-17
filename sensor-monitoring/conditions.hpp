#pragma once

#include <algorithm>
#include "data_types.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{
namespace condition
{

/**
 * @brief A condition used to trigger an action when a number of sensors are at
 * or above a given value
 * @details A given group of sensors is updated with their last known sensor
 * value, which then the entire group is check if there are a given number of
 * them at or above a value which would cause the condition to be true
 *
 * @param[in] sensors - Group of sensors
 * @param[in] path - Path of a sensor within the group
 * @param[in] count - Number of sensors needed at or above value
 * @param[in] value - Value of sensors to be at or above
 *
 * @return Lambda function
 *     A lambda function to determine if the number of sensors within the group
 *     are at or above the given value
 */
template <typename T>
auto countAtOrAbove(Group& sensors, const char* path, size_t count, T&& value)
{
    return [&sensors,
            path,
            count,
            value = std::forward<T>(value)](T&& arg)
    {
        Group::iterator it =
            std::find_if(sensors.begin(),
                         sensors.end(),
                         [&path](auto const& sensor)
                         {
                             return std::get<0>(sensor) == path;
                         });
        if (it != std::end(sensors))
        {
            std::get<1>(*it) = arg;
        }
        size_t condCount =
            std::count_if(sensors.begin(),
                          sensors.end(),
                          [&value](auto const& sensor)
                          {
                              return std::get<1>(sensor) >= value;
                          });
        return condCount >= count;
    };
}

} // namespace condition
} // namespace monitoring
} // namespace sensor
} // namespace phosphor
