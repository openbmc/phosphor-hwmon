#pragma once

#include "interface.hpp"

#include <any>
#include <chrono>
#include <map>
#include <string>
#include <tuple>
#include <utility>

using Object = std::map<InterfaceType, std::any>;
using ObjectInfo = std::tuple<sdbusplus::bus::bus*, std::string, Object>;
using RetryIO = std::tuple<size_t, std::chrono::milliseconds>;
using ObjectStateData = std::pair<std::string, ObjectInfo>;
