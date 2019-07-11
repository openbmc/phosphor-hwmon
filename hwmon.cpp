#include "hwmon.hpp"

#include "env.hpp"

namespace hwmon
{

bool getAttributes(const std::string& type, Attributes& attributes)
{
    // clang-format off
    auto a =
        std::find_if(
            typeAttrMap.begin(),
            typeAttrMap.end(),
            [&](const auto& e)
            {
                return type == getHwmonType(e);
            });
    // clang-format on

    if (a == typeAttrMap.end())
    {
        return false;
    }

    attributes = *a;
    return true;
}

std::string getValueType(const SensorSet::key_type& sensor)
{
    auto type = hwmon::entry::cinput; // Default type is input
    if (hwmon::type::cpower == sensor.first)
    {
        auto average = env::getEnv("AVERAGE", sensor.first, sensor.second);
        if ("true" == average)
        {
            type = hwmon::entry::caverage;
        }
    }
    return type;
}

} //  namespace hwmon
