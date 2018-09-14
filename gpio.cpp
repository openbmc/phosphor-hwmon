#include "gpio.hpp"

#include <cstdlib>
#include <memory>
#include <phosphor-logging/log.hpp>
#include <string>
#include <thread>

namespace gpio
{

using namespace phosphor::logging;

std::unique_ptr<GpioHandle>
    GpioHandle::BuildGpioHandle(const std::string& gpiochip,
                                const std::string& line)
{
    int chip_id = std::atoi(gpiochip.c_str());
    int line_int = std::atoi(line.c_str());

    if (chip_id < 0 || line_int < 0)
    {
        log<level::ERR>("Invalid chip_id or line_offset",
                        entry("CHIP_ID: %d", chip_id),
                        entry("LINE_OFFSET: %d", line_int));
        return nullptr;
    }

    try
    {
        uint32_t line_offset = static_cast<uint32_t>(line_int);

        gpioplus::Chip chip(static_cast<uint32_t>(chip_id));
        gpioplus::HandleFlags flags(chip.getLineInfo(line_offset).flags);
        flags.output = true;
        gpioplus::Handle handle(chip, {{line_offset, 0}}, flags,
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
    std::this_thread::sleep_for(pause);
}
} // namespace gpio
