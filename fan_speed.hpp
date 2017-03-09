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
         * @param[in] sysfsRoot - The hwmon class root
         * @param[in] instance - The hwmon instance (ex. hwmon1)
         * @param[in] id - The hwmon id
         * @param[in] bus - Dbus bus object
         * @param[in] objPath - Dbus object path
         * @param[in] defer - Dbus object registration defer
         */
        FanSpeed(const std::string& sysfsRoot,
                 const std::string& instance,
                 const std::string& id,
                 sdbusplus::bus::bus& bus,
                 const char* objPath,
                 bool defer) : FanSpeedObject(bus, objPath, defer),
                    sysfsRoot(sysfsRoot),
                    instance(instance),
                    id(id)
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
        /** @brief hwmon class root */
        std::string sysfsRoot;
        /** @brief hwmon instance */
        std::string instance;
        /** @brief hwmon type */
        static constexpr auto type = "fan";
        /** @brief hwmon id */
        std::string id;
};

} // namespace hwmon
