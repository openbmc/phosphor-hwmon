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
#include <cstdlib>
#include <experimental/filesystem>
#include <fstream>
#include <memory>
#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Control/Device/error.hpp>
#include <xyz/openbmc_project/Sensor/Device/error.hpp>
#include "sysfs.hpp"

using namespace phosphor::logging;
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

/**
 * @brief Return the path to use for a call out.
 *
 * If the path does not contain iio-hwmon, assume passed in path is the call
 * out path.
 *
 * @param[in] ofPath - Open firmware path to search for matching phandle value
 *
 * @return Path to use for call out
 */
std::string findCalloutPath(const std::string& ofPath)
{
    static constexpr auto iioHwmonStr = "iio-hwmon";

    if (ofPath.find(iioHwmonStr) != std::string::npos)
    {
        auto matchpath = findPhandleMatch(ofPath, ofRoot);
        auto n = matchpath.rfind('/');
        if (n != std::string::npos)
        {
            return matchpath.substr(0, n);
        }
    }

    return ofPath;
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
        if (fs::canonical(path) != fullOfPath)
        {
            // Try to find HWMON instance via phandle values.
            // Used for IIO device drivers.
            auto ofpath = fullOfPath.string();
            auto matchpath = findPhandleMatch(path, ofpath);
            if (!std::string(matchpath).empty())
            {
                return hwmonInst.path();
            }
            else
            {
                continue;
            }
        }

        return hwmonInst.path();
    }

    return std::string();
}

int readSysfsWithCallout(const std::string& root,
                         const std::string& instance,
                         const std::string& type,
                         const std::string& id,
                         const std::string& sensor,
                         bool throwDeviceBusy)
{
    namespace fs = std::experimental::filesystem;

    int value = 0;
    std::ifstream ifs;
    fs::path instancePath{root};
    instancePath /= instance;
    std::string fullPath = make_sysfs_path(instancePath,
                                           type, id, sensor);

    ifs.exceptions(std::ifstream::failbit
                   | std::ifstream::badbit
                   | std::ifstream::eofbit);
    try
    {
        ifs.open(fullPath);
        ifs >> value;
    }
    catch (const std::exception& e)
    {
        // Too many GCC bugs (53984, 66145) to do
        // this the right way...

        // errno should still reflect the error from the failing open
        // or read system calls that got us here.
        auto rc = errno;

        if ((rc == EAGAIN) && throwDeviceBusy)
        {
            throw DeviceBusyException(fullPath);
        }

        // If the directory disappeared then this application should gracefully
        // exit.  There are race conditions between the unloading of a hwmon
        // driver and the stopping of this service by systemd.  To prevent
        // this application from falsely failing in these scenarios, it will
        // simply exit if the directory or file can not be found.  It is up
        // to the user(s) of this provided hwmon object to log the appropriate
        // errors if the object disappears when it should not.
        if (rc == ENOENT)
        {
            exit(0);
        }
        instancePath /= "device";
        auto callOutPath = findCalloutPath(fs::canonical(instancePath));
        using namespace sdbusplus::xyz::openbmc_project::Sensor::Device::Error;

        // this throws a ReadFailure.
        elog<ReadFailure>(
            xyz::openbmc_project::Sensor::Device::
                ReadFailure::CALLOUT_ERRNO(rc),
            xyz::openbmc_project::Sensor::Device::
                ReadFailure::CALLOUT_DEVICE_PATH(
                    fs::canonical(callOutPath).c_str()));
    }

    return value;
}

uint64_t writeSysfsWithCallout(const uint64_t& value,
                               const std::string& root,
                               const std::string& instance,
                               const std::string& type,
                               const std::string& id,
                               const std::string& sensor)
{
    namespace fs = std::experimental::filesystem;

    std::string valueStr = std::to_string(value);
    std::ofstream ofs;
    fs::path instancePath{root};
    instancePath /= instance;
    std::string fullPath = make_sysfs_path(instancePath,
                                           type, id, sensor);

    ofs.exceptions(std::ofstream::failbit
                   | std::ofstream::badbit
                   | std::ofstream::eofbit);
    try
    {
        ofs.open(fullPath);
        ofs << valueStr;
    }
    catch (const std::exception& e)
    {
        // errno should still reflect the error from the failing open
        // or write system calls that got us here.
        auto rc = errno;
        instancePath /= "device";
        auto callOutPath = findCalloutPath(fs::canonical(instancePath));
        using namespace sdbusplus::xyz::openbmc_project::Control::Device::Error;
        report<WriteFailure>(
            xyz::openbmc_project::Control::Device::
                WriteFailure::CALLOUT_ERRNO(rc),
            xyz::openbmc_project::Control::Device::
                WriteFailure::CALLOUT_DEVICE_PATH(
                    fs::canonical(callOutPath).c_str()));

        exit(EXIT_FAILURE);
    }

    return value;
}

}
// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
