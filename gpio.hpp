#pragma once

#include <chrono>
#include <gpioplus/chip.hpp>
#include <gpioplus/handle.hpp>
#include <memory>
#include <string>

namespace gpio
{

/**
 * Object created to encapsulate gpioplus objects.
 */
class GpioHandle
{
  public:
    static std::unique_ptr<GpioHandle>
        BuildGpioHandle(const std::string& gpiochip, const std::string& line);
    GpioHandle(gpioplus::Chip&& chip, gpioplus::HandleFlags&& flags,
               gpioplus::Handle&& handle) :
        chip(std::move(chip)),
        flags(std::move(flags)), handle(std::move(handle))
    {
    }

    void setValue(uint8_t v);

  private:
    gpioplus::Chip chip;
    gpioplus::HandleFlags flags;
    gpioplus::Handle handle;
    static constexpr std::chrono::milliseconds pause{500};
};
} // namespace gpio
