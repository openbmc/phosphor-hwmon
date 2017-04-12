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
        size_t condCount = std::count_if(sensors.begin(),
                                         sensors.end(),
                                         [&value](auto const& val)
                                         {
                                             return std::get<1>(val) >= value;
                                         });
        return condCount >= count;
    };
}

} // namespace condition
} // namespace monitoring
} // namespace sensor
} // namespace phosphor
