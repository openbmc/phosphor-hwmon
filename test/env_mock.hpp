#pragma once

#include "sensorset.hpp"

#include <string>

#include <gmock/gmock.h>

class EnvInterface
{
  public:
    virtual ~EnvInterface() = default;

    virtual std::string getEnv(const char* key) const = 0;
    virtual std::string getEnv(const char* prefix,
                               const SensorSet::key_type& sensor) const = 0;
    virtual std::string getEnv(const char* prefix, const std::string& type,
                               const std::string& id) const = 0;
    virtual std::string
        getIndirectID(std::string path, const std::string& fileSuffix,
                      const SensorSet::key_type& sensor) const = 0;
};

class EnvMock : public EnvInterface
{
  public:
    virtual ~EnvMock() = default;

    MOCK_CONST_METHOD1(getEnv, std::string(const char*));
    MOCK_CONST_METHOD2(getEnv,
                       std::string(const char*, const SensorSet::key_type&));
    MOCK_CONST_METHOD3(getEnv, std::string(const char*, const std::string&,
                                           const std::string&));
    MOCK_CONST_METHOD3(getIndirectID,
                       std::string(std::string, const std::string&,
                                   const SensorSet::key_type&));
};

// Set this before each test that hits a call to getEnv().
extern EnvInterface* envIntf;
