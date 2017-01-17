#pragma once

#include <fstream>
#include <string>

template <typename T>
void read_sysfs(const std::string& path, T& val)
{
    std::ifstream s(path);
    s >> val;
}

template <typename T>
void write_sysfs(const std::string& path, const T& val)
{
    std::ofstream s(path);
    s << val;
}

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

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
