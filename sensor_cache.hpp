#pragma once

#include <liburing.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>

/**
 *  @class SensorCache
 *  @brief Cache for async sensor reads.
 */
class SensorCache {
  public:
    /**
     *  @brief Constructor for SensorCache.
     *  @param[in] size - For initializing cache size.
     *  @param[in] path - Hwmon instance path, for IO access. 
     */
    explicit SensorCache(const std::uint32_t size, const std::string& path);
    SensorCache(const SensorCache&) = delete;
    SensorCache& operator = (const SensorCache&) = delete;
    SensorCache(SensorCache&&) = delete;
    SensorCache& operator = (SensorCache&&) = delete;
    ~SensorCache();

    /**
     *  @brief Returns a cached read value.
     *  @param[in] type - Hwmon type (ex. temp).
     *  @param[in] id - Hwmon ID (ex. 1).
     *  @param[in] sensor - Hwmon sensor (ex. input).
     *  @return int64_t - Sensor read value.
     */
    std::int64_t getSensorValue(
      const std::string& type,
      const std::string& id,
      const std::string& sensor
    );

    /**
     * @brief Iterate through _sensorMap and submit SQEs for io_uring, if they
     *   are not already in the SQ (determined using _submittedSet).
     */
    void submitReadRequests();

    /** @brief Process any finished CQEs. Update _sensorMap. */
    void updateCacheValues();

  private:

    /** @brief Stores information for each read. */
    struct ReadResult {
      /** @brief Path of hwmon sensor. */
      std::string path;
      /** @brief Read value. */
      std::int64_t value;
      /** @brief File descriptor. */
      int fd;
      /** @brief Return code; 0 for OK, and errno otherwise. */
      int rc;
      /** @brief Number of remaining retries for this read operation. */
      std::size_t retriesRemaining;
    };

    /** @brief Hwmon instance path, for IO access. */
    const std::string _path;

    /** @brief {sensor path: *ReadResult struct} */
    std::unordered_map<std::string, std::shared_ptr<ReadResult>> _sensorMap;

    /**
     * @brief Set of sensor paths, detailing which sensors are currently in
     *   the SQ.
     */
    std::unordered_set<std::string> _submittedSet

    /** @brief Ring for async IO. */
    struct io_uring ring;
}
