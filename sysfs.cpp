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
#include <memory>
#include "sysfs.hpp"
#include "util.hpp"
#include "directory.hpp"

std::string findHwmon(const std::string& ofNode)
{
    static constexpr auto hwmonRoot = "/sys/class/hwmon";

    std::string fullOfPath{"/sys/firmware/devicetree/base"};
    fullOfPath.append(ofNode);

    std::string hwmonInst;
    Directory d(hwmonRoot);

    while (d.next(hwmonInst))
    {
        std::string hwmonPath{hwmonRoot};
        hwmonPath.append("/");
        hwmonPath.append(hwmonInst);
        std::string path{hwmonPath};
        path.append("/");
        path.append("of_node");

        auto real = std::unique_ptr<char, Free<char>>(realpath(path.c_str(), nullptr));
        if (!real || real.get() != fullOfPath)
        {
            continue;
        }

        return hwmonPath;
    }

    return std::string();
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
