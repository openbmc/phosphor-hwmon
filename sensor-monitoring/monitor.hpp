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
        Monitor& operator=(Monitor&&) = delete;
        ~Monitor() = default;

        Monitor(sdbusplus::bus::bus& bus);

        void run() noexcept;

        void log_error(const char* msg);

    private:
        sdbusplus::bus::bus& bus;

};

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
