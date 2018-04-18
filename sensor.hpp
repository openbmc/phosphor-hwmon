#pragma once

#include <string>
#include <tuple>

#include "hwmon.hpp"
#include "interface.hpp"

namespace sensor {

static constexpr auto typeAttrMap =
{
    // 1 - hwmon class
    // 2 - unit
    // 3 - sysfs scaling factor
    // 4 - namespace
    std::make_tuple(
        hwmon::type::ctemp,
        ValueInterface::Unit::DegreesC,
        -3,
        "temperature"),
    std::make_tuple(
        hwmon::type::cfan,
        ValueInterface::Unit::RPMS,
        0,
        "fan_tach"),
    std::make_tuple(
        hwmon::type::cvolt,
        ValueInterface::Unit::Volts,
        -3,
        "voltage"),
    std::make_tuple(
        hwmon::type::ccurr,
        ValueInterface::Unit::Amperes,
        -3,
        "current"),
    std::make_tuple(
        hwmon::type::cenergy,
        ValueInterface::Unit::Joules,
        -6,
        "energy"),
    std::make_tuple(
        hwmon::type::cpower,
        ValueInterface::Unit::Watts,
        -6,
        "power"),
};

inline auto getHwmonType(decltype(typeAttrMap)::const_reference attrs)
{
    return std::get<0>(attrs);
}

inline auto getUnit(decltype(typeAttrMap)::const_reference attrs)
{
    return std::get<1>(attrs);
}

inline auto getScale(decltype(typeAttrMap)::const_reference attrs)
{
    return std::get<2>(attrs);
}

inline auto getNamespace(decltype(typeAttrMap)::const_reference attrs)
{
    return std::get<3>(attrs);
}

using AttributeIterator = decltype(*typeAttrMap.begin());
using Attributes
    = std::remove_cv<std::remove_reference<AttributeIterator>::type>::type;

/** @brief Get Attribute tuple for the type
 *
 *  Given a type, it tries to find the corresponding tuple
 *
 *  @param[in] type the sensor type
 *  @param[in,out] A pointer to the Attribute tuple
 */
bool getAttributes(const std::string& type, Attributes& attributes);

}  // sensor

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
