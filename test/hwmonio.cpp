/**
 * Copyright © 2017 IBM Corporation
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
#include <string>
#include "../sysfs.hpp"

int main(int argc, char* argv[])
{
    using namespace std::string_literals;

    if (argc < 6)
    {
        std::cerr << "Usage: " << argv[0]
            << " [read|write] PATH TYPE N ATTR [VALUE]" << std::endl;
        return 1;
    }

    sysfs::hwmonio::HwmonIO io(argv[2]);

    static constexpr auto retries = 10;
    static constexpr std::chrono::milliseconds delay{100};

    if ("read"s == argv[1])
    {
        std::cout << io.read(argv[3], argv[4], argv[5], retries, delay) <<
            std::endl;
    }
    else
    {
        io.write(
                strtol(argv[6], nullptr, 0),
                argv[3], argv[4], argv[5], retries, delay);
    }

    return 0;
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
