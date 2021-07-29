#include "config.h"

#include "read_cache.hpp"

#include "sysfs.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <limits>
#include <phosphor-logging/elog-errors.hpp>
#include <system_error>
#include <vector>

using namespace phosphor::logging;

/* Utility functions and values. */
namespace
{
/**
 * @brief List of errors that can occur during sensor reads. An attempt should
 * be made to re-try the read operation.
 */
static constexpr auto RETRYABLE_ERRORS = {
    /* Retry on bus or device errors or timeouts in case they are transient. */
    EIO,
    ETIMEDOUT,
    /* Retry CRC errors. */
    EBADMSG,
    /*
     * Some hwmon drivers do this when they aren't ready instead of blocking.
     * Retry.
     */
    EAGAIN,
    /*
     * We'll see this when for example i2c devices are unplugged but the driver
     * is still bound. Retry rather than exit on the off chance the device is
     * plugged back in and the driver doesn't do a remove/probe. If a remove
     * does occur, we'll eventually get ENOENT.
     */
    ENXIO,
    /*
     * Some devices return this when they are busy doing something else. Even
     * if being busy isn't the cause, a retry still gives this app a shot at
     * getting data as opposed to failing out on the first try.
     */
    ENODATA,
    /*
     * Some devices return this if the hardware is being powered off in a normal
     * manner, as incomplete data is received. Retrying allows time for the
     * system to clean up driver devices, or in the event of a real failure,
     * attempt to get the rest of the data.
     */
    EMSGSIZE,
};

/**
 * @brief Since sensor values are of type int64_t, this determines the maximum
 * buffer size required for a read.
 */
static const uint32_t MAX_READ_SIZE =
    std::to_string(std::numeric_limits<std::int64_t>::min()).length() + 1;

/**
 * @brief Returns int64_t sensor value from read() buffer.
 * @param[in] bytes - Number of bytes read from read().
 * @param[in] buffer - Pre-allocated char buffer containing read() value.
 * @return std::int64_t
 */
std::int64_t getReadResult(std::uint32_t bytes, char* buffer)
{
    /* Last byte from read() potentially marks EOF; clear and set NULL char. */
    if (bytes <= 1)
    {
        buffer[0] = '\0';
    }
    else
    {
        buffer[bytes - 1] = '\0';
    }

    return buffer[0] == '\0' ? 0 : std::atoll(buffer);
}

/**
 * @brief Handles cases of ENOENT and ENODEV.
 * @details If the directory or device disappeared then this application should
 * gracefully exit. There are race conditions between the unloading of a hwmon
 * driver and the stopping of this service by systemd. To prevent this
 * application from falsely failing in these scenarios, it will simply exit if
 * the directory or file can not be found. It is up to the user(s) of this
 * provided hwmon object to log the appropriate errors if the object disappears
 * when it should not.
 * @param[in] rc - Return code.
 * @param[in] path - Sensor that yielded the return code.
 */
void checkForExit(std::int32_t rc, const std::string& path)
{
    if (rc == ENOENT || rc == ENODEV)
    {
        log<level::INFO>(
            fmt::format("io_uring: reading sensor {} returned ({}), exiting",
                        path, std::strerror(rc))
                .c_str());
        exit(0);
    }
}

/**
 * @brief Handles error code.
 * @param[in] rc - Return code.
 */
std::int64_t handleError(std::int32_t rc)
{
#if NEGATIVE_ERRNO_ON_FAIL
    return -rc;
#endif
    throw std::system_error(rc, std::generic_category());
}
} /* namespace */

ReadCache::ReadCache(const std::uint32_t size,
                     const hwmonio::HwmonIOInterface* ioAccess) :
    _cacheSize(size),
    _ioAccess(ioAccess), _ring(std::make_unique<stdplus::IoUring>(size))
{
    _sensorMap.reserve(size);
    _submittedSet.reserve(size);
}

ReadCache::~ReadCache()
{
    /* Close any open file descriptors. */
    for (auto const& e : _sensorMap)
    {
        std::int32_t fd = e.second->_fd;
        if (fd != -1 && close(fd) == -1)
        {
            log<level::ERR>(
                fmt::format("io_uring FAIL: close() of {} failed ({})", e.first,
                            std::strerror(errno))
                    .c_str());
        }
    }
}

