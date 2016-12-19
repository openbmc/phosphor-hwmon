#ifndef __HWMON_H
#define __HWMON_H

#include <string>

namespace hwmon
{
using namespace std::literals;

namespace entry
{
static const std::string input = "input"s;
}

namespace type
{
static const std::string fan = "fan"s;
static const std::string temp = "temp"s;
static const std::string volt = "in"s;
}
}

#endif
