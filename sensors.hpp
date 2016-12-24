#pragma once

#include <string>
#include <stdexcept>
#include <sensors/sensors.h>
#include <vector>

namespace libsensors
{

/** @class LibSensorsException
 *  @brief For non-zero return codes from libsensor APIs.
 */
struct LibSensorsException: public std::runtime_error
{
    LibSensorsException(const LibSensorsException&) = default;
    LibSensorsException& operator=(const LibSensorsException&) = default;
    LibSensorsException(LibSensorsException&&) = default;
    LibSensorsException& operator=(LibSensorsException&&) = default;
    ~LibSensorsException() = default;
    LibSensorsException() = delete;
    explicit LibSensorsException(int rc)
        : std::runtime_error(
              std::string(
                  "Unexpected libsensors error: ") + std::to_string(rc)) {}
};

namespace details
{

using ChipName = const sensors_chip_name *;
} // namespace details

/** @class Chip
 *  @brief Provide C++ bindings to libsensor chip_name APIs.
 */
class Chip final
{
    public:
        Chip& operator=(const Chip&) = default;
        Chip(Chip&&) = default;
        Chip(const Chip&) = default;
        Chip& operator=(Chip&&) = default;
        Chip() = delete;
        ~Chip() = default;

        /** @brief path
         *
         *  Provide access to sensor_chip_name.path.
         *
         *  @return The chip hwmon sysfs path.
         */
        std::string path() const;

    private:
        /** @brief Constructor
         *
         *  Cannot be constructed directly.  Obtain Chip instances
         *  via the LibSensors class.
         */
        Chip(details::ChipName chip) noexcept : chip(chip) {}

        /** @brief The libsensors chip handle. */
        details::ChipName chip;

        /** @brief Allow construction by LibSensors. */
        friend class LibSensors;
};

/** @class LibSensors
 *  @brief Provides C++ bindings to libsensor APIs.
 */
class LibSensors final
{
    public:
        LibSensors(const LibSensors&) = delete;
        LibSensors& operator=(const LibSensors&) = delete;
        LibSensors(LibSensors&&) = default;
        LibSensors& operator=(LibSensors&&) = default;
        ~LibSensors() noexcept;

        /** @brief chips
         *
         *  C++ Adaptation of sensors_get_detected_chips.
         *
         *  @return Detected chips.
         */
        std::vector<Chip> chips() const;

    private:
        /** @brief Default constructor
         *
         *  Cannot construct directly.  Use loadDefault to obtain a
         *  library handle.
         */
        LibSensors();

        /** @brief The number of times the library has been opened. */
        static size_t useCount;

        /** @brief Allow construction by loadDefault. */
        friend LibSensors loadDefault();
};

/** @brief Obtain a library access handle. */
LibSensors loadDefault();
} // namespace libsensors

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
