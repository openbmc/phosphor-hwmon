#include "data_types.hpp"
#include "functor.hpp"
#include "monitor.hpp"
#include "conditions.hpp"
#include "actions.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

// Example vector of sensors constructing a group
static Group fan_zone_group = {
    std::make_tuple(
        "/xyz/openbmc_project/sensors/fan_tach/fan0",
        static_cast<int64_t>(0)
    ),
    std::make_tuple(
        "/xyz/openbmc_project/sensors/fan_tach/fan1",
        static_cast<int64_t>(0)
    ),
    std::make_tuple(
        "/xyz/openbmc_project/sensors/fan_tach/fan2",
        static_cast<int64_t>(0)
    ),
    std::make_tuple(
        "/xyz/openbmc_project/sensors/fan_tach/fan3",
        static_cast<int64_t>(0)
    )
};

const std::vector<std::tuple<std::vector<std::shared_ptr<Event>>,
                             std::vector<Action>>>
    Monitor::events
{ // Example vector of Events with START trigger
    {std::make_tuple(std::vector<std::shared_ptr<Event>>(
        { // Example vector of StartEvent
            std::make_shared<StartEvent>(
                std::vector<Condition>(
                    { // Example vector of StartEvent conditions
                        make_condition(propertyStart<int64_t>(
                            "/xyz/openbmc_project/sensors/fan_tach/fan0",
                            "xyz.openbmc_project.Sensor.Value",
                            "Value",
                            condition::countAtOrAbove(
                                fan_zone_group,
                                "/xyz/openbmc_project/sensors/fan_tach/fan0",
                                static_cast<size_t>(3),
                                static_cast<int64_t>(8000ll)
                            )
                        )),
                        make_condition(propertyStart<int64_t>(
                            "/xyz/openbmc_project/sensors/fan_tach/fan1",
                            "xyz.openbmc_project.Sensor.Value",
                            "Value",
                            condition::countAtOrAbove(
                                fan_zone_group,
                                "/xyz/openbmc_project/sensors/fan_tach/fan1",
                                static_cast<size_t>(3),
                                static_cast<int64_t>(8000ll)
                            )
                        )),
                        make_condition(propertyStart<int64_t>(
                            "/xyz/openbmc_project/sensors/fan_tach/fan2",
                            "xyz.openbmc_project.Sensor.Value",
                            "Value",
                            condition::countAtOrAbove(
                                fan_zone_group,
                                "/xyz/openbmc_project/sensors/fan_tach/fan2",
                                static_cast<size_t>(3),
                                static_cast<int64_t>(8000ll)
                            )
                        )),
                        make_condition(propertyStart<int64_t>(
                            "/xyz/openbmc_project/sensors/fan_tach/fan3",
                            "xyz.openbmc_project.Sensor.Value",
                            "Value",
                            condition::countAtOrAbove(
                                fan_zone_group,
                                "/xyz/openbmc_project/sensors/fan_tach/fan3",
                                static_cast<size_t>(3),
                                static_cast<int64_t>(8000ll)
                            )
                        )),
                    }
                )
            ),
            std::make_shared<SignalEvent>(
                "interface='org.freedesktop.DBus.Properties',"
                "member='PropertiesChanged',"
                "type='signal',"
                "path='/xyz/openbmc_project/sensors/fan_tach/fan0'",
                std::vector<Condition>(
                    { // Example vector of SignalEvent conditions
                        make_condition(propertySignal<int64_t>(
                            "xyz.openbmc_project.Sensor.Value",
                            "Value",
                            condition::countAtOrAbove(
                                fan_zone_group,
                                "/xyz/openbmc_project/sensors/fan_tach/fan0",
                                static_cast<size_t>(3),
                                static_cast<int64_t>(8000ll)
                            )
                        ))
                    }
                )
            ),
            std::make_shared<SignalEvent>(
                "interface='org.freedesktop.DBus.Properties',"
                "member='PropertiesChanged',"
                "type='signal',"
                "path='/xyz/openbmc_project/sensors/fan_tach/fan1'",
                std::vector<Condition>(
                    { // Example vector of SignalEvent conditions
                        make_condition(propertySignal<int64_t>(
                            "xyz.openbmc_project.Sensor.Value",
                            "Value",
                            condition::countAtOrAbove(
                                fan_zone_group,
                                "/xyz/openbmc_project/sensors/fan_tach/fan1",
                                static_cast<size_t>(3),
                                static_cast<int64_t>(8000ll)
                            )
                        ))
                    }
                )
            ),
            std::make_shared<SignalEvent>(
                "interface='org.freedesktop.DBus.Properties',"
                "member='PropertiesChanged',"
                "type='signal',"
                "path='/xyz/openbmc_project/sensors/fan_tach/fan2'",
                std::vector<Condition>(
                    { // Example vector of SignalEvent conditions
                        make_condition(propertySignal<int64_t>(
                            "xyz.openbmc_project.Sensor.Value",
                            "Value",
                            condition::countAtOrAbove(
                                fan_zone_group,
                                "/xyz/openbmc_project/sensors/fan_tach/fan2",
                                static_cast<size_t>(3),
                                static_cast<int64_t>(8000ll)
                            )
                        ))
                    }
                )
            ),
            std::make_shared<SignalEvent>(
                "interface='org.freedesktop.DBus.Properties',"
                "member='PropertiesChanged',"
                "type='signal',"
                "path='/xyz/openbmc_project/sensors/fan_tach/fan3'",
                std::vector<Condition>(
                    { // Example vector of SignalEvent conditions
                        make_condition(propertySignal<int64_t>(
                            "xyz.openbmc_project.Sensor.Value",
                            "Value",
                            condition::countAtOrAbove(
                                fan_zone_group,
                                "/xyz/openbmc_project/sensors/fan_tach/fan3",
                                static_cast<size_t>(3),
                                static_cast<int64_t>(8000ll)
                            )
                        ))
                    }
                )
            )
        }),
        std::vector<Action>(
        {
            make_action(
                action::log_error(
                    "ERROR: Number of fans at or above 8000rpms reached"
                )
            )
        })
    )}
};

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
