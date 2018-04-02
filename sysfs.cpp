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
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <experimental/filesystem>
#include <fstream>
#include <memory>
#include <phosphor-logging/log.hpp>
#include <thread>
#include "config.h"
#include "sysfs.hpp"

using namespace std::string_literals;
namespace fs = std::experimental::filesystem;

namespace sysfs {

static constexpr auto retryableErrors = {
    /*
     * Retry on bus or device errors or timeouts in case
     * they are transient.
     */
    EIO,
    ETIMEDOUT,

    /*
     * Retry CRC errors.
     */
    EBADMSG,

    /*
     * Some hwmon drivers do this when they aren't ready
     * instead of blocking.  Retry.
     */
    EAGAIN,
    /*
     * We'll see this when for example i2c devices are
     * unplugged but the driver is still bound.  Retry
     * rather than exit on the off chance the device is
     * plugged back in and the driver doesn't do a
     * remove/probe.  If a remove does occur, we'll
     * eventually get ENOENT.
     */
    ENXIO,

    /*
     * Some devices return this when they are busy doing
     * something else.  Even if being busy isn't the cause,
     * a retry still gives this app a shot at getting data
     * as opposed to failing out on the first try.
     */
    ENODATA,
};

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

std::string findHwmonFromOFPath(const std::string& ofNode)
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
        if (!matchpath.empty())
        {
            return hwmonInst.path();
        }
    }

    return emptyString;
}

std::string findHwmonFromDevPath(const std::string& devPath)
{
    fs::path p{"/sys"};
    p /= devPath;
    p /= "hwmon";

    try
    {
        //This path is also used as a filesystem path to an environment
        //file, and that has issues with ':'s in the path so they've
        //been converted to '--'s.  Convert them back now.
        size_t pos = 0;
        std::string path = p;
        while ((pos = path.find("--")) != std::string::npos)
        {
            path.replace(pos, 2, ":");
        }

        for (const auto& hwmonInst : fs::directory_iterator(path))
        {
            if ((hwmonInst.path().filename().string().find("hwmon") !=
                   std::string::npos))
            {
                return hwmonInst.path();
            }
        }
    }
    catch (const std::exception& e)
    {
        using namespace phosphor::logging;
        log<level::ERR>(
                "Unable to find hwmon directory from the dev path",
                entry("PATH=%s", devPath.c_str()));
    }
    return emptyString;
}

namespace hwmonio
{

HwmonIO::HwmonIO(const std::string& path) : p(path)
{

}

int64_t HwmonIO::read(
        const std::string& type,
        const std::string& id,
        const std::string& sensor,
        size_t retries,
        std::chrono::milliseconds delay,
        bool isOCC) const
{
    int64_t val;
    std::ifstream ifs;
    auto fullPath = sysfs::make_sysfs_path(
            p, type, id, sensor);

    ifs.exceptions(
            std::ifstream::failbit |
                std::ifstream::badbit |
                std::ifstream::eofbit);

    while (true)
    {
        try
        {
            errno = 0;
            if (!ifs.is_open())
                ifs.open(fullPath);
            ifs.clear();
            ifs.seekg(0);
            ifs >> val;
        }
        catch (const std::exception& e)
        {
            auto rc = errno;

            if (!rc)
            {
                throw;
            }

            if (rc == ENOENT || rc == ENODEV)
            {
                // If the directory or device disappeared then this application
                // should gracefully exit.  There are race conditions between the
                // unloading of a hwmon driver and the stopping of this service
                // by systemd.  To prevent this application from falsely failing
                // in these scenarios, it will simply exit if the directory or
                // file can not be found.  It is up to the user(s) of this
                // provided hwmon object to log the appropriate errors if the
                // object disappears when it should not.
                exit(0);
            }

            if (isOCC)
            {
                if (rc == EREMOTEIO)
                {
                    // For the OCCs, when an EREMOTEIO is return, set the
                    // value to 255*1000
                    // (0xFF = sensor is failed, 1000 = sensor factor)
                    val = 255000;
                    break;
                }
            }

            if (0 == std::count(
                        retryableErrors.begin(),
                        retryableErrors.end(),
                        rc) ||
                    !retries)
            {
                // Not a retryable error or out of retries.
#ifdef NEGATIVE_ERRNO_ON_FAIL
                return -rc;
#endif

                // Work around GCC bugs 53984 and 66145 for callers by
                // explicitly raising system_error here.
                throw std::system_error(rc, std::generic_category());
            }

            --retries;
            std::this_thread::sleep_for(delay);
            continue;
        }
        break;
    }

    return val;
}

void HwmonIO::write(
        uint32_t val,
        const std::string& type,
        const std::string& id,
        const std::string& sensor,
        size_t retries,
        std::chrono::milliseconds delay) const

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

    while (true)
    {
        try
        {
            errno = 0;
            if (!ofs.is_open())
                ofs.open(fullPath);
            ofs.clear();
            ofs.seekp(0);
            ofs << val;
            ofs.flush();
        }
        catch (const std::exception& e)
        {
            auto rc = errno;

            if (!rc)
            {
                throw;
            }

            if (rc == ENOENT)
            {
                exit(0);
            }

            if (0 == std::count(
                        retryableErrors.begin(),
                        retryableErrors.end(),
                        rc) ||
                    !retries)
            {
                // Not a retryable error or out of retries.

                // Work around GCC bugs 53984 and 66145 for callers by
                // explicitly raising system_error here.
                throw std::system_error(rc, std::generic_category());
            }

            --retries;
            std::this_thread::sleep_for(delay);
            continue;
        }
        break;
    }
}

std::string HwmonIO::path() const
{
    return p;
}

} // namespace hwmonio
}
// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
