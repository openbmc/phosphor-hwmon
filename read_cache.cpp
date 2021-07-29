#include "config.h"

#include "read_cache.hpp"

#include "sysfs.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <phosphor-logging/elog-errors.hpp>
#include <system_error>
#include <vector>

using namespace phosphor::logging;

static constexpr auto retryableErrors = {
    /*
     * Retry on bus or device errors or timeouts in case
     * they are transient.
     */
    EIO,
    ETIMEDOUT,

    /*
     * Retry CRC errors.
     */
    EBADMSG,

    /*
     * Some hwmon drivers do this when they aren't ready
     * instead of blocking.  Retry.
     */
    EAGAIN,
    /*
     * We'll see this when for example i2c devices are
     * unplugged but the driver is still bound.  Retry
     * rather than exit on the off chance the device is
     * plugged back in and the driver doesn't do a
     * remove/probe.  If a remove does occur, we'll
     * eventually get ENOENT.
     */
    ENXIO,

    /*
     * Some devices return this when they are busy doing
     * something else.  Even if being busy isn't the cause,
     * a retry still gives this app a shot at getting data
     * as opposed to failing out on the first try.
     */
    ENODATA,

    /*
     * Some devices return this if the hardware is being
     * powered off in a normal manner, as incomplete data
     * is received. Retrying allows time for the system to
     * clean up driver devices, or in the event of a real
     * failure, attempt to get the rest of the data.
     */
    EMSGSIZE,
};

ReadCache::ReadCache(const std::uint32_t size, const std::string& path,
                     const std::uint32_t retries) :
    _cacheSize(size),
    _hwmonPath(path), _retries(retries),
    _ring(std::make_unique<stdplus::IoUring>(size))
{
    _sensorMap.reserve(size);
    _submittedSet.reserve(size);
}

ReadCache::~ReadCache()
{
    for (auto const& e : _sensorMap)
    {
        int fd = (e.second)->_fd;
        if (fd != -1 && close(fd) == -1)
        {
            log<level::ERR>(fmt::format("io_uring FAIL: fd {} close ({})", fd,
                                        std::strerror(errno))
                                .c_str());
        }
    }
}

std::int64_t ReadCache::getSensorValue(const std::string& type,
                                       const std::string& id,
                                       const std::string& sensor)
{
    std::string pathKey = sysfs::make_sysfs_path(_hwmonPath, type, id, sensor);

    // Sensor value does not exist in cache yet; initialize.
    if (_sensorMap.count(pathKey) == 0)
    {
        std::shared_ptr<ReadResult> resultPtr = std::make_shared<ReadResult>(
            std::make_unique<CacheCQEHandler>(pathKey, this), 0, -1, 0,
            _retries, std::chrono::high_resolution_clock::time_point::min());
        _sensorMap.emplace(pathKey, std::move(resultPtr));

        if (_sensorMap.size() > _cacheSize)
        {
            log<level::ERR>(
                fmt::format("io_uring FAIL: cache size exceeded {}", _cacheSize)
                    .c_str());
            throw std::system_error(ENOMEM, std::generic_category());
        }

        submitReadRequest(pathKey);
        return 1;
    }

    std::shared_ptr<ReadResult> result = _sensorMap[pathKey];
    std::int64_t sensorValue = result->_value;
    int rc = result->_rc;

    // Already submitted in io_uring. Get cache (possibly stale) value.
    if (_submittedSet.count(pathKey) != 0)
    {
        return sensorValue;
    }

    // Successful read.
    if (rc == 0)
    {
        result->_retriesRemaining = _retries;
        submitReadRequest(pathKey);

        return sensorValue;
    }

    // If the directory or device disappeared then this application
    // should gracefully exit.  There are race conditions between
    // the unloading of a hwmon driver and the stopping of this
    // service by systemd.  To prevent this application from falsely
    // failing in these scenarios, it will simply exit if the
    // directory or file can not be found.  It is up to the user(s)
    // of this provided hwmon object to log the appropriate errors
    // if the object disappears when it should not.
    if (rc == ENOENT || rc == ENODEV)
    {
        log<level::ERR>(
            fmt::format("io_uring FAIL: exiting with ({})", std::strerror(rc))
                .c_str());
        exit(0);
    }

    // No more retries, or unretryable errror encountered.
    int count = std::count(retryableErrors.begin(), retryableErrors.end(), rc);
    if (count == 0 || result->_retriesRemaining == 0)
    {
        if (count == 0)
        {
            log<level::ERR>(
                fmt::format("io_uring FAIL: {} has unretryable error ({})",
                            pathKey, std::strerror(rc))
                    .c_str());
        }
        else
        {
            log<level::ERR>(
                fmt::format("io_uring FAIL: {} out of retries", pathKey)
                    .c_str());
        }

        result->_retriesRemaining = _retries;

#if NEGATIVE_ERRNO_ON_FAIL
        return -rc;
#endif
        throw std::system_error(rc, std::generic_category());
    }

    // Makes sure that retry attempt is not submitted too early from the
    // last returned result.
    std::chrono::milliseconds delta =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - result->_timeStamp);
    if (delta.count() >= 100)
    {
        log<level::INFO>(fmt::format("io_uring: {} has retryable error ({})",
                                     pathKey, std::strerror(rc))
                             .c_str());

        result->_retriesRemaining--;
        submitReadRequest(pathKey);
    }

    return sensorValue;
}

