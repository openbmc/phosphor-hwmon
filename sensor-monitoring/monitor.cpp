#include "monitor.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

Monitor::Monitor(sdbusplus::bus::bus& bus) :
    bus(bus)
{
    // Process thru given events that are type 'signal'
    for (auto& event : events)
    {
        for (auto pEvent : std::get<std::vector<std::shared_ptr<Event>>>(event))
        {
            if (pEvent->trigger != Event::Trigger::SIGNAL)
            {
                continue;
            }

            auto signalEvent = static_cast<SignalEvent*>(pEvent.get());
            eventArgs.emplace_back(std::make_unique<eventArg>(this,
                                                              signalEvent,
                                                              &event));
            matches.emplace_back(bus,
                                 signalEvent->signature,
                                 handleSignal,
                                 eventArgs.back().get());
        }
    }
}

void Monitor::processStart() noexcept
{
    sdbusplus::message::message nullMsg{nullptr};

    // Process thru given events that are type 'start'
    for (auto& event : events)
    {
        for (auto pEvent : std::get<std::vector<std::shared_ptr<Event>>>(event))
        {
            if (pEvent->trigger == Event::Trigger::START)
            {
                handleEvent(nullMsg, *pEvent, event);
            }
        }
    }
}

int Monitor::handleSignal(sd_bus_message* msg,
                          void* data,
                          sd_bus_error* err)
{
    auto sdbpMsg = sdbusplus::message::message(msg);
    auto& eventArg = *static_cast<Monitor::eventArg*>(data);
    std::get<0>(eventArg)->handleEvent(
        sdbpMsg,
        static_cast<const SignalEvent&>(*std::get<1>(eventArg)),
        *std::get<2>(eventArg));
    return 0;
}

void Monitor::handleEvent(sdbusplus::message::message& msg,
                          const Event& event,
                          const std::tuple<std::vector<std::shared_ptr<Event>>,
                                           std::vector<Action>>& eventDef)
{
    // Iterate over conditions
    for (auto& cond : event)
    {
        if (!cond(bus, msg, *this))
        {
            continue;
        }
        // Perform defined actions
        for (auto& act : std::get<1>(eventDef))
        {
            act(bus, *this);
        }
        return;
    }
}

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
