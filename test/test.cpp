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
#include "../mainloop.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include <thread>

auto server_thread(void* data)
{
    auto mgr = static_cast<MainLoop*>(data);

    mgr->run();

    return static_cast<void*>(nullptr);
}

void runTests(MainLoop& loop)
{
    loop.shutdown();
    std::cout << "Success!\n";
}

int main()
{
    char tmpl[] = "/tmp/hwmon-test.XXXXXX";
    std::string dir = mkdtemp(tmpl);
    std::string entry = dir + "/temp1_input";
    std::ofstream f{entry};
    f << "1234";

    auto loop = MainLoop(dir);
    auto t = std::thread(server_thread, &loop);

    runTests(loop);

    // Wait for server thread to exit.
    t.join();
    unlink(entry.c_str());
    rmdir(dir.c_str());

    return 0;
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
