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

}

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
