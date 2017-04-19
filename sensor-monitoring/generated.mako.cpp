## This file is a template, the comment below is emitted into the generated file
/* This is an auto generated file. Do not edit. */
#include "data_types.hpp"
#include "monitor.hpp"
#include "functor.hpp"
#include "conditions.hpp"
#include "actions.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

% for g in events["groups"]:
static Group ${g['name']} = {
    % for m in g['members']:
    std::make_tuple(
        "/xyz/openbmc_project/sensors/${str(g['type'])}/${str(m)}",
        static_cast<${g['value_type']}>(${g['value_init']})
    ),
    % endfor
};
% endfor

const std::vector<std::tuple<std::vector<std::shared_ptr<Event>>,
                             std::vector<Action>>>
    Monitor::events
{

};

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
