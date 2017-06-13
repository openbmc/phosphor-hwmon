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

using namespace phosphor::logging;

std::string findHwmon(const std::string& ofNode)
{
    namespace fs = std::experimental::filesystem;
    static constexpr auto hwmonRoot = "/sys/class/hwmon";
    static constexpr auto ofRoot = "/sys/firmware/devicetree/base";

    fs::path fullOfPath{ofRoot};
    fullOfPath /= ofNode;

    for (const auto& hwmonInst : fs::directory_iterator(hwmonRoot))
    {
        auto path = hwmonInst.path();
        path /= "of_node";

        if (fs::canonical(path) != fullOfPath)
        {
            continue;
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

        // If the directory disappeared then this application should gracefully
        // exit.  There are race conditions between the unloading of a hwmon
        // driver and the stopping of this service by systemd.  To prevent
        // this application from falsely failing in these scenarios, it will
        // simply exit if the directory or file can not be found.  It is up
        // to the user(s) of this provided hwmon object to log the appropriate
        // errors if the object disappears when it should not.
        if(errno == ENOENT)
        {
            exit(0);
        }
        instancePath /= "device";
        using namespace sdbusplus::xyz::openbmc_project::Sensor::Device::Error;
        report<ReadFailure>(
            xyz::openbmc_project::Sensor::Device::
                ReadFailure::CALLOUT_ERRNO(rc),
            xyz::openbmc_project::Sensor::Device::
                ReadFailure::CALLOUT_DEVICE_PATH(
                    fs::canonical(instancePath).c_str()));

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
                    fs::canonical(instancePath).c_str()));

        exit(EXIT_FAILURE);
    }

    return value;
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
