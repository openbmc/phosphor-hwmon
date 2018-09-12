#include "timer.hpp"

#include <chrono>
#include <cstring>
#include <system_error>

namespace phosphor
{
namespace hwmon
{

static std::chrono::microseconds getTime()
{
    using namespace std::chrono;
    auto usec = steady_clock::now().time_since_epoch();
    return duration_cast<microseconds>(usec);
}

Timer::Timer(sd_event* event, std::function<void()> callback,
             std::chrono::microseconds usec, timer::Action action) :
    event(event),
    callback(callback), duration(usec), action(action)
{
    auto r = sd_event_add_time(event, &eventSource,
                               CLOCK_MONOTONIC,            // Time base
                               (getTime() + usec).count(), // When to fire
                               0,              // Use default event accuracy
                               timeoutHandler, // Callback handler on timeout
                               this);          // User data
    if (r < 0)
    {
        throw std::system_error(r, std::generic_category(), std::strerror(-r));
    }
}

int Timer::timeoutHandler(sd_event_source* eventSource, uint64_t usec,
                          void* userData)
{
    auto timer = static_cast<Timer*>(userData);

    if (timer->getAction() == timer::ON)
    {
        auto r = sd_event_source_set_time(
            eventSource, (getTime() + timer->getDuration()).count());
        if (r < 0)
        {
            throw std::system_error(r, std::generic_category(),
                                    std::strerror(-r));
        }
        r = sd_event_source_set_enabled(eventSource, timer::ON);
        if (r < 0)
        {
            throw std::system_error(r, std::generic_category(),
                                    std::strerror(-r));
        }
    }

    if (timer->callback)
    {
        timer->callback();
    }

    return 0;
}

} // namespace hwmon
} // namespace phosphor
