#include "gpio.hpp"

#include <cstdlib>
#include <memory>
#include <phosphor-logging/log.hpp>
#include <string>

namespace gpio
{

using namespace phosphor::logging;

std::unique_ptr<GpioHandle>
    GpioHandle::BuildGpioHandle(const std::string& gpiochip,
                                const std::string& line)
{
    int chipId = std::atoi(gpiochip.c_str());

    if (chipId < 0)
    {
        log<level::ERR>("Invalid chipId", entry("chipId: %d", chipId));
        return nullptr;
    }

    /* TODO: consider checking if end returned is the end of the line. */
    unsigned long lineInt = std::strtoul(line.c_str(), NULL, 10);
    uint32_t lineOffset = static_cast<uint32_t>(lineInt);

    try
    {
        gpioplus::Chip chip(static_cast<uint32_t>(chipId));
        gpioplus::HandleFlags flags(chip.getLineInfo(lineOffset).flags);
        flags.output = true;
        gpioplus::Handle handle(chip, {{lineOffset, 0}}, flags,
                                "phosphor-hwmon");

        return std::make_unique<GpioHandle>(std::move(chip), std::move(flags),
                                            std::move(handle));
    }
    catch (const std::exception& e)
    {
        log<level::ERR>("Unable to set up GPIO handle",
                        entry("ERROR: %s", e.what()));
        return nullptr;
    }
}

void GpioHandle::setValue(uint8_t v)
{
    handle.setValues({v});
}
} // namespace gpio
