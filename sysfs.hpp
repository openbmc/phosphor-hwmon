#pragma once

#include <chrono>
#include <exception>
#include <fstream>
#include <string>

namespace sysfs {

using namespace std::literals;
inline std::string make_sysfs_path(const std::string& path,
                                   const std::string& type,
                                   const std::string& id,
                                   const std::string& entry)
{
    return path + "/"s + type + id + "_"s + entry;
}

inline std::string make_sysfs_path(const std::string& path,
                                   const std::string& file)
{
    return path + "/"s + file;
}

/** @brief Return the path to the phandle file matching value in io-channels.
 *
 *  This function will take two passed in paths.
 *  One path is used to find the io-channels file.
 *  The other path is used to find the phandle file.
 *  The 4 byte phandle value is read from the phandle file(s).
 *  The 4 byte phandle value and 4 byte index value is read from io-channels.
 *  When a match is found, the path to the matching phandle file is returned.
 *
 *  @param[in] iochanneldir - Path to file for getting phandle from io-channels
 *  @param[in] phandledir - Path to use for reading from phandle file
 *
 *  @return Path to phandle file with value matching that in io-channels
 */
std::string findPhandleMatch(
        const std::string& iochanneldir,
        const std::string& phandledir);

/** @brief Find hwmon instances
 *
 *  Look for a matching hwmon instance given an
 *  open firmware device path.
 *
 *  @param[in] ofNode- The open firmware device path.
 *
 *  @returns[in] - The hwmon instance path or an empty
 *                 string if no match is found.
 */
std::string findHwmon(const std::string& ofNode);

/** @brief Return the path to use for a call out.
 *
 *  Return an empty string if a callout path cannot be
 *  found.
 *
 *  @param[in] instancePath - /sys/class/hwmon/hwmon<N> path.
 *
 *  @return Path to use for call out
 */
std::string findCalloutPath(const std::string& instancePath);

namespace hwmonio
{
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
         *  @param[in] file - The sysfs file to read.
         *  @param[in] retries - The number of times to retry.
         *  @param[in] delay - The time to sleep between retry attempts.
         *
         *  @return val - The read value.
         */
        int64_t read(
                const std::string& file,
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
         *  @param[in] file - The sysfs file to write.
         *  @param[in] retries - The number of times to retry.
         *  @param[in] delay - The time to sleep between retry attempts.
         */
        void write(
                uint32_t val,
                const std::string& file,
                size_t retries,
                std::chrono::milliseconds delay) const;

        /** @brief Hwmon instance path access.
         *
         *  @return path - The hwmon instance path.
         */
        std::string path() const;

    private:
        std::string p;

        /** @brief Implementation of read()
         *
         *  @parm[in] fullPath - The full path of the file to read
         *  @param[in] retries - The number of times to retry.
         *  @param[in] delay - The time to sleep between retry attempts.
         *
         *  @return val - The read value.
         */
        int64_t readInternal(const std::string& fullPath,
                             size_t retries,
                             std::chrono::milliseconds delay) const;

        /** @brief Implementation of read()
         *  @param[in] val - The value to be written.
         *  @param[in] file - The full path of the file to write.
         *  @param[in] retries - The number of times to retry.
         *  @param[in] delay - The time to sleep between retry attempts.
         */
        void writeInternal(uint32_t val,
                           const std::string& file,
                           size_t retries,
                           std::chrono::milliseconds delay) const;
};
} // namespace hwmonio
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
