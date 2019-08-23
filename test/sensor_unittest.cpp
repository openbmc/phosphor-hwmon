#include "env_mock.hpp"
#include "gpio_mock.hpp"
#include "hwmonio_mock.hpp"
#include "sensor.hpp"

#include <gpioplus/test/handle.hpp>
#include <memory>
#include <utility>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace env
{

// Delegate all calls to getEnv() to the mock
std::string EnvImpl::get(const char* key) const
{
    return mockEnv.get(key);
}

EnvImpl env_impl;

} // namespace env

using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Pair;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrictMock;

class SensorTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        gpioIntf = nullptr;
        /* Always calls GPIOCHIP and GPIO checks, returning empty string. */
        EXPECT_CALL(env::mockEnv, get(StrEq("GPIOCHIP_temp5")))
            .Times(AnyNumber());
        EXPECT_CALL(env::mockEnv, get(StrEq("GPIO_temp5"))).Times(AnyNumber());
        EXPECT_CALL(env::mockEnv, get(StrEq("GAIN_temp5"))).Times(AnyNumber());
        EXPECT_CALL(env::mockEnv, get(StrEq("OFFSET_temp5")))
            .Times(AnyNumber());
        EXPECT_CALL(env::mockEnv, get(StrEq("REMOVERCS_temp5")))
            .Times(AnyNumber());
    }

    std::string temp = "temp";
    std::string five = "5";
};

TEST_F(SensorTest, BasicConstructorTest)
{
    /* Constructor test with nothing in an rcList or GPIO chip. */
    std::string emptyEnv;
    env::mockEnv.setFakeEnv(emptyEnv);

    auto sensorKey = std::make_pair(temp, five);
    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();
    std::string path = "/";

    auto sensor =
        std::make_unique<sensor::Sensor>(sensorKey, hwmonio_mock.get(), path);
    EXPECT_FALSE(sensor == nullptr);
}

TEST_F(SensorTest, SensorRequiresGpio)
{
    /* Constructor test with only the GPIO chip set. */

    StrictMock<GpioHandleMock> gMock;
    gpioIntf = &gMock;

    std::string gpioEnv = R"(
GPIOCHIP_temp5=chipA
GPIO_temp5=5
)";
    env::mockEnv.setFakeEnv(gpioEnv);

    /* The following piece of code can probably be copied above once it's
     * working.
     */
    auto handleMock = std::make_unique<gpioplus::test::HandleMock>();

    auto sensorKey = std::make_pair(temp, five);
    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();
    std::string path = "/";

    EXPECT_CALL(gMock, build(StrEq("chipA"), StrEq("5")))
        .WillOnce(Invoke([&](const std::string& chip, const std::string& line) {
            return std::move(handleMock);
        }));

    auto sensor =
        std::make_unique<sensor::Sensor>(sensorKey, hwmonio_mock.get(), path);
    EXPECT_FALSE(sensor == nullptr);
}

TEST_F(SensorTest, SensorHasGainAndOffsetAdjustValue)
{
    /* Construct a sensor that has a gain and offset, then verify they are used
     * when adjusting the value.
     */

    std::string gainOffsetEnv = R"(
GAIN_temp5=10
OFFSET_temp5=15
)";
    env::mockEnv.setFakeEnv(gainOffsetEnv);

    auto sensorKey = std::make_pair(temp, five);
    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();
    std::string path = "/";

    auto sensor =
        std::make_unique<sensor::Sensor>(sensorKey, hwmonio_mock.get(), path);
    EXPECT_FALSE(sensor == nullptr);

    double startingValue = 1.0;
    double resultValue = sensor->adjustValue(startingValue);
    EXPECT_DOUBLE_EQ(resultValue, 25.0);
}
