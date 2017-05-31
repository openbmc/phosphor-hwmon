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
#include <memory>
#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <xyz/openbmc_project/Control/Device/error.hpp>
#include <xyz/openbmc_project/Sensor/Device/error.hpp>
#include "sysfs.hpp"
#include "util.hpp"
#include <fstream>

using namespace phosphor::logging;
namespace fs = std::experimental::filesystem;

static constexpr auto ofRoot = "/sys/firmware/devicetree/base";

// This function will take two passed in paths.
// One path is used to find the io-channels file.
// The other path is sued to find the phandle file.
// The 4 byte phandle value is read from the phandle file(s).
// The 4 byte phandle value and 4 byte index value is read from io-channels.
// When a match is found, the path to the matching phandle file is returned.
std::string findPhandleMatch(const std::string& iochanneldir,
                             const std::string& phandledir)
{
    for (const auto& ofInst : fs::recursive_directory_iterator(phandledir))
    {
        auto path = ofInst.path();
        if ("phandle" == ofInst.path().filename())
        {
            auto ioChannelsPath = iochanneldir + "/io-channels";
            if (fs::exists(ioChannelsPath))
            {
                auto fullOfPathPhandle = ofInst.path();
                std::ifstream ioChannelsFile(ioChannelsPath,
                                             std::ios::in | std::ios::binary);
                std::ifstream pHandleFile(fullOfPathPhandle,
                                          std::ios::in | std::ios::binary);

                // Variable to read in the phandle value from phandle file.
                uint32_t pHandleValue;

                // io-channels file has a phandle value, and an index
                // Variable to read the phandle value into from io-channels file
                uint32_t phandle;
                // Variable to read the index value into from io-channels file
                uint32_t index;

                try
                {
                    pHandleFile.read(reinterpret_cast<char*>(&pHandleValue),
                                     sizeof(pHandleValue));

                    ioChannelsFile.read(reinterpret_cast<char*>(&phandle),
                                        sizeof(phandle));
                    ioChannelsFile.read(reinterpret_cast<char*>(&index),
                                        sizeof(index));
                }
                catch (const std::exception& e)
                {
                    log<level::ERR>(e.what());
                }

                ioChannelsFile.close();
                pHandleFile.close();

                if (phandle == pHandleValue)
                {
                    return ofInst.path();
                }
            }
        }
    }

    return std::string();
}

// openbmc/openbmc#1265
// Return the path to use for a call out.
// If the path does not contain iio-hwmon, assume passed in path is the
// call out path.
std::string make_sysfs_callout(const std::string& instancePath)
{
    static constexpr auto iioHwmonStr = "iio-hwmon";

    if (instancePath.find(iioHwmonStr) != std::string::npos)
    {
        auto matchpath = findPhandleMatch(instancePath, ofRoot);
        auto n = matchpath.rfind('/');
        if (n != std::string::npos)
        {
            return matchpath.substr(0, n);
        }
        else
        {
            return instancePath;
        }
    }
    else
    {
        return instancePath;
    }
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
                         const std::string& sensor)
{
    namespace fs = std::experimental::filesystem;

    int value = 0;
    std::ifstream ifs;
    fs::path instancePath{root};
    instancePath /= instance;
    std::string fullPath = make_sysfs_path(instancePath,
                                           type, id, sensor);

    std::string callOutPath = make_sysfs_callout(fs::canonical(instancePath));

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
        instancePath /= "device";
        using namespace sdbusplus::xyz::openbmc_project::Sensor::Device::Error;
        report<ReadFailure>(
            xyz::openbmc_project::Sensor::Device::
                ReadFailure::CALLOUT_ERRNO(rc),
            xyz::openbmc_project::Sensor::Device::
                ReadFailure::CALLOUT_DEVICE_PATH(
                    fs::canonical(callOutPath).c_str()));

        exit(EXIT_FAILURE);
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

    std::string callOutPath = make_sysfs_callout(fs::canonical(instancePath));

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

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
