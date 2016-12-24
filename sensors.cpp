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
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <memory>
#include "sensors.hpp"

namespace libsensors
{

std::string Sensor::label() const
{
    char* l = sensors_get_label(chip.chip, feature);
    if (!l)
    {
        throw FeatureException(feature->name);
    }

    std::unique_ptr<char, details::MallocFree<char>> cleanup(l);
    return std::string(l);
}

std::string Sensor::type() const
{
    static constexpr auto featureMap =
    {
        std::make_pair("in", SENSORS_FEATURE_IN),
        std::make_pair("fan", SENSORS_FEATURE_FAN),
        std::make_pair("temp", SENSORS_FEATURE_TEMP),
        std::make_pair("power", SENSORS_FEATURE_POWER),
        std::make_pair("energy", SENSORS_FEATURE_ENERGY),
        std::make_pair("curr", SENSORS_FEATURE_CURR),
        std::make_pair("humidity", SENSORS_FEATURE_HUMIDITY),
        std::make_pair("cpu", SENSORS_FEATURE_VID),
        std::make_pair("intrusion", SENSORS_FEATURE_INTRUSION),
    };

    auto ft = feature->type;
    auto it = std::find_if(
                  featureMap.begin(),
                  featureMap.end(),
                  [ft](const auto & i)
    {
        return i.second == ft;
    });

    if (it == featureMap.end())
    {
        throw FeatureException(feature->name);
    }

    return it->first;
}

std::string Chip::path() const
{
    return chip->path;
}

std::vector<Sensor> Chip::sensors() const
{
    auto iter = 0;
    details::Feature feature = nullptr;
    std::vector<Sensor> sensors;

    while ((feature = sensors_get_features(chip, &iter)))
    {
        sensors.push_back(decltype(sensors)::value_type(feature, Chip(chip)));
    }

    return sensors;
}

std::vector<Chip> LibSensors::chips() const
{
    auto iter = 0;
    details::ChipName chip = nullptr;
    std::vector<Chip> chips;

    while ((chip = sensors_get_detected_chips(nullptr, &iter)))
    {
        chips.push_back(decltype(chips)::value_type(chip));
    }

    return chips;
}

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
