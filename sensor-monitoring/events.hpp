#pragma once

#include "data_types.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

/**
 * @class Event
 * @brief A sensor monitoring triggered event
 * @details An event with an associated list of conditions to check
 */
class Event : public std::vector<Condition>
{
    public:
        /**
         * @brief Types of triggers of the event
         */
        enum class Trigger
        {
            UNKNOWN,
            START,
            SIGNAL
        };

        Event() = delete;
        Event(const Event&) = delete;
        Event(Event&&) = delete;
        Event& operator=(const Event&) = delete;
        Event& operator=(Event&&) = delete;
        virtual ~Event() = default;

        /**
         * @brief Constructs an event with given conditions and trigger
         *
         * @param[in] conditions - Conditions for the event
         * @param[in] t - Type of trigger of the event
         */
        Event(const std::vector<Condition>& conditions,
              Trigger t = Trigger::UNKNOWN) :
                  std::vector<Condition>(conditions),
                  trigger(t)
        {
            // Nothing to do here
        }

        /** @brief Event trigger type */
        Trigger trigger;
};

class StartEvent : public Event
{
    public:
        StartEvent() = delete;
        StartEvent(const StartEvent&) = delete;
        StartEvent(StartEvent&&) = delete;
        StartEvent& operator=(const StartEvent&) = delete;
        StartEvent& operator=(StartEvent&&) = delete;
        ~StartEvent() = default;

        /**
         * @brief Constructs a derived application started event
         *
         * @param[in] conditions - Conditions for the event
         */
        StartEvent(const std::vector<Condition>& conditions) :
            Event(conditions, Trigger::START)
        {
            // Nothing to do here
        }
};

class SignalEvent : public Event
{
    public:
        SignalEvent() = delete;
        SignalEvent(const SignalEvent&) = delete;
        SignalEvent(SignalEvent&&) = delete;
        SignalEvent& operator=(const SignalEvent&) = delete;
        SignalEvent& operator=(SignalEvent&&) = delete;
        ~SignalEvent() = default;

        /**
         * @brief Constructs a derived Dbus signal event
         *
         * @param[in] signature - Dbus object signature
         * @param[in] conditions - Conditions for the event
         */
        SignalEvent(const char* signature,
                    const std::vector<Condition>& conditions) :
                        Event(conditions, Trigger::SIGNAL),
                        signature(signature)
        {
            // Nothing to do here
        }

        /** @brief Dbus object signature */
        const char* signature;
};

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
