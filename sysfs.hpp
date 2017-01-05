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

inline const std::string make_sysfs_path(const std::string& path,
        const std::string& type,
        const std::string& id,
        const std::string& entry)
{
    using namespace std::literals;

    return path + "/"s + type + id + "_"s + entry;
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
