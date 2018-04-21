#pragma once

#include <map>
#include <set>
#include <string>

class SensorSet
{
  public:
    typedef std::map<std::pair<std::string, std::string>, std::set<std::string>>
        container_t;
    using mapped_type = container_t::mapped_type;
    using key_type = container_t::key_type;

    explicit SensorSet(const std::string& path);
    ~SensorSet() = default;
    SensorSet() = delete;
    SensorSet(const SensorSet&) = delete;
    SensorSet& operator=(const SensorSet&) = delete;
    SensorSet(SensorSet&&) = default;
    SensorSet& operator=(SensorSet&&) = default;

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

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
