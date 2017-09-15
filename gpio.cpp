/**
 * Copyright © 2017 Google
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
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "gpio.hpp"

namespace fs = std::experimental::filesystem;

namespace gpio {

std::string GpioManager::getGpioPath(int value)
{
    return _gpioDirBase + std::to_string(value);
}

void GpioManager::writeFile(const std::string &path, const std::string &value)
{
    std::ofstream valueAccess(path);
    if (valueAccess.good())
    {
        valueAccess << value << std::endl;
    }
}


// Given /sys/class/gpio/gpiochip*/base get the value from the file.
// if there is more than one match, just use the first for now.
// This code is very hack-ish.
// Examine of a specific base:
//   /sys/devices/platform/ahb/ahb:apb/1e780000.gpio/gpio/
// As noted in the larger bug for a gpio library it should really use a
// label instead of a path.  Then it can search for the path that
// corresponds to that label.
GpioManager::GpioManager(const std::string& base)
{
    int baseValue = 0;
    std::string fldr;

    // If they specify the base, use it, otherwise assume there's only one.
    if (!base.empty())
    {
        // Grab 0th folder.
        for (const auto& folder : fs::directory_iterator(base))
        {
            fldr = folder.path();
            break;
        }
    }
    else
    {
        for (const auto& folder : fs::directory_iterator(_gpioBase))
        {
            std::string fldr = folder.path();
            if (std::string::npos != fldr.find(_gpioChip))
            {
                // We found the gpiochip folder, so let's bring the chip base
                // yes it is in the name of the folder, but this feels more
                // straightforward.
                break;
            }
        }
    }

    std::ifstream baseFd(fldr + "/base");
    if (baseFd.good())
    {
        baseFd >> baseValue;
    }

    // In the error case we're setting the base to 0.
    // If we ignore that and export the relative value
    // it'll fail.  And later the associated files won't
    // exist.  So, the code will just always verify that
    // the files exist.
    _baseChipId = baseValue;
}

void GpioManager::setupGpio(int relGpio, const SensorSet::key_type& sensor)
{
    int absGpio = relGpio + _baseChipId;
    std::string path = getGpioPath(absGpio);
    _gpioAccess[sensor] = path;

    if (!fs::exists(path))
    {
        // Didn't already exist, let's create it.
        writeFile(_gpioExport, std::to_string(absGpio));
    }
}

void GpioManager::setGpio(const SensorSet::key_type& sensor,
                   const std::string& value,
                   bool pause)
{
    const auto& it = _gpioAccess.find(sensor);
    if (it != _gpioAccess.end())
    {
        std::string path = it->second;
        if (fs::exists(path))
        {
            writeFile(path + "/direction", value);

            if (pause)
            {
                std::this_thread::sleep_for(_pause);
            }
        }
    }
}

void GpioManager::unlockGpio(const SensorSet::key_type& sensor)
{
    std::string value = "high";
    return setGpio(sensor, value, true);
}

void GpioManager::lockGpio(const SensorSet::key_type& sensor)
{
    std::string value = "low";
    return setGpio(sensor, value, false);
}

}