std::int64_t ReadCache::getSensorValue(const std::string& type,
                                       const std::string& id,
                                       const std::string& sensor)
{
    std::string path =
        sysfs::make_sysfs_path(_ioAccess->path(), type, id, sensor);

    /* Sensor value does not exist in cache yet; initialize. */
    if (_sensorMap.count(path) == 0)
    {
        /* Initial read is synchronous. */
        std::int64_t initialValue =
            _ioAccess->read(type, id, sensor, hwmonio::retries, hwmonio::delay);

        std::shared_ptr<ReadResult> resultPtr = std::make_shared<ReadResult>(
            initialValue,
            std::move(std::make_unique<CacheCQEHandler>(path, this)),
            std::move(std::make_unique<char[]>(MAX_READ_SIZE)), -1, 0,
            hwmonio::retries);

        if (_sensorMap.size() == _cacheSize)
        {
            log<level::ERR>(
                fmt::format("io_uring FAIL: cache size exceeded {}", _cacheSize)
                    .c_str());

            return handleError(ENOMEM);
        }

        _sensorMap.emplace(path, std::move(resultPtr));
        std::int32_t requestRc = processReadRequest(path);

        return requestRc == 0 ? initialValue : handleError(requestRc);
    }

    std::shared_ptr<ReadResult> result = _sensorMap[path];

    std::int64_t sensorValue = result->_value;
    std::int32_t rc = result->_rc;

    /*
     * Current sensor already has read operation submitted in io_uring. Get
     * cached value.
     */
    if (_submittedSet.count(path) != 0)
    {
        return sensorValue;
    }

    /* Successful read. */
    if (rc == 0)
    {
        result->_retriesRemaining = hwmonio::retries;
        std::int32_t requestRc = processReadRequest(path);

        return requestRc == 0 ? sensorValue : handleError(requestRc);
    }

    checkForExit(rc, path);

    /* No more retries, or unretryable errror encountered. */
    std::int32_t count =
        std::count(RETRYABLE_ERRORS.begin(), RETRYABLE_ERRORS.end(), rc);
    if (count == 0 || result->_retriesRemaining == 0)
    {
        if (count == 0)
        {
            log<level::ERR>(
                fmt::format("io_uring FAIL: {} has unretryable error ({})",
                            path, std::strerror(rc))
                    .c_str());
        }
        else
        {
            log<level::ERR>(
                fmt::format("io_uring FAIL: {} out of retries", path).c_str());
        }

        result->_retriesRemaining = hwmonio::retries;
        return handleError(rc);
    }

    log<level::DEBUG>(fmt::format("io_uring: {} has retryable error ({})", path,
                                  std::strerror(rc))
                          .c_str());

    result->_retriesRemaining--;
    std::int32_t requestRc = processReadRequest(path);

    return requestRc == 0 ? sensorValue : handleError(requestRc);
}

std::unique_ptr<sdeventplus::source::IO>
    ReadCache::getIo(sdeventplus::Event& event)
{
    log<level::DEBUG>("io_uring: IO added to event loop");

    return std::make_unique<sdeventplus::source::IO>(
        event, _ring->getEventFd().get(), EPOLLIN | EPOLLET,
        [this](sdeventplus::source::IO&, int, std::uint32_t) {
            _ring->processEvents();
        });
}

std::int32_t ReadCache::processReadRequest(const std::string& path)
{
    std::shared_ptr<ReadResult> ptr = _sensorMap[path];

    struct io_uring_sqe& sqe = _ring->getSQE();

    if (ptr->_fd == -1)
    {
        std::int32_t fd = open(path.c_str(), O_RDONLY);
        if (fd == -1)
        {
            std::int32_t rc = errno;
            log<level::ERR>(
                fmt::format("io_uring FAIL: open() of {} failed ({})", path,
                            std::strerror(rc))
                    .c_str());
            ptr->_rc = rc;

            checkForExit(rc, path);

            return rc;
        }
        ptr->_fd = fd;
    }

    io_uring_prep_read(&sqe, ptr->_fd, ptr->_readBuffer.get(), MAX_READ_SIZE,
                       0);
    _ring->setHandler(sqe, ptr->_handler.get());
    _ring->submit();

    _submittedSet.insert(path);

    return 0;
}

ReadCache::CacheCQEHandler::CacheCQEHandler(const std::string& path,
                                            ReadCache* cachePtr) :
    stdplus::IoUring::CQEHandler(),
    _sensorPath(path), _cachePtr(cachePtr)
{
}

void ReadCache::CacheCQEHandler::handleCQE(io_uring_cqe& cqe) noexcept
{
    std::shared_ptr<ReadResult> resultPtr = _cachePtr->_sensorMap[_sensorPath];

    /* Return value is either number of bytes from read(), or -errno. */
    std::int32_t res = cqe.res;
    if (res >= 0)
    {
        resultPtr->_rc = 0;
        resultPtr->_value = getReadResult(res, resultPtr->_readBuffer.get());
    }
    else
    {
        resultPtr->_rc = -res;
    }

    _cachePtr->_submittedSet.erase(_sensorPath);
}

ReadCache::ReadResult::ReadResult(std::int64_t value,
                                  std::unique_ptr<CacheCQEHandler> handler,
                                  std::unique_ptr<char[]> readBuffer,
                                  std::int32_t fd, std::int32_t rc,
                                  std::uint32_t retriesRemaining) :
    _value(value),
    _handler(std::move(handler)), _readBuffer(std::move(readBuffer)), _fd(fd),
    _rc(rc), _retriesRemaining(retriesRemaining)
{
}
