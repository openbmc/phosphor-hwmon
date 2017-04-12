#include <sdbusplus/bus.hpp>
#include "monitor.hpp"

int main(void)
{
    auto bus = sdbusplus::bus::new_default();

    phosphor::sensor::monitoring::Monitor monitor(bus);

    // Keep application running
    while (true)
    {
        bus.process_discard();
        bus.wait();
    }

    return 0;
}
