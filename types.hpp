#pragma once

#include <experimental/any>
#include "interface.hpp"

using Object = std::map<InterfaceType, std::experimental::any>;
using ObjectInfo = std::tuple<sdbusplus::bus::bus*, std::string, Object>;
using RetryIO = std::tuple<size_t, std::chrono::milliseconds>;
using ObjectStateData = std::pair<std::string, ObjectInfo>;
