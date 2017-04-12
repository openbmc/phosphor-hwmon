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

class Monitor
{
    public:
        Monitor() = delete;
        Monitor(const Monitor&) = delete;
        Monitor(Monitor&&) = default;
        Monitor& operator=(const Monitor&) = delete;
        Monitor& operator=(Monitor&&) = default;
        ~Monitor() = default;

        explicit Monitor(sdbusplus::bus::bus& bus);

    private:
        sdbusplus::bus::bus& bus;

};

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
