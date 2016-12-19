#include <algorithm>
#include <sdbusplus/server.hpp>
#include <sdbusplus/exception.hpp>
#include <xyz/openbmc_project/Sensor/Value/server.hpp>

namespace sdbusplus
{
namespace xyz
{
namespace openbmc_project
{
namespace Sensor
{
namespace server
{

Value::Value(bus::bus& bus, const char* path)
        : _xyz_openbmc_project_Sensor_Value_interface(
                bus, path, _interface, _vtable, this)
{
}



auto Value::value() const ->
        int64_t
{
    return _value;
}

int Value::_callback_get_Value(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Value*>(context);
        m.append(convertForMessage(o->value()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Value::value(int64_t value) ->
        int64_t
{
    if (_value != value)
    {
        _value = value;
        _xyz_openbmc_project_Sensor_Value_interface.property_changed("Value");
    }

    return _value;
}

int Value::_callback_set_Value(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Value*>(context);

        int64_t v{};
        m.read(v);
        o->value(v);
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
namespace Value
{
static const auto _property_Value =
    utility::tuple_to_array(message::types::type_id<
            int64_t>());
}
}
auto Value::unit() const ->
        Unit
{
    return _unit;
}

int Value::_callback_get_Unit(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Value*>(context);
        m.append(convertForMessage(o->unit()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Value::unit(Unit value) ->
        Unit
{
    if (_unit != value)
    {
        _unit = value;
        _xyz_openbmc_project_Sensor_Value_interface.property_changed("Unit");
    }

    return _unit;
}

int Value::_callback_set_Unit(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Value*>(context);

        std::string v{};
        m.read(v);
        o->unit(convertUnitFromString(v));
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
namespace Value
{
static const auto _property_Unit =
    utility::tuple_to_array(message::types::type_id<
            std::string>());
}
}
auto Value::scale() const ->
        int64_t
{
    return _scale;
}

int Value::_callback_get_Scale(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* reply, void* context,
        sd_bus_error* error)
{
    using sdbusplus::server::binding::details::convertForMessage;

    try
    {
        auto m = message::message(sd_bus_message_ref(reply));

        auto o = static_cast<Value*>(context);
        m.append(convertForMessage(o->scale()));
    }
    catch(sdbusplus::internal_exception_t& e)
    {
        sd_bus_error_set_const(error, e.name(), e.description());
        return -EINVAL;
    }

    return true;
}

auto Value::scale(int64_t value) ->
        int64_t
{
    if (_scale != value)
    {
        _scale = value;
        _xyz_openbmc_project_Sensor_Value_interface.property_changed("Scale");
    }

    return _scale;
}

int Value::_callback_set_Scale(
        sd_bus* bus, const char* path, const char* interface,
        const char* property, sd_bus_message* value, void* context,
        sd_bus_error* error)
{
    try
    {
        auto m = message::message(sd_bus_message_ref(value));

        auto o = static_cast<Value*>(context);

        int64_t v{};
        m.read(v);
        o->scale(v);
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
namespace Value
{
static const auto _property_Scale =
    utility::tuple_to_array(message::types::type_id<
            int64_t>());
}
}


namespace
{
/** String to enum mapping for Value::Unit */
static const std::tuple<const char*, Value::Unit> mappingValueUnit[] =
        {
            std::make_tuple( "xyz.openbmc_project.Sensor.Value.Unit.DegreesC",                 Value::Unit::DegreesC ),
            std::make_tuple( "xyz.openbmc_project.Sensor.Value.Unit.RPMS",                 Value::Unit::RPMS ),
            std::make_tuple( "xyz.openbmc_project.Sensor.Value.Unit.Volts",                 Value::Unit::Volts ),
            std::make_tuple( "xyz.openbmc_project.Sensor.Value.Unit.Meters",                 Value::Unit::Meters ),
        };

} // anonymous namespace

auto Value::convertUnitFromString(std::string& s) ->
        Unit
{
    auto i = std::find_if(
            std::begin(mappingValueUnit),
            std::end(mappingValueUnit),
            [&s](auto& e){ return 0 == strcmp(s.c_str(), std::get<0>(e)); } );
    if (std::end(mappingValueUnit) == i)
    {
        throw sdbusplus::exception::InvalidEnumString();
    }
    else
    {
        return std::get<1>(*i);
    }
}

std::string convertForMessage(Value::Unit v)
{
    auto i = std::find_if(
            std::begin(mappingValueUnit),
            std::end(mappingValueUnit),
            [v](auto& e){ return v == std::get<1>(e); });
    return std::get<0>(*i);
}

const vtable::vtable_t Value::_vtable[] = {
    vtable::start(),
    vtable::property("Value",
                     details::Value::_property_Value
                        .data(),
                     _callback_get_Value,
                     _callback_set_Value,
                     vtable::property_::emits_change),
    vtable::property("Unit",
                     details::Value::_property_Unit
                        .data(),
                     _callback_get_Unit,
                     _callback_set_Unit,
                     vtable::property_::emits_change),
    vtable::property("Scale",
                     details::Value::_property_Scale
                        .data(),
                     _callback_get_Scale,
                     _callback_set_Scale,
                     vtable::property_::emits_change),
    vtable::end()
};

} // namespace server
} // namespace Sensor
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

