#pragma once

#include <exception>
#include <fstream>
#include <string>

namespace sysfs {

/**
 * @class DeviceBusyException
 *
 * An internal exception which will be thrown when
 * readSysfsWithCallout() hits an EAGAIN.  Will never bubble
 * up to terminate the application, nor does it need to be
 * reported.
 */
class DeviceBusyException : public std::runtime_error
{
    public:

        DeviceBusyException(const std::string& path) :
            std::runtime_error(path + " busy")
        {
        }
};

inline std::string make_sysfs_path(const std::string& path,
                                   const std::string& type,
                                   const std::string& id,
                                   const std::string& entry)
{
    using namespace std::literals;

    return path + "/"s + type + id + "_"s + entry;
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

/** @brief Read an hwmon sysfs value.
 *
 *  Calls exit(3) with bad status on failure.
 *
 *  @param[in] root - The hwmon class root.
 *  @param[in] instance - The hwmon instance (ex. hwmon1).
 *  @param[in] type - The hwmon type (ex. temp).
 *  @param[in] id - The hwmon id (ex. 1).
 *  @param[in] sensor - The hwmon sensor (ex. input).
 *  @param[in] throwDeviceBusy - will throw a DeviceBusyException
 *             on an EAGAIN errno instead of an error log exception.
 *
 *  @returns - The read value.
 */
int readSysfsWithCallout(const std::string& root,
                         const std::string& instance,
                         const std::string& type,
                         const std::string& id,
                         const std::string& sensor,
                         bool throwDeviceBusy = true);

 /** @brief Write a hwmon sysfs value
  *
  *  Calls exit(3) with bad status on failure
  *
  *  @param[in] value - The value to be written
  *  @param[in] root - The hwmon class root.
  *  @param[in] instance - The hwmon instance (ex. hwmon1).
  *  @param[in] type - The hwmon type (ex. fan).
  *  @param[in] id - The hwmon id (ex. 1).
  *  @param[in] sensor - The hwmon sensor (ex. target).
  *
  *  @returns - The value written
  */
uint64_t writeSysfsWithCallout(const uint64_t& value,
                               const std::string& root,
                               const std::string& instance,
                               const std::string& type,
                               const std::string& id,
                               const std::string& sensor);

namespace hwmonio
{

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
         *  Propogates any exceptions other than ENOENT.
         *  ENOENT will result in a call to exit(0) in case
         *  the underlying hwmon driver is unbound and
         *  the program is inadvertently left running.
         *
         *  @param[in] type - The hwmon type (ex. temp).
         *  @param[in] id - The hwmon id (ex. 1).
         *  @param[in] sensor - The hwmon sensor (ex. input).
         *
         *  @return val - The read value.
         */
        uint32_t read(
                const std::string& type,
                const std::string& id,
                const std::string& sensor) const;

        /** @brief Perform formatted hwmon sysfs write.
         *
         *  Propogates any exceptions other than ENOENT.
         *  ENOENT will result in a call to exit(0) in case
         *  the underlying hwmon driver is unbound and
         *  the program is inadvertently left running.
         *
         *  @param[in] val - The value to be written.
         *  @param[in] type - The hwmon type (ex. fan).
         *  @param[in] id - The hwmon id (ex. 1).
         *  @param[in] sensor - The hwmon sensor (ex. target).
         */
        void write(
                uint32_t val,
                const std::string& type,
                const std::string& id,
                const std::string& sensor) const;

        /** @brief Hwmon instance path access.
         *
         *  @return path - The hwmon instance path.
         */
        std::string path() const;

    private:
        std::string p;
};
} // namespace hwmonio
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
