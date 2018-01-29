#pragma once

#include "xyz/openbmc_project/Sensor/Value/server.hpp"
#include "xyz/openbmc_project/Sensor/Threshold/Warning/server.hpp"
#include "xyz/openbmc_project/Sensor/Threshold/Critical/server.hpp"
#include "xyz/openbmc_project/Control/FanSpeed/server.hpp"
#include "xyz/openbmc_project/Control/FanPwm/server.hpp"
#include <sdbusplus/server.hpp>

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;

using ValueInterface = sdbusplus::xyz::openbmc_project::Sensor::server::Value;
using ValueObject = ServerObject<ValueInterface>;
using WarningInterface =
    sdbusplus::xyz::openbmc_project::Sensor::Threshold::server::Warning;
using WarningObject = ServerObject<WarningInterface>;
using CriticalInterface =
    sdbusplus::xyz::openbmc_project::Sensor::Threshold::server::Critical;
using CriticalObject = ServerObject<CriticalInterface>;
using FanSpeedInterface =
    sdbusplus::xyz::openbmc_project::Control::server::FanSpeed;
using FanSpeedObject = ServerObject<FanSpeedInterface>;
using FanPwmInterface =
    sdbusplus::xyz::openbmc_project::Control::server::FanPwm;
using FanPwmObject = ServerObject<FanPwmInterface>;

enum class InterfaceType
{
    VALUE,
    WARN,
    CRIT,
    FAN_SPEED,
    FAN_PWM,
};

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
