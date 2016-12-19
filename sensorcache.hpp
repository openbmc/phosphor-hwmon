#ifndef __SENSORCACHE_H
#define __SENSORCACHE_H

#include <map>

class SensorCache
{
    public:
        typedef std::map<std::pair<std::string, std::string>,
                int> container_t;

        bool update(const container_t::key_type& k,
                    const container_t::mapped_type& v)
        {
            auto& i = container[k];
            if (v == i)
            {
                return false;
            }
            else
            {
                i = v;
                return true;
            }
        }
    private:
        container_t container;
};

#endif
