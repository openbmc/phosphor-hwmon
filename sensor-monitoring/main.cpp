#include <sdbusplus/bus.hpp>
#include "monitor.hpp"

int main(void)
{
    auto bus = sdbusplus::bus::new_default();

    phosphor::sensor::monitoring::Monitor monitor(bus);
    monitor.run();

    return 0;
}
