#pragma once

#include "data_types.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

class Monitor;

template <typename T>
auto make_condition(T&& condition)
{
    return Condition(std::forward<T>(condition));
}

template <typename T>
auto make_action(T&& action)
{
    return Action(std::forward<T>(action));
}

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
