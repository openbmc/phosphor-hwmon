#include "read_cache.hpp"

#include "sysfs.hpp"

#include <fcntl.h>

#include <cerrno>
#include <system_error>

ReadCache::ReadCache(const std::uint32_t size, const std::string& path) :
    _path(path), _ring(std::make_unique<stdplus::IoUring>(size)) {
  _sensorMap.reserve(size);
  _submittedSet.reserve(size);
}

std::int64_t ReadCache::getSensorValue(
  const std::string& type,
  const std::string& id,
  const std::string& sensor
) {
  std::string pathKey = sysfs::make_sysfs_path(_path, type, id, sensor);

  // Sensor value does not exist in cache yet; initialize.
  if (!_sensorMap.count(pathKey)) {
    _sensorMap.emplace(
      pathKey,
      std::make_shared<ReadResult>(ReadResult{pathKey, 0, 0, 0, 0})
    );

    submitReadRequest(pathKey);
    return 0;
  }

  std::shared_ptr<ReadResult> result = _sensorMap[pathKey]
  int64_t sensorValue = result->value;
  int rc = presulttr->rc;

  // handle all errors here
  // ....
  if (rc == ENOENT || rc == ENODEV) {
    exit(0);
  }

  submitReadRequest(pathKey);
  return sensorValue;
}

std::unique_ptr<sdeventplus::source::IO> ReadCache::getIO(
  sdeventplus::Event& event
) {
  return std::make_unique<sdeventplus::source::IO>(
    event,
    _ring->getEventFd().get(),
    EPOLLIN | EPOLLET,
    [&_ring](IO&, int, uint32_t) { _ring->process(); }
  );
}

void ReadCache::submitReadRequest(const std::string& path) {
  std::shared_ptr<ReadResult> ptr = _sensorMap[path];

  struct io_uring_sqe& sqe = _ring->getSQE();

  int fd = open(path, O_RDONLY);
  if (fd) {
    ptr->rc = errno;
    return;
  }
  ptr->fd = fd;

  io_uring_prep_read(
    &sqe,
    fd,
    &(ptr->value),
    sizeof(int64_t),
    0
  );
  io_uring_sqe_set_data(&sqe, ptr.get());
  _ring->setHandler(&sqe, &_handler);
  _ring->submit();
}

void ReadCache::CacheCQEHandler::handleCQE(io_uring_cqe& cqe) {
  ReadResult* result = io_uring_cqe_get_data(&cqe);
  int rc = cqe.res;

  if (close(result->fd)) {
    result->rc = errno;
    return;
  }

  result->rc = rc;
}
