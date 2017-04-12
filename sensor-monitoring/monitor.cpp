#include <phosphor-logging/log.hpp>
#include "monitor.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

using namespace phosphor::logging;

Monitor::Monitor(sdbusplus::bus::bus& bus) :
    bus(bus)
{

}

void Monitor::run() noexcept
{
    // Keep application running
    while (true)
    {
        bus.process_discard();
        bus.wait();
    }
}

void Monitor::log_error(const char* msg)
{
    log<level::ERR>(msg);
}

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
