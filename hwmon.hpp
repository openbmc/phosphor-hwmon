#pragma once

#include <string>

namespace hwmon
{
namespace entry
{
static constexpr auto cinput = "input";
static constexpr auto clabel = "label";
static constexpr auto ctarget = "target";
static constexpr auto cenable = "enable";
static constexpr auto cfault = "fault";

static const std::string input = cinput;
static const std::string label = clabel;
static const std::string target = ctarget;
static const std::string enable = cenable;
static const std::string fault = cfault;
}

namespace type
{
static constexpr auto cfan = "fan";
static constexpr auto ctemp = "temp";
static constexpr auto cvolt = "in";
static constexpr auto ccurr = "curr";
static constexpr auto cenergy = "energy";
static constexpr auto cpower = "power";
static constexpr auto cpwm = "pwm";


static const std::string fan = cfan;
static const std::string temp = ctemp;
static const std::string volt = cvolt;
static const std::string curr = ccurr;
static const std::string energy = cenergy;
static const std::string power = cpower;
static const std::string pwm = cpwm;
}
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
