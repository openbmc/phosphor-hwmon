#ifndef __SENSORSET_H
#define __SENSORSET_H

#include <map>
#include <set>
#include <string>

class SensorSet
{
    public:
        typedef std::map<std::pair<std::string, std::string>,
                std::set<std::string>> container_t;

        SensorSet(const std::string& path);

        container_t::const_iterator begin()
        {
            return const_cast<const container_t&>(container).begin();
        }

        container_t::const_iterator end()
        {
            return const_cast<const container_t&>(container).end();
        }

    private:
        container_t container;

};

#endif

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
