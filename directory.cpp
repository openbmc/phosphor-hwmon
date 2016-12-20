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
#include <cerrno>
#include <cstring>
#include <iostream>
#include "directory.hpp"

Directory::Directory(const std::string& path) : entry(nullptr)
{
    dirp = opendir(path.c_str());
    if (NULL == dirp)
    {
        auto e = errno;
        std::cerr << "Error opening directory " << path.c_str()
                  << " : " << strerror(e) << std::endl;
    }
}

Directory::~Directory()
{
    if (dirp)
    {
        closedir(dirp);
    }
}

bool Directory::next(std::string& name)
{
    if (!dirp) return false;

    dirent entry;
    dirent* result;

    auto rc = readdir_r(dirp, &entry, &result);

    if ((rc) || (NULL == result)) return false;

    name = entry.d_name;
    return true;
}
