#ifndef __DIRECTORY_H
#define __DIRECTORY_H

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

#endif

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
