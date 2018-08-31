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

#include "env.hpp"

#include "hwmon.hpp"

#include <cstdlib>
#include <fstream>

namespace env
{

std::string getEnv(const char* key)
{
    auto value = std::getenv(key);
    return (value) ? std::string(value) : std::string();
}

std::string getEnv(const char* prefix, const SensorSet::key_type& sensor)
{
    std::string key;

    key.assign(prefix);
    key.append(1, '_');
    key.append(sensor.first);
    key.append(sensor.second);

    return getEnv(key.c_str());
}

std::string getEnv(const char* prefix, const std::string& type,
                   const std::string& id)
{
    SensorSet::key_type sensor{type, id};
    return getEnv(prefix, sensor);
}

std::string getIndirectID(std::string path, const std::string& fileSuffix,
                          const SensorSet::key_type& sensor)
{
    std::string content;

    path.append(sensor.first);
    path.append(sensor.second);
    path.append(1, '_');
    path.append(fileSuffix);

    std::ifstream handle(path.c_str());
    if (!handle.fail())
    {
        content.assign((std::istreambuf_iterator<char>(handle)),
                       (std::istreambuf_iterator<char>()));

        if (!content.empty())
        {
            // remove the newline
            content.pop_back();
        }
    }

    return content;
}

} // namespace env

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
