#pragma once

#include <string>
#include <dirent.h>

class Directory
{
    public:
        explicit Directory(const std::string& path);
        ~Directory();

        bool next(std::string& name);

    private:
        dirent* entry;
        DIR* dirp;
};

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
