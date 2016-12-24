#pragma once

#include <string>
#include <stdexcept>
#include <sensors/sensors.h>
#include <vector>

namespace libsensors
{

/** @class FeatureException
 *  @brief For unsupported libsensors features.
 */
struct FeatureException : public std::runtime_error
{
    FeatureException(const FeatureException&) = default;
    FeatureException& operator=(const FeatureException&) = default;
    FeatureException(FeatureException&&) = default;
    FeatureException& operator=(FeatureException&&) = default;
    ~FeatureException() = default;
    FeatureException() = delete;
    explicit FeatureException(const char* feature)
        : std::runtime_error(
              std::string(
                  "Unexpected libsensors (sub)feature: ") + feature) {}
};

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
using Feature = const sensors_feature *;
} // namespace details

class Sensor;

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

        /** @brief sensors
         *
         *  C++ adapatation of sensors_get_features.
         *
         *  @return The sensors implemented by Chip.
         */
        std::vector<Sensor> sensors() const;

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
        /** @brief Feature APIs require Chip access. */
        friend class Sensor;
};

/** @class Sensor
 *  @brief Provide C++ bindings to libsensor feature APIs.
 */
class Sensor final
{
    public:
        Sensor(const Sensor&) = default;
        Sensor& operator=(const Sensor&) = default;
        Sensor(Sensor&&) = default;
        Sensor& operator=(Sensor&&) = default;
        Sensor() = delete;
        ~Sensor() = default;

        /** @brief type
         *
         *  Provide a string represenation of sensors_feature_type.
         *
         *  @return The string representation.
         */
        std::string type() const;

        /** @brief label
         *
         *  C++ adapatation of sensors_get_label.
         *
         *  @return The sensor label.
         */
        std::string label() const;

    private:
        /** @brief Constructor
         *
         *  Cannot be constructed directly.  Obtain Sensor instances
         *  via the Chip class.
         */
        Sensor(details::Feature feature, Chip chip) noexcept
            : feature(feature), chip(chip) {}

        /** @brief The libsensors feature handle. */
        details::Feature feature;
        /** @brief The parent chip. */
        Chip chip;

        /** @brief Allow construction by Chip. */
        friend class Chip;
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

namespace details
{

/** @struct MallocFree
 *  @brief A smart pointer deleter type.
 */
template <typename T>
struct MallocFree
{
    void operator()(T* ptr) const
    {
        free(ptr);
    }
};
} // namespace details
} // namespace libsensors

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
