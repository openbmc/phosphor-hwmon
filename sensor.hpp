#pragma once

#include "hwmonio.hpp"
#include "sensorset.hpp"
#include "types.hpp"

#include <chrono>
#include <gpioplus/handle.hpp>
#include <unordered_set>

namespace sensor
{

struct valueAdjust
{
    double gain = 1.0;
    int offset = 0;
    std::unordered_set<int> rmRCs;
};

/** @class Sensor
 *  @brief Sensor object based on a SensorSet container's key type
 *  @details Sensor object to create and modify an associated device's sensor
 *  attributes based on the key type of each sensor in the set provided by the
 *  device.
 */
class Sensor
{
  public:
    Sensor() = delete;
    Sensor(const Sensor&) = delete;
    Sensor(Sensor&&) = default;
    Sensor& operator=(const Sensor&) = delete;
    Sensor& operator=(Sensor&&) = default;
    ~Sensor() = default;

    /**
     * @brief Constructs Sensor object
     *
     * @param[in] sensor - A pair of sensor indentifiers
     * @param[in] ioAccess - Hwmon sysfs access
     * @param[in] devPath - Device sysfs path
     */
    explicit Sensor(const SensorSet::key_type& sensor,
                    const hwmonio::HwmonIO& ioAccess,
                    const std::string& devPath);

    /**
     * @brief Adds any sensor removal return codes for the sensor
     * @details Add all return codes defined within a device's config file
     * for the entire device or for the specific sensor.
     *
     * @param[in] rcList - List of return codes found for the sensor
     */
    void addRemoveRCs(const std::string& rcList);

    /**
     * @brief Get the adjustments struct for the sensor
     *
     * @return - Sensor adjustment struct
     */
    inline const valueAdjust& getAdjusts()
    {
        return sensorAdjusts;
    }

    /**
     * @brief Adjusts a sensor value
     * @details Adjusts the value given by any gain and/or offset defined
     * for this sensor object and returns that adjusted value.
     *
     * @param[in] value - Value to be adjusted
     *
     * @return - Adjusted sensor value
     */
    SensorValueType adjustValue(SensorValueType value);

    /**
     * @brief Add value interface and value property for sensor
     * @details When a sensor has an associated input file, the Sensor.Value
     * interface is added along with setting the Value property to the
     * corresponding value found in the input file.
     *
     * @param[in] retryIO - Hwmon sysfs file retry constraints
     *                      (number of and delay between)
     * @param[in] info - Sensor object information
     *
     * @return - Shared pointer to the value object
     */
    std::shared_ptr<ValueObject> addValue(const RetryIO& retryIO,
                                          ObjectInfo& info);

    /**
     * @brief Add status interface and functional property for sensor
     * @details When a sensor has an associated fault file, the
     * OperationalStatus interface is added along with setting the
     * Functional property to the corresponding value found in the
     * fault file.
     *
     * @param[in] info - Sensor object information
     *
     * @return - Shared pointer to the status object
     */
    std::shared_ptr<StatusObject> addStatus(ObjectInfo& info);

    /**
     * @brief Unlock the gpio, set to high if relevant.
     */
    void unlockGpio();

    /**
     * @brief Lock the gpio, set to low if relevant.
     */
    void lockGpio();

  private:
    /** @brief Sensor object's identifiers */
    SensorSet::key_type sensor;

    /** @brief Hwmon sysfs access. */
    const hwmonio::HwmonIO& ioAccess;

    /** @brief Physical device sysfs path. */
    const std::string& devPath;

    /** @brief Structure for storing sensor adjustments */
    valueAdjust sensorAdjusts;

    /** @brief Optional pointer to GPIO handle. */
    std::unique_ptr<gpioplus::Handle> handle;

    /** @brief default pause after unlocking gpio. */
    static constexpr std::chrono::milliseconds pause{500};

    /** @brief sensor scale from configuration. */
    int64_t scale;
};

} // namespace sensor
