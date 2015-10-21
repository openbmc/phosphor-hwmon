#include <regex>
#include <iostream>
#include "sensorset.H"
#include "directory.H"

// TODO: Issue#2 - STL regex generates really bloated code.  Use POSIX regex
//       interfaces instead.
static const std::regex sensors_regex =
    std::regex("^(fan|in|temp)([0-9]+)_([a-z]*)", std::regex::extended);
static const auto sensor_regex_match_count = 4;

SensorSet::SensorSet(const std::string& path)
{
    Directory d(path);
    std::string file;

    while(d.next(file))
    {
        std::smatch match;
        std::regex_search(file, match, sensors_regex);

        if (match.size() != sensor_regex_match_count) continue;

        container[make_pair(match[1],match[2])].emplace(match[3]);
    }
}
