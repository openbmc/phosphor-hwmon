#pragma once

#include <string>

namespace hwmon
{
using namespace std::literals;

namespace entry
{
static const std::string input = "input"s;
static const std::string label = "label"s;
}

namespace type
{
static const std::string fan = "fan"s;
static const std::string temp = "temp"s;
static const std::string volt = "in"s;
}
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
