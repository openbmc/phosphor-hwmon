#pragma once

#include "interface.hpp"

namespace hwmon
{

/**
 * @class FanSpeed
 * @brief Target fan speed control implementation
 * @details Derived FanSpeedObject type that writes the target value to sysfs
 * which in turn sets the fan speed to that target value
 */
class FanSpeed : public FanSpeedObject
{
    public:

        /**
         * @brief Constructs FanSpeed Object
         *
         * @param[in] targetPath - Target instance path
         * @param[in] sysfsFullPath - Full sysfs path of target instance
         * @param[in] bus - Dbus bus object
         * @param[in] objPath - Dbus object path
         * @param[in] defer - Dbus object registration defer
         */
        FanSpeed(const std::string& targetPath,
                 const std::string& sysfsFullPath,
                 sdbusplus::bus::bus& bus,
                 const char* objPath,
                 bool defer) : FanSpeedObject(bus, objPath, defer),
                    targetPath(targetPath),
                    sysfsFullPath(sysfsFullPath)
        {
            // Nothing to do here
        }

        /**
         * @brief Set the value of target
         *
         * @return Value of target
         */
        uint64_t target(uint64_t value) override;

    private:
        /** @brief Target instance path */
        std::string targetPath;
        /** @brief Full sysfs path of the target instance */
        std::string sysfsFullPath;
};

} // namespace hwmon
