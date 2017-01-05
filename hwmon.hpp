#pragma once

#include <string>

namespace hwmon
{
namespace entry
{
static constexpr auto cinput = "input";
static constexpr auto clabel = "label";

static const std::string input = cinput;
static const std::string label = clabel;
}

namespace type
{
static constexpr auto cfan = "fan";
static constexpr auto ctemp = "temp";
static constexpr auto cvolt = "in";

static const std::string fan = cfan;
static const std::string temp = ctemp;
static const std::string volt = cvolt;
}
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
