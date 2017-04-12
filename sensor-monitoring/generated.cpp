#include "data_types.hpp"
#include "monitor.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

const std::vector<std::tuple<std::vector<std::shared_ptr<Event>>,
                             std::vector<Action>>>
    Monitor::events
{
     // TODO openbmc/openbmc#1493
     //      Fill in with events parsed from sensor monitor definition YAML
};

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
