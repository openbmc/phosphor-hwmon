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
#include "config.h"

#include "hwmonio.hpp"
#include "mainloop.hpp"
#include "sysfs.hpp"

#include <CLI/CLI.hpp>

#include <iostream>
#include <memory>

static void exit_with_error(const std::string& help, const char* err)
{
    std::cerr << "ERROR: " << err << std::endl << help << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
    // Read arguments.
    std::string syspath = "";
    std::string devpath = "";
    std::string sensor_id = "";

    CLI::App app{"OpenBMC Hwmon Daemon"};
    app.add_option("-p,--path", syspath, "sysfs location to monitor");
    app.add_option("-o,--dev-path", devpath, "device path to monitor");
    app.add_option("-i,--sensor-id", sensor_id, "dbus sensor instance id");

    CLI11_PARSE(app, argc, argv);

    std::string path;
    std::string param;

    // This path may either be a device path (starts with /devices),
    // a bus device identifier (when USE_BUS_DEVICE is enabled),
    // or an open firmware device tree path.
    if (!devpath.empty())
    {
        param = devpath;

        if constexpr (USE_BUS_DEVICE)
        {
            auto fullDevPath = sysfs::findDevPathFromBusDevice(devpath);
            if (!fullDevPath.empty())
            {
                path = sysfs::findHwmonFromDevPath(fullDevPath);
                if (path.empty())
                {
                    exit_with_error(
                        app.help("", CLI::AppFormatMode::All),
                        "Unable to find hwmon device from bus device path.");
                }
            }
        }
        else
        {
            // When disabled, use the original logic based on path format
            if (devpath.substr(0, 8) == "/devices")
            {
                path = sysfs::findHwmonFromDevPath(devpath);
                if (path.empty())
                {
                    exit_with_error(
                        app.help("", CLI::AppFormatMode::All),
                        "Unable to find hwmon device from device path.");
                }
            }
            else
            {
                path = sysfs::findHwmonFromOFPath(devpath);
                if (path.empty())
                {
                    exit_with_error(
                        app.help("", CLI::AppFormatMode::All),
                        "Unable to find hwmon device from OF path.");
                }
            }
        }
    }
    else if (!syspath.empty())
    {
        path = syspath;
        param = syspath;
    }

    if (path.empty())
    {
        exit_with_error(app.help("", CLI::AppFormatMode::All),
                        "Path not specified or invalid.");
    }

    // Determine the physical device sysfs path.
    auto calloutPath = sysfs::findCalloutPath(path);
    if (calloutPath.empty())
    {
        exit_with_error(app.help("", CLI::AppFormatMode::All),
                        "Unable to determine callout path.");
    }

    hwmonio::HwmonIO io(path);
    MainLoop loop(sdbusplus::bus::new_default(), param, path, calloutPath,
                  BUSNAME_PREFIX, SENSOR_ROOT, sensor_id, &io);
    loop.run();

    return 0;
}
