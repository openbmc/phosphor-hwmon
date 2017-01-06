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

class Critical
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
        Critical() = delete;
        Critical(const Critical&) = delete;
        Critical& operator=(const Critical&) = delete;
        Critical(Critical&&) = default;
        Critical& operator=(Critical&&) = default;
        virtual ~Critical() = default;

        /** @brief Constructor to put object onto bus at a dbus path.
         *  @param[in] bus - Bus to attach to.
         *  @param[in] path - Path to attach at.
         */
        Critical(bus::bus& bus, const char* path);




        /** Get value of CriticalHigh */
        virtual int64_t criticalHigh() const;
        /** Set value of CriticalHigh */
        virtual int64_t criticalHigh(int64_t value);
        /** Get value of CriticalLow */
        virtual int64_t criticalLow() const;
        /** Set value of CriticalLow */
        virtual int64_t criticalLow(int64_t value);
        /** Get value of CriticalAlarmHigh */
        virtual bool criticalAlarmHigh() const;
        /** Set value of CriticalAlarmHigh */
        virtual bool criticalAlarmHigh(bool value);
        /** Get value of CriticalAlarmLow */
        virtual bool criticalAlarmLow() const;
        /** Set value of CriticalAlarmLow */
        virtual bool criticalAlarmLow(bool value);


    private:

        /** @brief sd-bus callback for get-property 'CriticalHigh' */
        static int _callback_get_CriticalHigh(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'CriticalHigh' */
        static int _callback_set_CriticalHigh(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);

        /** @brief sd-bus callback for get-property 'CriticalLow' */
        static int _callback_get_CriticalLow(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'CriticalLow' */
        static int _callback_set_CriticalLow(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);

        /** @brief sd-bus callback for get-property 'CriticalAlarmHigh' */
        static int _callback_get_CriticalAlarmHigh(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'CriticalAlarmHigh' */
        static int _callback_set_CriticalAlarmHigh(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);

        /** @brief sd-bus callback for get-property 'CriticalAlarmLow' */
        static int _callback_get_CriticalAlarmLow(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);
        /** @brief sd-bus callback for set-property 'CriticalAlarmLow' */
        static int _callback_set_CriticalAlarmLow(
            sd_bus*, const char*, const char*, const char*,
            sd_bus_message*, void*, sd_bus_error*);


        static constexpr auto _interface = "xyz.openbmc_project.Sensor.Threshold.Critical";
        static const vtable::vtable_t _vtable[];
        sdbusplus::server::interface::interface
                _xyz_openbmc_project_Sensor_Threshold_Critical_interface;

        int64_t _criticalHigh{};
        int64_t _criticalLow{};
        bool _criticalAlarmHigh{};
        bool _criticalAlarmLow{};

};


} // namespace server
} // namespace Threshold
} // namespace Sensor
} // namespace openbmc_project
} // namespace xyz
} // namespace sdbusplus

