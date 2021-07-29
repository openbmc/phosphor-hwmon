#pragma once

#include "hwmonio.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
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
    ~ReadCache() = default;

    /**
     *  @brief Creates a new read request for the sensor, and returns the cached
     *    read value.
     *  @details The requested read makes a best effort to complete and update
     *    the cache with its newly read value the next time this function is
     *    called for the same sensor.
     *  @param[in] type - Hwmon type (ex. temp).
     *  @param[in] id - Hwmon ID (ex. 1).
     *  @param[in] sensor - Hwmon sensor (ex. input).
     *  @return std::int64_t
     */
    std::int64_t getSensorValue(const std::string& type, const std::string& id,
                                const std::string& sensor);

    /**
     *  @brief Create IO for event loop, hook up io_uring callback to it, and
     *    return it.
     *  @param[in] event - Event loop structure.
     *  @return std::unique_ptr<sdeventplus::source::IO>
     */
    std::unique_ptr<sdeventplus::source::IO> getIo(sdeventplus::Event& event);

    /* Keeps track of elapsed time for profiling io_uring process CQEs. */
    std::uint64_t _elapsed = 0;

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
        CacheCQEHandler(const std::string& path, ReadCache* cachePtr) :
            stdplus::IoUring::CQEHandler(), _sensorPath(path),
            _cachePtr(cachePtr)
        {
        }

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

    /**
     * @brief This determines the maximum buffer size required for a read.
     */
    static constexpr std::uint32_t MAX_READ_SIZE = 25;

    /** @brief Stores information for each read. */
    struct ReadResult
    {
        /** @brief Read value, converted from _readBuffer. */
        std::int64_t _value;
        /** @brief Handler, which also contains sensor path information. */
        std::unique_ptr<CacheCQEHandler> _cqeHandler;
        /** @brief Read value buffer. */
        std::array<char, MAX_READ_SIZE> _readBuffer = {};
        /** @brief File descriptor index (in io_uring polling array). */
        std::optional<stdplus::IoUring::FileHandle> _fileHandle = std::nullopt;
        /** @brief Return code; 0 for OK, and errno otherwise. */
        std::int32_t _rc = 0;
        /** @brief Number of remaining retries for this read operation. */
        std::uint32_t _retriesRemaining = hwmonio::retries;
    };

    /** @brief Size of cache. */
    const std::uint32_t _cacheSize;

    /** @brief For accessing hwmon io. */
    const hwmonio::HwmonIOInterface* _ioAccess;

    /** @brief Map of {sensor path: *ReadResult}. */
    std::unordered_map<std::string, std::shared_ptr<ReadResult>> _sensorMap;

    /**
     * @brief Keeps track of which sensors are currently submitted in the
     *   io_uring.
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
