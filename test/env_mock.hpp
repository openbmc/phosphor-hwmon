#pragma once

#include "env.hpp"

#include <string>

#include <gmock/gmock.h>

namespace env
{

class EnvMock : public Env
{
  public:
    MOCK_CONST_METHOD1(get, std::string(const char*));
};

static inline EnvMock mockEnv;

} // namespace env
