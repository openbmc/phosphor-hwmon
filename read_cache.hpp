#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <sdeventplus/event.hpp>
#include <sdeventplus/source/io.hpp>
#include <stdplus/io_uring.hpp>
#include <unordered_map>

/**
 *  @class ReadCache
 *  @brief Cache for sensor reads.
 */
class ReadCache
{
  public:
    /**
     *  @brief Constructor for ReadCache.
     *  @param[in] size - For initializing cache size.
     *  @param[in] path - Hwmon instance path, for IO access.
     *  @param[in] retries - Number of maximum read retries per sensor.
     */
    explicit ReadCache(const std::uint32_t size, const std::string& path,
                       const std::uint32_t retries);

    ReadCache(const ReadCache&) = delete;
    ReadCache& operator=(const ReadCache&) = delete;
    ReadCache(ReadCache&&) = delete;
    ReadCache& operator=(ReadCache&&) = delete;
    ~ReadCache() = default;

    /**
     *  @brief Returns a cached read value.
     *  @param[in] type - Hwmon type (ex. temp).
     *  @param[in] id - Hwmon ID (ex. 1).
     *  @param[in] sensor - Hwmon sensor (ex. input).
     *  @return int64_t - Sensor read value.
     */
    std::int64_t getSensorValue(const std::string& type, const std::string& id,
                                const std::string& sensor);

    /**
     *  @brief Create IO for event loop, then hook up io_uring callback.
     *  @param[in] event - Event loop structure.
     */
    std::unique_ptr<sdeventplus::source::IO> getIo(sdeventplus::Event& event);

  private:
    /**
     *  @class CacheCQEHandler
     *  @brief Handles finished CQEs.
     */
    class CacheCQEHandler : public stdplus::IoUring::CQEHandler
    {
      public:
        /**
         *  @brief Constructor for CacheCQEHandler.
         *  @param[in] path - Sensor path.
         *  @param[in] cachePtr - Pointer to current instance of the cache.
         */
        CacheCQEHandler(const std::string& path, ReadCache* cachePtr);

        /**
         * @brief Overriden method for handling CQEs on completion.
         * @param[in] cqe - Completed CQE from io_uring.
         */
        void handleCQE(io_uring_cqe& cqe) noexcept;

      private:
        /** @brief Sensor path. */
        const std::string _sensorPath;

        /** @brief Pointer to outer cache instance. */
        ReadCache* _cachePtr;
    };

    /** @brief Stores information for each read. */
    struct ReadResult
    {
        /** @brief Handler, which also contains sensor path information. */
        std::unique_ptr<CacheCQEHandler> _handler;
        /** @brief Read value. */
        std::int64_t _value;
        /** @brief File descriptor. */
        int _fd;
        /** @brief Return code; 0 for OK, and errno otherwise. */
        int _rc;
        /** @brief Number of remaining retries for this read operation. */
        std::size_t _retriesRemaining;

        /** @brief Constructor for ReadResult; used because of make_shared. */
        ReadResult(std::unique_ptr<CacheCQEHandler> handler, std::int64_t value,
                   int fd, int rc, std::size_t retriesRemaining);
    };

    /** @brief Hwmon instance path, for IO access. */
    const std::string _hwmonPath;

    /** @brief Maximum possible retries for each sensor read. */
    const std::uint32_t _retries;

    /** @brief {sensor path: *ReadResult struct} */
    std::unordered_map<std::string, std::shared_ptr<ReadResult>> _sensorMap;

    /** @brief Ring for async IO. */
    std::unique_ptr<stdplus::IoUring> _ring;

    /**
     * @brief Submit a SQE for a sensor read.
     * @param[in] path - The sensor path.
     */
    void submitReadRequest(const std::string& path);
};
