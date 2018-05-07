#pragma once

#include "types.hpp"
#include "sensorset.hpp"
#include "hwmonio.hpp"

namespace sensor
{

class Sensor
{
    public:
        Sensor() = delete;
        Sensor(const Sensor&) = delete;
        Sensor(Sensor&&) = delete;
        Sensor& operator=(const Sensor&) = delete;
        Sensor& operator=(Sensor&&) = delete;
        ~Sensor() = default;

        explicit Sensor(const SensorSet::key_type& sensor,
                        const hwmonio::HwmonIO& ioAccess,
                        const std::string& devPath);

        /**
         * @brief Add status interface and functional property for sensor
         * @details When a sensor has an associated fault file, the OperationalStatus
         * interface is added along with setting the Functional property to the
         * corresponding value found in the fault file.
         *
         * @param[in] info - Sensor object information
         *
         * @return - Shared pointer to the status object
         */
        std::shared_ptr<StatusObject> addStatus(
                ObjectInfo& info);

    private:
        SensorSet::key_type _sensor;

        hwmonio::HwmonIO _ioAccess;

        const std::string _devPath;
};

} // namespace sensor
