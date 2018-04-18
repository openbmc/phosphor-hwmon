#pragma once

#include "sensorset.hpp"
#include "mainloop.hpp"

namespace sensor
{

std::shared_ptr<StatusObject> addStatus(
        const SensorSet::key_type& sensor,
        const hwmonio::HwmonIO& ioAccess,
        const std::string& devPath,
        ObjectInfo& info);

} // namespace sensor
