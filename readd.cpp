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
#include <iostream>
#include <memory>
#include "argument.hpp"
#include "mainloop.hpp"
#include "config.h"
#include "sysfs.hpp"

static void exit_with_error(const char* err, char** argv)
{
    ArgumentParser::usage(argv);
    std::cerr << std::endl;
    std::cerr << "ERROR: " << err << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
    // Read arguments.
    auto options = std::make_unique<ArgumentParser>(argc, argv);

    // Parse out path argument.
    auto path = (*options)["of-name"];
    if (path != ArgumentParser::empty_string)
    {
        path = sysfs::findHwmonFromOFPath(path);
    }

    if (path == ArgumentParser::empty_string)
    {
        path = (*options)["path"];
    }

    if (path == ArgumentParser::empty_string)
    {
        exit_with_error("Path not specified or invalid.", argv);
    }

    // Finished getting options out, so cleanup the parser.
    options.reset();

    // Determine the physical device sysfs path.
    auto calloutPath = sysfs::findCalloutPath(path);
    if (calloutPath.empty())
    {
        exit_with_error("Unable to determine callout path.", argv);
    }

    MainLoop loop(
        sdbusplus::bus::new_default(),
        path,
        calloutPath,
        BUSNAME_PREFIX,
        SENSOR_ROOT);
    loop.run();

    return 0;
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
