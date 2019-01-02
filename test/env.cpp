#include "env_mock.hpp"

// Set this before each test that hits a call to getEnv().
EnvInterface* envIntf = nullptr;

namespace env
{

std::string getEnv(const char* key)
{
    return (envIntf) ? envIntf->getEnv(key) : "";
}

std::string getEnv(const char* prefix, const SensorSet::key_type& sensor)
{
    return (envIntf) ? envIntf->getEnv(prefix, sensor) : "";
}

std::string getEnv(const char* prefix, const std::string& type,
                   const std::string& id)
{
    return (envIntf) ? envIntf->getEnv(prefix, type, id) : "";
}

std::string getIndirectID(std::string path, const std::string& fileSuffix,
                          const SensorSet::key_type& sensor)
{
    return (envIntf) ? envIntf->getIndirectID(path, fileSuffix, sensor) : "";
}

} // namespace env
