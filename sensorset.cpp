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
#include <regex>
#include <iostream>
#include "sensorset.hpp"
#include "directory.hpp"
#include "hwmon.hpp"

// TODO: Issue#2 - STL regex generates really bloated code.  Use POSIX regex
//       interfaces instead.
static const std::regex sensors_regex =
    std::regex("^(fan|in|temp)([0-9]+)_([a-z]*)", std::regex::extended);
static const auto sensor_regex_match_count = 4;

SensorSet::SensorSet(const std::string& path)
{
    Directory d(path);
    std::string file;

    while (d.next(file))
    {
        std::smatch match;
        std::regex_search(file, match, sensors_regex);

        if (match.size() != sensor_regex_match_count)
        {
            continue;
        }

        if (match[3] == hwmon::entry::label)
        {
            continue;
        }

        container[make_pair(match[1], match[2])].emplace(match[3]);
    }
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
