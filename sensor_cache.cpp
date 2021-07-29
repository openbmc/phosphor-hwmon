#include "sensor_cache.hpp"

#include "sysfs.hpp"

#include <fcntl.h>

#include <cerrno>
#include <system_error>

SensorCache::SensorCache(const std::uint32_t size, const std::string& path) :
    _path(path) {
  io_uring_queue_init(size, &ring, 0);
  _sensorMap.reserve(size);
  _submittedSet.reserve(size);
}

SensorCache::~SensorCache() {
  io_uring_queue_exit(&ring);
}

std::int64_t SensorCache::getSensorValue(
  const std::string& type,
  const std::string& id,
  const std::string& sensor
) {
  std::string key = sysfs::make_sysfs_path(_path, type, id, sensor);
  if (!_sensorMap.count(key)) {
    _sensorMap.emplace(
      key,
      std::make_shared<ReadResult>(ReadResult{key, 0, 0, 0, 0})
    );
    // throw std::runtime_error("Sensor key does not exist.");
    return 0;
  }

  std::shared_ptr<ReadResult> ptr = _sensorMap[key]
  int64_t sensorValue = ptr->value;
  int rc = ptr->rc;

  // handle all errors here
  // ....
  if (rc == ENOENT || rc == ENODEV) {
    exit(0);
  }

  return sensorValue;
}

void SensorCache::submitReadRequests() {
  for (auto& e : _sensorMap) {
    const std::string path_key = e.first;
    std::shared_ptr<ReadResult> ptr = e.second;

    // If path is already in the SQ, do not add again.
    if (!_submittedSet.find(path_key)) {
      continue;
    }

    struct io_uring_sqe* sqe = io_uring_get_sqe(&_ring);
    if (!sqe) {
      ptr->rc = ENOBUFS;
      continue;
    }

    int fd = open(path_key, O_RDONLY);
    if (fd) {
      ptr->rc = errno;
      continue;
    }
    ptr->fd = fd;

    io_uring_prep_read(
      sqe,
      fd,
      &(ptr->value),
      sizeof(int64_t),
      0
    );
    io_uring_sqe_set_data(sqe, ptr.get());

    _submittedSet.insert(path_key);
  }

  io_uring_submit(&_ring);
}

void SensorCache::updateCacheValues() {
  struct io_uring_cqe* cqe;

  int rc = -io_uring_peek_cqe(&_ring, &cqe);
  while (!rc) {
    ReadResult* result = io_uring_cqe_get_data(cqe);

    io_uring_cqe_seen(&_ring, cqe);
    _submittedSet.remove(result->path);

    if (close(result->fd)) {
      result->rc = errno;
    }
  }

  if (rc != EAGAIN || rc != EWOULDBLOCK) {
    throw std::system_error(rc, "Liburing failure.");
  }
}
