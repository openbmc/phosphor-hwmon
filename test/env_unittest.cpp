#include "env.hpp"
#include "env_mock.hpp"
#include "util.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrictMock;

TEST(EnvTest, EmptyEnv)
{
    EXPECT_FALSE(
        phosphor::utility::isAverageEnvSet(std::make_pair("power", "1")));
}

TEST(EnvTest, ValidAverageEnv)
{
    StrictMock<EnvMock> eMock;
    envIntf = &eMock;

    std::string power = "power";
    std::string one = "1";
    std::string two = "2";

    EXPECT_CALL(eMock, getEnv(StrEq("AVERAGE"), power, one))
        .WillOnce(Return("true"));
    EXPECT_CALL(eMock, getEnv(StrEq("AVERAGE"), power, two))
        .WillOnce(Return("bar"));

    EXPECT_TRUE(phosphor::utility::isAverageEnvSet(std::make_pair(power, one)));
    EXPECT_FALSE(
        phosphor::utility::isAverageEnvSet(std::make_pair(power, two)));
}
