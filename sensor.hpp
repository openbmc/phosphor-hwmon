#pragma once

#include <cstring>
#include <unordered_set>

#include "types.hpp"
#include "sensorset.hpp"
#include "hwmonio.hpp"

namespace sensor
{

struct valueAdjust
{
    double gain = 1.0;
    int offset = 0;
    std::unordered_set<int> rmRCs;
};

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

        void addRemoveRCs(const std::string& rcList);

        inline const valueAdjust& getAdjusts()
        {
            return sensorAdjusts;
        }

        int64_t adjustValue(int64_t value);

        std::shared_ptr<ValueObject> addValue(
                const RetryIO& retryIO,
                ObjectInfo& info);

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

        valueAdjust sensorAdjusts;
};

} // namespace sensor
