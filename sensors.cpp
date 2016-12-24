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
#include <sensors/sensors.h>
#include "sensors.hpp"

namespace libsensors
{

LibSensors::LibSensors()
{
    if (!useCount)
    {
        auto rc = sensors_init(nullptr);
        if (rc)
        {
            throw LibSensorsException(rc);
        }
    }
    ++useCount;
}

LibSensors::~LibSensors() noexcept
{
    --useCount;
    if (!useCount)
    {
        sensors_cleanup();
    }
}

LibSensors loadDefault()
{
    return LibSensors();
}

size_t LibSensors::useCount = 0;
} // namespace libsensors

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
