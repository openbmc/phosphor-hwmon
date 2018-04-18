#include "hwmon.hpp"

namespace hwmon {

bool getAttributes(const std::string& type, Attributes& attributes)
{
    // *INDENT-OFF*
    auto a = std::find_if(
                typeAttrMap.begin(),
                typeAttrMap.end(),
                [&](const auto & e)
                {
                   return type == getHwmonType(e);
                });
    // *INDENT-ON*

    if (a == typeAttrMap.end())
    {
        return false;
    }

    attributes = *a;
    return true;
}

}  //  namespace hwmon
