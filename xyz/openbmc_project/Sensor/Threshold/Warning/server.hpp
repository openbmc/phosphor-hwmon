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
namespace Threshold
{
namespace server
{

class Warning
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
        Warning() = delete;
        Warning(const Warning&) = delete;
        Warning& operator=(const Warning&) = delete;
        Warning(Warning&&) = default;
        Warning& operator=(Warning&&) = default;
        virtual ~Warning() = default;

        /** @brief Constructor to put object onto bus at a dbus path.
         *  @param[in] bus - Bus to attach to.
         *  @param[in] path - Path to attach at.
         */
        Warning(bus::bus& bus, const char* path);




        /** Get value of WarningHigh */
        virtual int64_t warningHigh() const;
        /** Set value of WarningHigh */
        virtual int64_t warningHigh(int64_t value);
        /** Get value of WarningLow */
        virtual int64_t warningLow() const;
        /** Set value of WarningLow */
        virtual int64_t warningLow(int64_t value);
        /** Get value of WarningAlarmHigh */
        virtual bool warningAlarmHigh() const;
        /** Set value of WarningAlarmHigh */
        virtual bool warningAlarmHigh(bool value);
        /** Get value of WarningAlarmLow */
        virtual bool warningAlarmLow() const;
        /** Set value of WarningAlarmLow */
        virtual bool warningAlarmLow(bool value);


    private:

        /** @brief sd-bus callback for get-property 'WarningHigh' */
        static int _callback_get_WarningHigh(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'WarningHigh' */
        static int _callback_set_WarningHigh(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);

        /** @brief sd-bus callback for get-property 'WarningLow' */
        static int _callback_get_WarningLow(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'WarningLow' */
        static int _callback_set_WarningLow(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);

        /** @brief sd-bus callback for get-property 'WarningAlarmHigh' */
        static int _callback_get_WarningAlarmHigh(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'WarningAlarmHigh' */
        static int _callback_set_WarningAlarmHigh(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);

        /** @brief sd-bus callback for get-property 'WarningAlarmLow' */
        static int _callback_get_WarningAlarmLow(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'WarningAlarmLow' */
        static int _callback_set_WarningAlarmLow(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);


        static constexpr auto _interface = "xyz.openbmc_project.Sensor.Threshold.Warning";
        static const vtable::vtable_t _vtable[];
        sdbusplus::server::interface::interface
                _xyz_openbmc_project_Sensor_Threshold_Warning_interface;

        int64_t _warningHigh{};
        int64_t _warningLow{};
        bool _warningAlarmHigh{};
        bool _warningAlarmLow{};

};


} // namespace server
} // namespace Threshold
} // namespace Sensor
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

