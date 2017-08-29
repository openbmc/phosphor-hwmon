/**
 * Copyright © 2016 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cerrno>
#include <cstdlib>
#include <experimental/filesystem>
#include <fstream>
#include <memory>
#include "sysfs.hpp"

using namespace std::string_literals;
namespace fs = std::experimental::filesystem;

namespace sysfs {

static const auto emptyString = ""s;
static constexpr auto ofRoot = "/sys/firmware/devicetree/base";

std::string findPhandleMatch(
        const std::string& iochanneldir,
        const std::string& phandledir)
{
    // TODO: At the moment this method only supports device trees
    // with iio-hwmon nodes with a single sensor.  Typically
    // device trees are defined with all the iio sensors in a
    // single iio-hwmon node so it would be nice to add support
    // for lists of phandles (with variable sized entries) via
    // libfdt or something like that, so that users are not
    // forced into implementing unusual looking device trees
    // with multiple iio-hwmon nodes - one for each sensor.

    fs::path ioChannelsPath{iochanneldir};
    ioChannelsPath /= "io-channels";

    if (!fs::exists(ioChannelsPath))
    {
        return emptyString;
    }

    uint32_t ioChannelsValue;
    std::ifstream ioChannelsFile(ioChannelsPath);

    ioChannelsFile.read(
            reinterpret_cast<char*>(&ioChannelsValue),
            sizeof(ioChannelsValue));

    for (const auto& ofInst : fs::recursive_directory_iterator(phandledir))
    {
        auto path = ofInst.path();
        if ("phandle" != path.filename())
        {
            continue;
        }
        std::ifstream pHandleFile(path);
        uint32_t pHandleValue;

        pHandleFile.read(
                reinterpret_cast<char*>(&pHandleValue),
                sizeof(pHandleValue));

        if (ioChannelsValue == pHandleValue)
        {
            return path;
        }
    }

    return emptyString;
}

std::string findCalloutPath(const std::string& instancePath)
{
    // Follow the hwmon instance (/sys/class/hwmon/hwmon<N>)
    // /sys/devices symlink.
    fs::path devPath{instancePath};
    devPath /= "device";

    try
    {
        devPath = fs::canonical(devPath);
    }
    catch (const std::system_error& e)
    {
        return emptyString;
    }

    // See if the device is backed by the iio-hwmon driver.
    fs::path p{devPath};
    p /= "driver";
    p = fs::canonical(p);

    if (p.filename() != "iio_hwmon")
    {
        // Not backed by iio-hwmon.  The device pointed to
        // is the callout device.
        return devPath;
    }

    // Find the DT path to the iio-hwmon platform device.
    fs::path ofDevPath{devPath};
    ofDevPath /= "of_node";

    try
    {
        ofDevPath = fs::canonical(ofDevPath);
    }
    catch (const std::system_error& e)
    {
        return emptyString;
    }

    // Search /sys/bus/iio/devices for the phandle in io-channels.
    // If a match is found, use the corresponding /sys/devices
    // iio device as the callout device.
    static constexpr auto iioDevices = "/sys/bus/iio/devices";
    for (const auto& iioDev: fs::recursive_directory_iterator(iioDevices))
    {
        p = iioDev.path();
        p /= "of_node";

        try
        {
            p = fs::canonical(p);
        }
        catch (const std::system_error& e)
        {
            continue;
        }

        auto match = findPhandleMatch(ofDevPath, p);
        auto n = match.rfind('/');
        if (n != std::string::npos)
        {
            // This is the iio device referred to by io-channels.
            // Remove iio:device<N>.
            try
            {
                return fs::canonical(iioDev).parent_path();
            }
            catch (const std::system_error& e)
            {
                return emptyString;
            }
        }
    }

    return emptyString;
}

std::string findHwmon(const std::string& ofNode)
{
    static constexpr auto hwmonRoot = "/sys/class/hwmon";

    fs::path fullOfPath{ofRoot};
    fullOfPath /= ofNode;

    for (const auto& hwmonInst : fs::directory_iterator(hwmonRoot))
    {
        auto path = hwmonInst.path();
        path /= "of_node";

        try
        {
            path = fs::canonical(path);
        }
        catch (const std::system_error& e)
        {
            // realpath may encounter ENOENT (Hwmon
            // instances have a nasty habit of
            // going away without warning).
            continue;
        }

        if (path == fullOfPath)
        {
            return hwmonInst.path();
        }

        // Try to find HWMON instance via phandle values.
        // Used for IIO device drivers.
        auto matchpath = findPhandleMatch(path, fullOfPath);
        if (!std::string(matchpath).empty())
        {
            return hwmonInst.path();
        }
    }

    return emptyString;
}

namespace hwmonio
{

HwmonIO::HwmonIO(const std::string& path) : p(path)
{

}

uint32_t HwmonIO::read(
        const std::string& type,
        const std::string& id,
        const std::string& sensor) const
{
    uint32_t val;
    std::ifstream ifs;
    auto fullPath = sysfs::make_sysfs_path(
            p, type, id, sensor);

    ifs.exceptions(
            std::ifstream::failbit |
                std::ifstream::badbit |
                std::ifstream::eofbit);
    try
    {
        ifs.open(fullPath);
        ifs >> val;
    }
    catch (const std::exception& e)
    {
        auto rc = errno;

        if (rc && rc == ENOENT)
        {
            // If the directory disappeared then this application should
            // gracefully exit.  There are race conditions between the
            // unloading of a hwmon driver and the stopping of this service
            // by systemd.  To prevent this application from falsely failing
            // in these scenarios, it will simply exit if the directory or
            // file can not be found.  It is up to the user(s) of this
            // provided hwmon object to log the appropriate errors if the
            // object disappears when it should not.
            exit(0);
        }

        if (rc)
        {
            // Work around GCC bugs 53984 and 66145 for callers by
            // explicitly raising system_error here.
            throw std::system_error(rc, std::generic_category());
        }

        throw;
    }
    return val;
}

void HwmonIO::write(
        uint32_t val,
        const std::string& type,
        const std::string& id,
        const std::string& sensor) const
{
    std::ofstream ofs;
    auto fullPath = sysfs::make_sysfs_path(
            p, type, id, sensor);

    ofs.exceptions(
            std::ofstream::failbit |
                std::ofstream::badbit |
                std::ofstream::eofbit);

    // See comments in the read method for an explanation of the odd exception
    // handling behavior here.

    try
    {
        ofs.open(fullPath);
        ofs << val;
    }
    catch (const std::exception& e)
    {
        auto rc = errno;

        if (rc && rc == ENOENT)
        {
            exit(0);
        }

        if (rc)
        {
            throw std::system_error(rc, std::generic_category());
        }

        throw;
    }
}

std::string HwmonIO::path() const
{
    return p;
}

} // namespace hwmonio
}
// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
