#include "gpio_handle.hpp"

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
    /* TODO: consider checking if end returned is the end of the line to verify
     * the number found was the entire string.  This currently does no error
     * checking on the input.
     */
    unsigned long chipId = std::strtoul(gpiochip.c_str(), NULL, 10);
    unsigned long lineOffset = std::strtoul(line.c_str(), NULL, 10);

    try
    {
        gpioplus::Chip chip(chipId);
        gpioplus::HandleFlags flags(chip.getLineInfo(lineOffset).flags);
        flags.output = true;
        gpioplus::Handle handle(chip, {{static_cast<uint32_t>(lineOffset), 0}},
                                flags, "phosphor-hwmon");

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
