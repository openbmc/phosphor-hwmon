#pragma once

#include "data_types.hpp"

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

class Monitor;

/**
 * @brief Create a condition function object
 *
 * @param[in] condition - The condition being created
 *
 * @return - The created condition funtion object
 */
template <typename T>
auto make_condition(T&& condition)
{
    return Condition(std::forward<T>(condition));
}

/**
 * @brief Create an action function object
 *
 * @param[in] action - The action being created
 *
 * @return - The created action funtion object
 */
template <typename T>
auto make_action(T&& action)
{
    return Action(std::forward<T>(action));
}

/**
 * @struct Property Changed Condtion
 * @brief A match filter functor to test Dbus property value changed signals
 *
 * @tparam T - The type of the property value
 * @tparam U - The type of the condition
 */
template <typename T, typename U>
struct PropertyChangedCondition
{
    PropertyChangedCondition() = delete;
    ~PropertyChangedCondition() = default;
    PropertyChangedCondition(const PropertyChangedCondition&) = default;
    PropertyChangedCondition& operator=(const PropertyChangedCondition&) = default;
    PropertyChangedCondition(PropertyChangedCondition&&) = default;
    PropertyChangedCondition& operator=(PropertyChangedCondition&&) = default;
    PropertyChangedCondition(const char* iface, const char* property,
                             U&& condition) :
        _iface(iface),
        _property(property),
        _condition(std::forward<U>(condition)) { }

    /** @brief Test a property value.
     *
     * Extract the property from the PropertiesChanged
     * message and run the condition test.
     */
    bool operator()(
        sdbusplus::bus::bus&,
        sdbusplus::message::message& msg,
        Monitor&) const
    {
        std::map<std::string, sdbusplus::message::variant<T>> properties;
        const char* iface = nullptr;

        msg.read(iface);
        if (!iface || strcmp(iface, _iface))
        {
            return false;
        }

        msg.read(properties);
        auto it = properties.find(_property);
        if (it == properties.cend())
        {
            return false;
        }

        return _condition(
                   std::forward<T>(it->second.template get<T>()));
    }

private:
    const char* _iface;
    const char* _property;
    U _condition;
};

/**
 * @struct Property Condition Base
 * @brief A match filter functor to test property values
 * @details The base property condition struct that retrieves the property value
 * for a property condition
 */
struct PropertyConditionBase
{
    PropertyConditionBase() = delete;
    virtual ~PropertyConditionBase() = default;
    PropertyConditionBase(const PropertyConditionBase&) = default;
    PropertyConditionBase& operator=(const PropertyConditionBase&) = default;
    PropertyConditionBase(PropertyConditionBase&&) = default;
    PropertyConditionBase& operator=(PropertyConditionBase&&) = default;

    /** @brief Constructor
     *
     *  The service argument can be nullptr.  If something
     *  else is provided the function will call the the
     *  service directly.  If omitted, the function will
     *  look up the service in the ObjectMapper.
     *
     *  @param path - The path of the object containing
     *     the property to be tested.
     *  @param iface - The interface hosting the property
     *     to be tested.
     *  @param property - The property to be tested.
     *  @param service - The DBus service hosting the object.
     */
    PropertyConditionBase(
        const char* path,
        const char* iface,
        const char* property,
        const char* service) :
        _path(path ? path : std::string()),
        _iface(iface),
        _property(property),
        _service(service) {}

    /** @brief Forward comparison to type specific implementation. */
    virtual bool eval(sdbusplus::message::message&) const = 0;

    /** @brief Test a property value.
     *
     * Make a DBus call and test the value of any property.
     */
    bool operator()(
        sdbusplus::bus::bus&,
        sdbusplus::message::message&,
        Monitor&) const;

    /** @brief Test a property value.
     *
     * Make a DBus call and test the value of any property.
     */
    bool operator()(
        const std::string&,
        sdbusplus::bus::bus&,
        Monitor&) const;

private:
    std::string _path;
    std::string _iface;
    std::string _property;
    const char* _service;
};

/**
 * @struct Property Condtion
 * @brief A match filter functor to test property values
 *
 * @tparam T - The type of the property value
 * @tparam U - The type of the condition
 */
template <typename T, typename U>
struct PropertyCondition final : public PropertyConditionBase
{
    PropertyCondition() = delete;
    ~PropertyCondition() = default;
    PropertyCondition(const PropertyCondition&) = default;
    PropertyCondition& operator=(const PropertyCondition&) = default;
    PropertyCondition(PropertyCondition&&) = default;
    PropertyCondition& operator=(PropertyCondition&&) = default;

    /** @brief Constructor
     *
     *  The service argument can be nullptr.  If something
     *  else is provided the function will call the the
     *  service directly.  If omitted, the function will
     *  look up the service in the ObjectMapper.
     *
     *  @param path - The path of the object containing
     *     the property to be tested.
     *  @param iface - The interface hosting the property
     *     to be tested.
     *  @param property - The property to be tested.
     *  @param condition - The test to run on the property.
     *  @param service - The DBus service hosting the object.
     */
    PropertyCondition(
        const char* path,
        const char* iface,
        const char* property,
        U&& condition,
        const char* service) :
        PropertyConditionBase(path, iface, property, service),
        _condition(std::forward<decltype(condition)>(condition)) {}

    /** @brief Test a property value.
     *
     * Make a DBus call and test the value of any property.
     */
    bool eval(sdbusplus::message::message& msg) const override
    {
        sdbusplus::message::variant<T> value;
        msg.read(value);
        return _condition(std::forward<T>(value.template get<T>()));
    }

private:
    U _condition;
};

/**
 * @brief Used to process a Dbus property changed signal event
 *
 * @param[in] iface - Sensor value interface
 * @param[in] property - Sensor value property
 * @param[in] condition - Condition function to perform
 *
 * @tparam T - The type of the property
 * @tparam U - The type of the condition
 */
template <typename T, typename U>
auto propertySignal(const char* iface,
                    const char* property,
                    U&& condition)
{
    return PropertyChangedCondition<T, U>(iface,
                                          property,
                                          std::move(condition));
}

/**
 * @brief Used to process condtions on a start event
 *
 * @param[in] path - Sensor's Dbus path
 * @param[in] iface - Sensor value interface
 * @param[in] property - Sensor value property
 * @param[in] condition - Condition function to perform
 * @param[in] service - Service to lookup sensor Dbus object
 *
 * @tparam T - The type of the property
 * @tparam U - The type of the condition
 */
template <typename T, typename U>
auto propertyStart(const char* path,
                   const char* iface,
                   const char* property,
                   U&& condition,
                   const char* service = nullptr)
{
    return PropertyCondition<T, U>(path,
                                   iface,
                                   property,
                                   std::move(condition),
                                   service);
}

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
