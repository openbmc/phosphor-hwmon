#pragma once

#include <string>

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
         *  @param[in] path - hwmon sysfs instance to manage
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
            const std::string& path,
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

        /** @brief Shutdown requested. */
        bool _shutdown;
        /** @brief Path to hwmon sysfs instance. */
        std::string _path;
        /** @brief DBus busname prefix. */
        const char* _prefix;
        /** @brief DBus sensors namespace root. */
        const char* _root;
};
