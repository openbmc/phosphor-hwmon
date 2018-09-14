#pragma once

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
    /**
     * Method called to validate inputs and create a GpioHandle.
     *
     * @param[in] gpiochip - gpiochip id as string, e.g. "0", or "1"
     * @param[in] line - gpio line offset as string.
     * @return A GpioHandle on success nullptr on failure.
     */
    static std::unique_ptr<GpioHandle>
        BuildGpioHandle(const std::string& gpiochip, const std::string& line);

    /* Delete copy constructor and assignment operator and default constructor
     * to use move operations.
     */
    GpioHandle() = delete;
    GpioHandle(const GpioHandle&) = delete;
    GpioHandle(GpioHandle&&) = default;
    GpioHandle& operator=(const GpioHandle&) = delete;
    GpioHandle& operator=(GpioHandle&&) = default;
    ~GpioHandle() = default;

    /**
     * Constructor for GpioHandler
     *
     * @param[in] chip - a gpioplus::Chip to use
     * @param[in] flags - a gpioplus::HandleFlags to use
     * @param[in] handle - a gpioplus::Handle to use
     */
    GpioHandle(gpioplus::Chip&& chip, gpioplus::HandleFlags&& flags,
               gpioplus::Handle&& handle) :
        chip(std::move(chip)),
        flags(std::move(flags)), handle(std::move(handle))
    {
    }

    /**
     * Given a value, set that value for the gpio line.
     *
     * @param[in] v - the value (1 or 0).
     */
    void setValue(uint8_t v);

  private:
    /** @brief the gpioplus Chip object to hold. */
    gpioplus::Chip chip;

    /** @brief the gpioplus flags object to hold. */
    gpioplus::HandleFlags flags;

    /** @brief the gpioplus handle we use to set values. */
    gpioplus::Handle handle;
};
} // namespace gpio
