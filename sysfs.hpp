#pragma once

#include <fstream>
#include <string>

inline std::string make_sysfs_path(const std::string& path,
                                   const std::string& type,
                                   const std::string& id,
                                   const std::string& entry)
{
    using namespace std::literals;

    return path + "/"s + type + id + "_"s + entry;
}


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

/** @brief Read an hwmon sysfs value.
 *
 *  Calls exit(3) with bad status on failure.
 *
 *  @param[in] root - The hwmon class root.
 *  @param[in] instance - The hwmon instance (ex. hwmon1).
 *  @param[in] type - The hwmon type (ex. temp).
 *  @param[in] id - The hwmon id (ex. 1).
 *  @param[in] sensor - The hwmon sensor (ex. input).
 *
 *  @returns - The read value.
 */
int readSysfsWithCallout(const std::string& root,
                         const std::string& instance,
                         const std::string& type,
                         const std::string& id,
                         const std::string& sensor);

 /** @brief Write a hwmon sysfs value
  *
  *  Calls exit(3) with bad status on failure
  *
  *  @param[in] value - The value to be written
  *  @param[in] targetPath - The hwmon target instance
  *  @param[in] sysfsFullPath - The full sysfs path of the target instance
  *
  *  @returns - The value written
  */
uint64_t writeSysfsWithCallout(const uint64_t& value,
                               const std::string& targetPath,
                               const std::string& sysfsFullPath);

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
