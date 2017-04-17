#pragma once

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include "events.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

/**
 * @class Monitor
 * @brief OpenBMC Sensor Monitoring application
 * @details A configurable application to perform a set of actions based on one
 * or more conditions for sensors within a group
 */
class Monitor
{
    public:
        Monitor() = delete;
        Monitor(const Monitor&) = delete;
        Monitor(Monitor&&) = default;
        Monitor& operator=(const Monitor&) = delete;
        Monitor& operator=(Monitor&&) = delete;
        ~Monitor() = default;

        /**
         * @brief Constructs monitor object
         *
         * @param[in] bus - Dbus bus object
         */
        Monitor(sdbusplus::bus::bus& bus);

        /**
         * @brief Process events triggered by the application starting
         */
        void processStart() noexcept;

        /**
         * @brief Handle an event being processed
         *
         * @param[in] msg - Dbus msg
         * @param[in] event - Event to be handled
         * @param[in] eventDef - The event's full definition
         */
        void handleEvent(sdbusplus::message::message& msg,
                         const Event& event,
                         const std::tuple<std::vector<std::shared_ptr<Event>>,
                                    std::vector<Action>>& eventDef);

        /**
         * @brief Log an error
         *
         * @param[in] msg - Message to log
         */
        void log_error(const char* msg);

        /**
         * @brief An event's set of arguments
         */
        using eventArg = std::tuple<Monitor*,
                                    const SignalEvent*,
                                    const std::tuple<
                                        std::vector<std::shared_ptr<Event>>,
                                        std::vector<Action>>*>;

    private:
        /** @brief Connection for sdbusplus bus */
        sdbusplus::bus::bus& bus;
        /** @brief List of events to process */
        static const std::vector<
            std::tuple<std::vector<std::shared_ptr<Event>>,
                       std::vector<Action>>> events;
        /** @brief List of event arguments */
        std::vector<std::unique_ptr<eventArg>> eventArgs;
        /** @brief list of Dbus matches for callbacks */
        std::vector<sdbusplus::server::match::match> matches;

        /**
         * @brief Handle an event signal
         *
         * @param[in] msg - Data associated with the subscribed signal
         * @param[in] data - Pointer to the event sensor's data
         * @param[in] err - Contains any sdbus error reference if occurred
         *
         * @return 0
         */
        static int handleSignal(sd_bus_message* msg,
                                void* data,
                                sd_bus_error* err);

};

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
