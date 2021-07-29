#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdplus/io_uring.hpp>
#include <sdeventplus/event.hpp>
#include <sdeventplus/source/io.hpp>
#include <unordered_map>

/**
 *  @class ReadCache
 *  @brief Cache for sensor reads.
 */
class ReadCache {
  public:
    /**
     *  @brief Constructor for ReadCache.
     *  @param[in] size - For initializing cache size.
     *  @param[in] path - Hwmon instance path, for IO access. 
     */
    explicit ReadCache(const std::uint32_t size, const std::string& path);
    ReadCache(const ReadCache&) = delete;
    ReadCache& operator = (const ReadCache&) = delete;
    ReadCache(ReadCache&&) = delete;
    ReadCache& operator = (ReadCache&&) = delete;
    ~ReadCache() = default;

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
     *  @brief Create IO for event loop, then hook up io_uring callback.
     *  @param[in] event - Event loop structure.
     */
    std::unique_ptr<sdeventplus::source::IO> getIO(sdeventplus::Event& event);

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

    /** @brief Ring for async IO. */
    std::unique_ptr<stdplus::IoUring> _ring;

    /**
     * @brief Submit a SQE for a sensor read.
     * @param[in] path - The sensor path.
     */
    void submitReadRequest(const std::string& path);

    /** @brief Process any finished CQEs. Update _sensorMap. */
    void updateCacheValues();

    /**
     *  @class CacheCQEHandler
     *  @brief Handles finished CQEs.
     */
    class CacheCQEHandler : public stdplus::IoUring::CQEHandler {
      public:
        void handleCQE(io_uring_cqe& cqe);
    };

    /** @brief Uses finished CQEs to update _sensorMap. */
    CacheCQEHandler _handler;
}
