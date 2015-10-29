#include <cerrno>
#include <cstring>
#include <iostream>
#include "directory.H"

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
