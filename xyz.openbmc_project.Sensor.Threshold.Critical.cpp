#include <algorithm>
#include <sdbusplus/server.hpp>
#include <sdbusplus/exception.hpp>
#include <xyz/openbmc_project/Sensor/Threshold/Critical/server.hpp>

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

Critical::Critical(bus::bus& bus, const char* path)
        : _xyz_openbmc_project_Sensor_Threshold_Critical_interface(
                bus, path, _interface, _vtable, this)
{
}



auto Critical::criticalHigh() const ->
        int64_t
{
    return _criticalHigh;
}

int Critical::_callback_get_CriticalHigh(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Critical*>(context);
        m.append(convertForMessage(o->criticalHigh()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Critical::criticalHigh(int64_t value) ->
        int64_t
{
    if (_criticalHigh != value)
    {
        _criticalHigh = value;
        _xyz_openbmc_project_Sensor_Threshold_Critical_interface.property_changed("CriticalHigh");
    }

    return _criticalHigh;
}

int Critical::_callback_set_CriticalHigh(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Critical*>(context);

        int64_t v{};
        m.read(v);
        o->criticalHigh(v);
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
namespace Critical
{
static const auto _property_CriticalHigh =
    utility::tuple_to_array(message::types::type_id<
            int64_t>());
}
}
auto Critical::criticalLow() const ->
        int64_t
{
    return _criticalLow;
}

int Critical::_callback_get_CriticalLow(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Critical*>(context);
        m.append(convertForMessage(o->criticalLow()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Critical::criticalLow(int64_t value) ->
        int64_t
{
    if (_criticalLow != value)
    {
        _criticalLow = value;
        _xyz_openbmc_project_Sensor_Threshold_Critical_interface.property_changed("CriticalLow");
    }

    return _criticalLow;
}

int Critical::_callback_set_CriticalLow(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Critical*>(context);

        int64_t v{};
        m.read(v);
        o->criticalLow(v);
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
namespace Critical
{
static const auto _property_CriticalLow =
    utility::tuple_to_array(message::types::type_id<
            int64_t>());
}
}
auto Critical::criticalAlarmHigh() const ->
        bool
{
    return _criticalAlarmHigh;
}

int Critical::_callback_get_CriticalAlarmHigh(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Critical*>(context);
        m.append(convertForMessage(o->criticalAlarmHigh()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Critical::criticalAlarmHigh(bool value) ->
        bool
{
    if (_criticalAlarmHigh != value)
    {
        _criticalAlarmHigh = value;
        _xyz_openbmc_project_Sensor_Threshold_Critical_interface.property_changed("CriticalAlarmHigh");
    }

    return _criticalAlarmHigh;
}

int Critical::_callback_set_CriticalAlarmHigh(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Critical*>(context);

        bool v{};
        m.read(v);
        o->criticalAlarmHigh(v);
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
namespace Critical
{
static const auto _property_CriticalAlarmHigh =
    utility::tuple_to_array(message::types::type_id<
            bool>());
}
}
auto Critical::criticalAlarmLow() const ->
        bool
{
    return _criticalAlarmLow;
}

int Critical::_callback_get_CriticalAlarmLow(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Critical*>(context);
        m.append(convertForMessage(o->criticalAlarmLow()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Critical::criticalAlarmLow(bool value) ->
        bool
{
    if (_criticalAlarmLow != value)
    {
        _criticalAlarmLow = value;
        _xyz_openbmc_project_Sensor_Threshold_Critical_interface.property_changed("CriticalAlarmLow");
    }

    return _criticalAlarmLow;
}

int Critical::_callback_set_CriticalAlarmLow(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Critical*>(context);

        bool v{};
        m.read(v);
        o->criticalAlarmLow(v);
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
namespace Critical
{
static const auto _property_CriticalAlarmLow =
    utility::tuple_to_array(message::types::type_id<
            bool>());
}
}


const vtable::vtable_t Critical::_vtable[] = {
    vtable::start(),
    vtable::property("CriticalHigh",
                     details::Critical::_property_CriticalHigh
                        .data(),
                     _callback_get_CriticalHigh,
                     _callback_set_CriticalHigh,
                     vtable::property_::emits_change),
    vtable::property("CriticalLow",
                     details::Critical::_property_CriticalLow
                        .data(),
                     _callback_get_CriticalLow,
                     _callback_set_CriticalLow,
                     vtable::property_::emits_change),
    vtable::property("CriticalAlarmHigh",
                     details::Critical::_property_CriticalAlarmHigh
                        .data(),
                     _callback_get_CriticalAlarmHigh,
                     _callback_set_CriticalAlarmHigh,
                     vtable::property_::emits_change),
    vtable::property("CriticalAlarmLow",
                     details::Critical::_property_CriticalAlarmLow
                        .data(),
                     _callback_get_CriticalAlarmLow,
                     _callback_set_CriticalAlarmLow,
                     vtable::property_::emits_change),
    vtable::end()
};

} // namespace server
} // namespace Threshold
} // namespace Sensor
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

