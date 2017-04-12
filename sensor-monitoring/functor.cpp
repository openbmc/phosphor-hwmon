#include "functor.hpp"
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>

namespace phosphor
{
namespace sensor
{
namespace monitoring
{

bool PropertyConditionBase::operator()(sdbusplus::bus::bus& bus,
                                       sdbusplus::message::message&,
                                       Monitor& mon) const
{
    std::string path(_path);
    return (*this)(path, bus, mon);
}

bool PropertyConditionBase::operator()(const std::string& path,
                                       sdbusplus::bus::bus& bus,
                                       Monitor&) const
{
    std::string host;

    if (_service)
    {
        host.assign(_service);
    }
    else
    {
        auto mapperCall = bus.new_method_call(
                              "xyz.openbmc_project.ObjectMapper",
                              "/xyz/openbmc_project/object_mapper",
                              "xyz.openbmc_project.ObjectMapper",
                              "GetObject");
        mapperCall.append(path);
        mapperCall.append(std::vector<std::string>({_iface}));
        auto mapperResponseMsg = bus.call(mapperCall);
        if (mapperResponseMsg.is_method_error())
        {
            return false;
        }

        std::map<std::string, std::vector<std::string>> mapperResponse;
        mapperResponseMsg.read(mapperResponse);
        if (mapperResponse.empty())
        {
            return false;
        }

        host = mapperResponse.begin()->first;
    }
    auto hostCall = bus.new_method_call(host.c_str(),
                                        path.c_str(),
                                        "org.freedesktop.DBus.Properties",
                                        "Get");
    hostCall.append(_iface);
    hostCall.append(_property);
    auto hostResponseMsg = bus.call(hostCall);
    if (hostResponseMsg.is_method_error())
    {
        return false;
    }

    return eval(hostResponseMsg);
}

} // namespace monitoring
} // namespace sensor
} // namespace phosphor
