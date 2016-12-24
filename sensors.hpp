#pragma once

#include <string>
#include <stdexcept>

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
