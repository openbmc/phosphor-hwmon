#pragma once

#include <string>

namespace hwmon
{
namespace entry
{
static constexpr auto cinput = "input";
static constexpr auto clabel = "label";
static constexpr auto ctarget = "target";

static const std::string input = cinput;
static const std::string label = clabel;
static const std::string target = ctarget;
}

namespace type
{
static constexpr auto cfan = "fan";
static constexpr auto ctemp = "temp";
static constexpr auto cvolt = "in";
static constexpr auto ccurr = "current";
static constexpr auto cenergy = "energy";
static constexpr auto cpower = "power";

static const std::string fan = cfan;
static const std::string temp = ctemp;
static const std::string volt = cvolt;
static const std::string curr = ccurr;
static const std::string energy = cenergy;
static const std::string power = cpower;
}
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
