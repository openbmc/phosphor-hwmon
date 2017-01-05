#pragma once

#include <map>
#include "sensorset.hpp"

/** @class ObjectSet
 *  @brief Associative array of hwmon sensor instances.
 *
 *  Associate hwmon sensor instances with data required for
 *  the application to maintain the state of a dbus object.
 */
class ObjectSet
{
    public:
        using mapped_type = std::tuple<SensorSet::mapped_type>;
        using container_t = std::map<SensorSet::key_type, mapped_type>;

        ~ObjectSet() = default;
        ObjectSet() = delete;
        ObjectSet(const ObjectSet&) = delete;
        ObjectSet& operator=(const ObjectSet&) = delete;
        ObjectSet(ObjectSet&&) = default;
        ObjectSet& operator=(ObjectSet&&) = default;

    private:
        container_t container;
};

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
