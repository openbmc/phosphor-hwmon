#pragma once

#include "sensorset.hpp"

#include <optional>
#include <string>
#include <vector>

/** @class AverageHandling
 *  @brief Handle avergae value when AVERAGE_* is set in env
 */
class Average
{
  public:
    /** @brief The key type of average_set */
    using averageKey = SensorSet::key_type;

    /** @brief <average, average_interval>
     *  average is the previous value of power*_average.
     *  average_interval is the previous value of power*_average_interval.
     */
    using averageValue = std::pair<int64_t, int64_t>;

    /** @brief Store sensors' <averageKey, averageValue> map */
    using averageMap = std::map<averageKey, averageValue>;

    /** @brief Get averageValue in averageMap based on averageKey.
     *  This function will be called only when the env AVERAGE_xxx is set to
     *  true.
     *
     *  @param[in] sensorKey - Sensor details
     *
     *  @return - Optional
     *      return {}, if sensorKey can not be found in averageMap
     *      return averageValue, if sensorKey can be found in averageMap
     */
    std::optional<averageValue>
        getAverageValue(const averageKey& sensorKey) const;

    /** @brief Set average value in averageMap based on sensor key.
     *  This function will be called only when the env AVERAGE_xxx is set to
     *  true.
     *
     *  @param[in] sensorKey - Sensor details
     *  @param[in] sensorValue - The related average values of this sensor
     */
    void setAverageValue(const averageKey& sensorKey,
                         const averageValue& sensorValue);

    /** @brief Calculate the average value.
     *  delta = curInterval - preInterval, and curInterval should be bigger than
     * preInterval.
     *
     *  @param[in] preAverage - The previous average value from *_average file
     *  @param[in] preInterval - The previous interval value from
     *  *_average_interval file
     *  @param[in] curAverage - The current average value from *_average file
     *  @param[in] delta - The delta between previous and current interval
     *
     *  @return value - New calculated average value
     */
    int64_t calAverage(int64_t preAverage, int64_t preInterval,
                       int64_t curAverage, int64_t delta);

  private:
    /** @brief Store the average sensor set */
    averageMap _averageMap;
};