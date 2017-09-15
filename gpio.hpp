#pragma once

#include "sensorset.hpp"

#include <chrono>
#include <map>
#include <memory>
#include <string>

namespace gpio
{

class GpioManager
{
  public:
    /*
     * Initializes the data by getting the GPIO chip id.
     */
    GpioManager(const std::string& base);
    GpioManager(const GpioManager&) = delete;
    GpioManager& operator=(const GpioManager&) = delete;

    /**
     * Given a relative GPIO number and a sensor, export the GPIO
     * if necessary and set the direction to out.
     *
     */
    void setupGpio(int relGpio, const SensorSet::key_type& sensor);

    /*
     * Given a sensor, if it has a GPIO control associated, set the
     * GPIO to 1, and sleep for 500ms.
     */
    void unlockGpio(const SensorSet::key_type& sensor);

    /*
     * Given a sensor, if it has a GPIO control associated, set the
     * GPIO to 0.
     */
    void lockGpio(const SensorSet::key_type& sensor);

  private:
    static constexpr auto _gpioBase = "/sys/class/gpio/";
    static constexpr auto _gpioDirBase = "/sys/class/gpio/gpio";
    static constexpr auto _gpioExport = "/sys/class/gpio/export";
    static constexpr auto _gpioChip = "gpiochip";
    static constexpr std::chrono::milliseconds _pause{500};

    /* Helper method for getting the path to a GPIO based on its id. */
    std::string getGpioPath(int value);

    /* Given a path, write the string value to it. */
    void writeFile(const std::string& path, const std::string& value);

    /*
     * Actually set the GPIO and optionally pause afterwards.
     */
    void setGpio(const SensorSet::key_type& sensor, const std::string& value,
                 bool pause);

    std::map<SensorSet::key_type, std::string> _gpioAccess;
    int _baseChipId = 0;
};

} // namespace gpio

extern std::unique_ptr<gpio::GpioManager> pGpio;
