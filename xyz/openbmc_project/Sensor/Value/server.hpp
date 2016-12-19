#pragma once
#include <tuple>
#include <systemd/sd-bus.h>
#include <sdbusplus/server.hpp>

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

class Value
{
    public:
        /* Define all of the basic class operations:
         *     Not allowed:
         *         - Default constructor to avoid nullptrs.
         *         - Copy operations due to internal unique_ptr.
         *     Allowed:
         *         - Move operations.
         *         - Destructor.
         */
        Value() = delete;
        Value(const Value&) = delete;
        Value& operator=(const Value&) = delete;
        Value(Value&&) = default;
        Value& operator=(Value&&) = default;
        virtual ~Value() = default;

        /** @brief Constructor to put object onto bus at a dbus path.
         *  @param[in] bus - Bus to attach to.
         *  @param[in] path - Path to attach at.
         */
        Value(bus::bus& bus, const char* path);

        enum class Unit
        {
            DegreesC,
            RPMS,
            Volts,
            Meters,
        };



        /** Get value of Value */
        virtual int64_t value() const;
        /** Set value of Value */
        virtual int64_t value(int64_t value);
        /** Get value of Unit */
        virtual Unit unit() const;
        /** Set value of Unit */
        virtual Unit unit(Unit value);
        /** Get value of Scale */
        virtual int64_t scale() const;
        /** Set value of Scale */
        virtual int64_t scale(int64_t value);

    /** @brief Convert a string to an appropriate enum value.
     *  @param[in] s - The string to convert in the form of
     *                 "xyz.openbmc_project.Sensor.Value.<value name>"
     *  @return - The enum value.
     */
    static Unit convertUnitFromString(std::string& s);

    private:

        /** @brief sd-bus callback for get-property 'Value' */
        static int _callback_get_Value(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'Value' */
        static int _callback_set_Value(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);

        /** @brief sd-bus callback for get-property 'Unit' */
        static int _callback_get_Unit(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'Unit' */
        static int _callback_set_Unit(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);

        /** @brief sd-bus callback for get-property 'Scale' */
        static int _callback_get_Scale(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'Scale' */
        static int _callback_set_Scale(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);


        static constexpr auto _interface = "xyz.openbmc_project.Sensor.Value";
        static const vtable::vtable_t _vtable[];
        sdbusplus::server::interface::interface
                _xyz_openbmc_project_Sensor_Value_interface;

        int64_t _value{};
        Unit _unit{};
        int64_t _scale{};

};

/* Specialization of sdbusplus::server::bindings::details::convertForMessage
 * for enum-type Value::Unit.
 *
 * This converts from the enum to a constant c-string representing the enum.
 *
 * @param[in] e - Enum value to convert.
 * @return C-string representing the name for the enum value.
 */
std::string convertForMessage(Value::Unit e);

} // namespace server
} // namespace Sensor
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

