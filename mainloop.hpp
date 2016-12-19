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
         */
        explicit MainLoop(const std::string& path);

        /** @brief Start polling loop and process dbus traffic. */
        void run();

        /** @brief Stop loop from another thread.
         *
         *  Typically only used by testcases.
         */
        void shutdown() noexcept;

    private:

        /** @brief Shutdown requested. */
        volatile bool _shutdown;
        /** @brief Path to hwmon sysfs instance. */
        std::string _path;
};
