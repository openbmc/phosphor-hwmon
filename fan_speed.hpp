#pragma once

#include "interface.hpp"
#include "sysfs.hpp"

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
         * @param[in] instancePath - The hwmon instance path
         *     (ex. /sys/class/hwmon/hwmon1)
         * @param[in] devPath - The /sys/devices sysfs path
         * @param[in] id - The hwmon id
         * @param[in] bus - Dbus bus object
         * @param[in] objPath - Dbus object path
         * @param[in] defer - Dbus object registration defer
         */
        FanSpeed(const std::string& instancePath,
                 const std::string& devPath,
                 const std::string& id,
                 sdbusplus::bus::bus& bus,
                 const char* objPath,
                 bool defer) : FanSpeedObject(bus, objPath, defer),
                    id(id),
                    ioAccess(instancePath),
                    devPath(devPath)
        {
            // Nothing to do here
        }

        /**
         * @brief Set the value of target
         *
         * @return Value of target
         */
        uint64_t target(uint64_t value) override;

        /**
         * @brief Writes the pwm_enable sysfs entry.
         */
        void enable();

    private:
        /** @brief hwmon type */
        static constexpr auto type = "fan";
        /** @brief hwmon id */
        std::string id;
        /** @brief Hwmon sysfs access. */
        sysfs::hwmonio::HwmonIO ioAccess;
        /** @brief Physical device path. */
        std::string devPath;

};

} // namespace hwmon
