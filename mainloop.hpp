#pragma once

#include <string>
#include <vector>
#include <experimental/any>
#include <sdbusplus/server.hpp>
#include "sensorset.hpp"
#include "sysfs.hpp"
#include "interface.hpp"

static constexpr auto default_interval = 1000000;

using Object = std::map<InterfaceType, std::experimental::any>;
using ObjectInfo = std::tuple<sdbusplus::bus::bus*, std::string, Object>;

/** @class MainLoop
 *  @brief hwmon-readd main application loop.
 */
class MainLoop
{
    public:
        MainLoop() = delete;
        MainLoop(const MainLoop&) = delete;
        MainLoop& operator=(const MainLoop&) = delete;
        MainLoop(MainLoop&&) = default;
        MainLoop& operator=(MainLoop&&) = default;
        ~MainLoop() = default;

        /** @brief Constructor
         *
         *  @param[in] bus - sdbusplus bus client connection.
         *  @param[in] path - hwmon sysfs instance to manage
         *  @param[in] devPath - physical device sysfs path.
         *  @param[in] prefix - DBus busname prefix.
         *  @param[in] root - DBus sensors namespace root.
         *
         *  Any DBus objects are created relative to the DBus
         *  sensors namespace root.
         *
         *  At startup, the application will own a busname with
         *  the format <prefix>.hwmon<n>.
         */
        MainLoop(
            sdbusplus::bus::bus&& bus,
            const std::string& path,
            const std::string& devPath,
            const char* prefix,
            const char* root);

        /** @brief Start polling loop and process dbus traffic. */
        void run();

        /** @brief Stop loop from another thread.
         *
         *  Typically only used by testcases.
         */
        void shutdown() noexcept;

    private:
        using mapped_type = std::tuple<SensorSet::mapped_type, std::string, ObjectInfo>;
        using SensorState = std::map<SensorSet::key_type, mapped_type>;

        /** @brief sdbusplus bus client connection. */
        sdbusplus::bus::bus _bus;
        /** @brief sdbusplus freedesktop.ObjectManager storage. */
        sdbusplus::server::manager::manager _manager;
        /** @brief Shutdown requested. */
        volatile bool _shutdown;
        /** @brief hwmon sysfs class path. */
        std::string _hwmonRoot;
        /** @brief hwmon sysfs instance. */
        std::string _instance;
        /** @brief physical device sysfs path. */
        std::string _devPath;
        /** @brief DBus busname prefix. */
        const char* _prefix;
        /** @brief DBus sensors namespace root. */
        const char* _root;
        /** @brief hwmon instance is for an OCC. */
        bool _isOCC = false;
        /** @brief DBus object state. */
        SensorState state;
        /** @brief Sleep interval in microseconds. */
        uint64_t _interval = default_interval;
        /** @brief Hwmon sysfs access. */
        sysfs::hwmonio::HwmonIO ioAccess;
};
