#include "env.hpp"
#include "env_mock.hpp"
#include "util.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::StrEq;

namespace env
{

// Delegate all calls to getEnv() to the mock
std::string EnvImpl::get(const char* key) const
{
    return mockEnv.get(key);
}

EnvImpl env_impl;

} // namespace env

TEST(EnvTest, EmptyEnv)
{
    EXPECT_FALSE(
        phosphor::utility::isAverageEnvSet(std::make_pair("power", "1")));
}

TEST(EnvTest, ValidAverageEnv)
{
    std::string power = "power";
    std::string one = "1";
    std::string two = "2";

    EXPECT_CALL(env::mockEnv, get(StrEq("AVERAGE_power1")))
        .WillOnce(Return("true"));
    EXPECT_CALL(env::mockEnv, get(StrEq("AVERAGE_power2")))
        .WillOnce(Return("bar"));

    EXPECT_TRUE(phosphor::utility::isAverageEnvSet(std::make_pair(power, one)));
    EXPECT_FALSE(
        phosphor::utility::isAverageEnvSet(std::make_pair(power, two)));
}