std::unique_ptr<sdeventplus::source::IO>
    ReadCache::getIo(sdeventplus::Event& event)
{
    log<level::INFO>("io_uring: IO added to event loop.");

    return std::make_unique<sdeventplus::source::IO>(
        event, _ring->getEventFd().get(), EPOLLIN | EPOLLET,
        [this](sdeventplus::source::IO&, int, std::uint32_t) {
            _ring->processEvents();
        });
}

void ReadCache::submitReadRequest(const std::string& path)
{
    std::shared_ptr<ReadResult> ptr = _sensorMap[path];

    struct io_uring_sqe& sqe = _ring->getSQE();

    if (ptr->_fd == -1)
    {
        int fd = open(path.c_str(), O_RDONLY);
        if (fd == -1)
        {
            log<level::ERR>(fmt::format("io_uring FAIL: fd {} open ({})", fd,
                                        std::strerror(errno))
                                .c_str());

            ptr->_rc = errno;
            return;
        }
        ptr->_fd = fd;
    }

    io_uring_prep_read(&sqe, ptr->_fd, &(ptr->_value), sizeof(std::int64_t), 0);
    _ring->setHandler(sqe, ptr->_handler.get());
    _ring->submit();

    _submittedSet.insert(path);
}

ReadCache::CacheCQEHandler::CacheCQEHandler(const std::string& path,
                                            ReadCache* cachePtr) :
    stdplus::IoUring::CQEHandler(),
    _sensorPath(path), _cachePtr(cachePtr)
{
}

void ReadCache::CacheCQEHandler::handleCQE(io_uring_cqe& cqe) noexcept
{
    std::shared_ptr<ReadResult> resultPtr =
        (_cachePtr->_sensorMap)[_sensorPath];
    resultPtr->_timeStamp = std::chrono::high_resolution_clock::now();
    resultPtr->_rc = cqe.res >= 0 ? 0 : -cqe.res;

    (_cachePtr->_submittedSet).erase(_sensorPath);
}

ReadCache::ReadResult::ReadResult(
    std::unique_ptr<CacheCQEHandler> handler, std::int64_t value, int fd,
    int rc, std::size_t retriesRemaining,
    std::chrono::time_point<std::chrono::high_resolution_clock> timeStamp) :
    _handler(std::move(handler)),
    _value(value), _fd(fd), _rc(rc), _retriesRemaining(retriesRemaining),
    _timeStamp(timeStamp)
{
}
