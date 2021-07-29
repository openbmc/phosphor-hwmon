#pragma once

#include "hwmonio.hpp"

#include <cstdint>
#include <memory>
#include <sdeventplus/event.hpp>
#include <sdeventplus/source/io.hpp>
#include <stdplus/io_uring.hpp>
#include <unordered_map>
#include <unordered_set>

/**
 *  @class ReadCache
 *  @brief Cache for sensor reads. Utilizes io_uring library.
 */
class ReadCache
{
  public:
    /**
     *  @brief Constructor for ReadCache.
     *  @param[in] size - For initializing cache size.
     *  @param[in] ioAccess - Access to hwmon io.
     */
    explicit ReadCache(const std::uint32_t size,
                       const hwmonio::HwmonIOInterface* ioAccess);

    ReadCache(const ReadCache&) = delete;
    ReadCache& operator=(const ReadCache&) = delete;
    ReadCache(ReadCache&&) = delete;
    ReadCache& operator=(ReadCache&&) = delete;
    ~ReadCache();

    /**
     *  @brief Creates a new read request for the sensor, and returns the cached
     *  read value.
     *  @details The requested read makes a best effort to complete and update
     *  the cache with its newly read value the next time this function is
     *  called for the same sensor.
     *  @param[in] type - Hwmon type (ex. temp).
     *  @param[in] id - Hwmon ID (ex. 1).
     *  @param[in] sensor - Hwmon sensor (ex. input).
     *  @return std::int64_t
     */
    std::int64_t getSensorValue(const std::string& type, const std::string& id,
                                const std::string& sensor);

    /**
     *  @brief Create IO for event loop, hook up io_uring callback to it, and
     *  return it.
     *  @param[in] event - Event loop structure.
     *  @return std::unique_ptr<sdeventplus::source::IO>
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
        /** @brief Read value, converted from _readBuffer. */
        std::int64_t _value;
        /** @brief Handler, which also contains sensor path information. */
        std::unique_ptr<CacheCQEHandler> _handler;
        /** @brief Read value buffer. */
        std::unique_ptr<char[]> _readBuffer;
        /** @brief File descriptor. */
        std::int32_t _fd;
        /** @brief Return code; 0 for OK, and errno otherwise. */
        std::int32_t _rc;
        /** @brief Number of remaining retries for this read operation. */
        std::uint32_t _retriesRemaining;

        /** @brief Constructor for ReadResult; used because of make_shared(). */
        ReadResult(std::int64_t value, std::unique_ptr<CacheCQEHandler> handler,
                   std::unique_ptr<char[]> readBuffer, std::int32_t fd,
                   std::int32_t rc, std::uint32_t retriesRemaining);
    };

    /** @brief Size of cache. */
    const std::uint32_t _cacheSize;

    /** @brief For accessing hwmon io. */
    const hwmonio::HwmonIOInterface* _ioAccess;

    /** @brief Map of {sensor path: *ReadResult}. */
    std::unordered_map<std::string, std::shared_ptr<ReadResult>> _sensorMap;

    /**
     * @brief Keeps track of which sensors are currently submitted in the
     * io_uring.
     */
    std::unordered_set<std::string> _submittedSet;

    /** @brief Ring for async IO. */
    std::unique_ptr<stdplus::IoUring> _ring;

    /**
     * @brief Prepare a SQE for a sensor read and submit.
     * @param[in] path - The sensor path.
     * @return std::int32_t - Return code of success/failure.
     */
    std::int32_t processReadRequest(const std::string& path);
};
