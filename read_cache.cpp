#include "config.h"

#include "read_cache.hpp"

#include "sysfs.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <phosphor-logging/elog-errors.hpp>
#include <system_error>

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

ReadCache::ReadCache(
  const std::uint32_t size,
  const std::string& path,
  const std::uint32_t retries
) : _hwmonPath(path),
    _retries(retries),
    _ring(std::make_unique<stdplus::IoUring>(size)) {
  _sensorMap.reserve(size);
}

std::int64_t ReadCache::getSensorValue(
  const std::string& type,
  const std::string& id,
  const std::string& sensor
) {
  std::string pathKey = sysfs::make_sysfs_path(_hwmonPath, type, id, sensor);

  // Sensor value does not exist in cache yet; initialize.
  if (_sensorMap.count(pathKey) == 0) {
    std::shared_ptr<ReadResult> resultPtr = std::make_shared<ReadResult>(
      std::make_unique<CacheCQEHandler>(pathKey, this),
      0,
      0,
      0,
      _retries
    );
    _sensorMap.emplace(pathKey, std::move(resultPtr));

    submitReadRequest(pathKey);
    return 0;
  }

  std::shared_ptr<ReadResult> result = _sensorMap[pathKey];
  std::int64_t sensorValue = result->_value;
  int rc = result->_rc;

  // handle all errors here
  if (rc == ENOENT || rc == ENODEV) {
    exit(0);
  }

  if (
    std::count(retryableErrors.begin(), retryableErrors.end(), rc) == 0
    || !(result->_retriesRemaining)
  ) {
    result->_retriesRemaining = _retries;

#if NEGATIVE_ERRNO_ON_FAIL
    return -rc;
#endif
    throw std::system_error(rc, std::generic_category());
  }

  if (rc) {
    result->_retriesRemaining--;
  } else {
    result->_retriesRemaining = _retries;
  }

  submitReadRequest(pathKey);
  return sensorValue;
}

std::unique_ptr<sdeventplus::source::IO> ReadCache::getIo(
  sdeventplus::Event& event
) {
  log<level::INFO>("io_uring added to event loop.");

  return std::make_unique<sdeventplus::source::IO>(
    event,
    _ring->getEventFd().get(),
    EPOLLIN | EPOLLET,
    [this](sdeventplus::source::IO&, int, std::uint32_t) {
      log<level::INFO>("Processing io_uring reads...");
      _ring->processEvents();
    }
  );
}

void ReadCache::submitReadRequest(const std::string& path) {
  std::shared_ptr<ReadResult> ptr = _sensorMap[path];

  struct io_uring_sqe& sqe = _ring->getSQE();

  int fd = open(path.c_str(), O_RDONLY);
  if (fd) {
    ptr->_rc = errno;
    return;
  }
  ptr->_fd = fd;

  io_uring_prep_read(
    &sqe,
    fd,
    &(ptr->_value),
    sizeof(std::int64_t),
    0
  );

  _ring->setHandler(sqe, ptr->_handler.get());
  _ring->submit();
}

ReadCache::CacheCQEHandler::CacheCQEHandler(
  const std::string& path,
  ReadCache* cachePtr
) : stdplus::IoUring::CQEHandler(),
    _sensorPath(path),
    _cachePtr(cachePtr) {}

void ReadCache::CacheCQEHandler::handleCQE(io_uring_cqe& cqe) noexcept {
  std::shared_ptr<ReadResult> resultPtr = (_cachePtr->_sensorMap)[_sensorPath];
  int rc = cqe.res;

  if (close(resultPtr->_fd)) {
    resultPtr->_rc = errno;
    return;
  }

  resultPtr->_rc = rc;
}

ReadCache::ReadResult::ReadResult(
  std::unique_ptr<CacheCQEHandler> handler,
  std::int64_t value,
  int fd,
  int rc,
  std::size_t retriesRemaining
) : _handler(std::move(handler)),
    _value(value),
    _fd(fd),
    _rc(rc),
    _retriesRemaining(retriesRemaining) {}
