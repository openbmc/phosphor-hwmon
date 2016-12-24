#pragma once

#include <xyz/openbmc_project/Sensor/Value/server.hpp>
#include <sdbusplus/server.hpp>

template <typename T>
using ServerObject = typename sdbusplus::server::object::object<T>;

using ValueInterface = sdbusplus::xyz::openbmc_project::Sensor::server::Value;
using ValueObject = ServerObject<ValueInterface>;

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
