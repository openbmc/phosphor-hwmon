#include <algorithm>
#include <sdbusplus/server.hpp>
#include <sdbusplus/exception.hpp>
#include <xyz/openbmc_project/Sensor/Threshold/Warning/server.hpp>

namespace sdbusplus
{
namespace xyz
{
namespace openbmc_project
{
namespace Sensor
{
namespace Threshold
{
namespace server
{

Warning::Warning(bus::bus& bus, const char* path)
        : _xyz_openbmc_project_Sensor_Threshold_Warning_interface(
                bus, path, _interface, _vtable, this)
{
}



auto Warning::warningHigh() const ->
        int64_t
{
    return _warningHigh;
}

int Warning::_callback_get_WarningHigh(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Warning*>(context);
        m.append(convertForMessage(o->warningHigh()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Warning::warningHigh(int64_t value) ->
        int64_t
{
    if (_warningHigh != value)
    {
        _warningHigh = value;
        _xyz_openbmc_project_Sensor_Threshold_Warning_interface.property_changed("WarningHigh");
    }

    return _warningHigh;
}

int Warning::_callback_set_WarningHigh(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Warning*>(context);

        int64_t v{};
        m.read(v);
        o->warningHigh(v);
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

namespace details
{
namespace Warning
{
static const auto _property_WarningHigh =
    utility::tuple_to_array(message::types::type_id<
            int64_t>());
}
}
auto Warning::warningLow() const ->
        int64_t
{
    return _warningLow;
}

int Warning::_callback_get_WarningLow(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Warning*>(context);
        m.append(convertForMessage(o->warningLow()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Warning::warningLow(int64_t value) ->
        int64_t
{
    if (_warningLow != value)
    {
        _warningLow = value;
        _xyz_openbmc_project_Sensor_Threshold_Warning_interface.property_changed("WarningLow");
    }

    return _warningLow;
}

int Warning::_callback_set_WarningLow(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Warning*>(context);

        int64_t v{};
        m.read(v);
        o->warningLow(v);
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

namespace details
{
namespace Warning
{
static const auto _property_WarningLow =
    utility::tuple_to_array(message::types::type_id<
            int64_t>());
}
}
auto Warning::warningAlarmHigh() const ->
        bool
{
    return _warningAlarmHigh;
}

int Warning::_callback_get_WarningAlarmHigh(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Warning*>(context);
        m.append(convertForMessage(o->warningAlarmHigh()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Warning::warningAlarmHigh(bool value) ->
        bool
{
    if (_warningAlarmHigh != value)
    {
        _warningAlarmHigh = value;
        _xyz_openbmc_project_Sensor_Threshold_Warning_interface.property_changed("WarningAlarmHigh");
    }

    return _warningAlarmHigh;
}

int Warning::_callback_set_WarningAlarmHigh(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Warning*>(context);

        bool v{};
        m.read(v);
        o->warningAlarmHigh(v);
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

namespace details
{
namespace Warning
{
static const auto _property_WarningAlarmHigh =
    utility::tuple_to_array(message::types::type_id<
            bool>());
}
}
auto Warning::warningAlarmLow() const ->
        bool
{
    return _warningAlarmLow;
}

int Warning::_callback_get_WarningAlarmLow(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Warning*>(context);
        m.append(convertForMessage(o->warningAlarmLow()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Warning::warningAlarmLow(bool value) ->
        bool
{
    if (_warningAlarmLow != value)
    {
        _warningAlarmLow = value;
        _xyz_openbmc_project_Sensor_Threshold_Warning_interface.property_changed("WarningAlarmLow");
    }

    return _warningAlarmLow;
}

int Warning::_callback_set_WarningAlarmLow(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Warning*>(context);

        bool v{};
        m.read(v);
        o->warningAlarmLow(v);
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

namespace details
{
namespace Warning
{
static const auto _property_WarningAlarmLow =
    utility::tuple_to_array(message::types::type_id<
            bool>());
}
}


const vtable::vtable_t Warning::_vtable[] = {
    vtable::start(),
    vtable::property("WarningHigh",
                     details::Warning::_property_WarningHigh
                        .data(),
                     _callback_get_WarningHigh,
                     _callback_set_WarningHigh,
                     vtable::property_::emits_change),
    vtable::property("WarningLow",
                     details::Warning::_property_WarningLow
                        .data(),
                     _callback_get_WarningLow,
                     _callback_set_WarningLow,
                     vtable::property_::emits_change),
    vtable::property("WarningAlarmHigh",
                     details::Warning::_property_WarningAlarmHigh
                        .data(),
                     _callback_get_WarningAlarmHigh,
                     _callback_set_WarningAlarmHigh,
                     vtable::property_::emits_change),
    vtable::property("WarningAlarmLow",
                     details::Warning::_property_WarningAlarmLow
                        .data(),
                     _callback_get_WarningAlarmLow,
                     _callback_set_WarningAlarmLow,
                     vtable::property_::emits_change),
    vtable::end()
};

} // namespace server
} // namespace Threshold
} // namespace Sensor
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

