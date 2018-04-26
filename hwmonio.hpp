#pragma once

#include <chrono>
#include <string>

namespace hwmonio {

static constexpr auto retries = 10;
static constexpr auto delay = std::chrono::milliseconds{100};

/** @class HwmonIO
 *  @brief Convenience wrappers for HWMON sysfs attribute IO.
 *
 *  Unburden the rest of the application from having to check
 *  ENOENT after every hwmon attribute io operation.  Hwmon
 *  device drivers can be unbound at any time; the program
 *  cannot always be terminated externally before we try to
 *  do an io.
 */
class HwmonIO
{
    public:
        HwmonIO() = delete;
        HwmonIO(const HwmonIO&) = default;
        HwmonIO(HwmonIO&&) = default;
        HwmonIO& operator=(const HwmonIO&) = default;
        HwmonIO& operator=(HwmonIO&&) = default;
        ~HwmonIO() = default;

        /** @brief Constructor
         *
         *  @param[in] path - hwmon instance root - eg:
         *      /sys/class/hwmon/hwmon<N>
         */
        explicit HwmonIO(const std::string& path);

        /** @brief Perform formatted hwmon sysfs read.
         *
         *  Propagates any exceptions other than ENOENT.
         *  ENOENT will result in a call to exit(0) in case
         *  the underlying hwmon driver is unbound and
         *  the program is inadvertently left running.
         *
         *  For possibly transient errors will retry up to
         *  the specified number of times.
         *
         *  @param[in] type - The hwmon type (ex. temp).
         *  @param[in] id - The hwmon id (ex. 1).
         *  @param[in] sensor - The hwmon sensor (ex. input).
         *  @param[in] retries - The number of times to retry.
         *  @param[in] delay - The time to sleep between retry attempts.
         *
         *  @return val - The read value.
         */
        int64_t read(
                const std::string& type,
                const std::string& id,
                const std::string& sensor,
                size_t retries,
                std::chrono::milliseconds delay) const;

        /** @brief Perform formatted hwmon sysfs write.
         *
         *  Propagates any exceptions other than ENOENT.
         *  ENOENT will result in a call to exit(0) in case
         *  the underlying hwmon driver is unbound and
         *  the program is inadvertently left running.
         *
         *  For possibly transient errors will retry up to
         *  the specified number of times.
         *
         *  @param[in] val - The value to be written.
         *  @param[in] type - The hwmon type (ex. fan).
         *  @param[in] id - The hwmon id (ex. 1).
         *  @param[in] retries - The number of times to retry.
         *  @param[in] delay - The time to sleep between retry attempts.
         */
        void write(
                uint32_t val,
                const std::string& type,
                const std::string& id,
                const std::string& sensor,
                size_t retries,
                std::chrono::milliseconds delay) const;


        /** @brief Hwmon instance path access.
         *
         *  @return path - The hwmon instance path.
         */
        std::string path() const;

    private:
        std::string p;
};
} // namespace hwmonio

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
