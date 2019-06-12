#include "env_mock.hpp"
#include "gpio_mock.hpp"
#include "hwmonio_mock.hpp"
#include "sensor.hpp"

#include <gpioplus/test/handle.hpp>
#include <memory>
#include <sdbusplus/test/sdbus_mock.hpp>
#include <utility>
#include <xyz/openbmc_project/Sensor/Device/error.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

class SensorTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        envIntf = nullptr;
        gpioIntf = nullptr;
    }

    std::string temp = "temp";
    std::string five = "5";
};

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Pair;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrictMock;
using ::testing::Throw;

TEST_F(SensorTest, BasicConstructorTest)
{
    /* Constructor test with nothing in an rcList or GPIO chip. */

    StrictMock<EnvMock> eMock;
    envIntf = &eMock;

    auto sensorKey = std::make_pair(temp, five);
    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();
    std::string path = "/";

    /* Always calls GPIOCHIP and GPIO checks, returning empty string. */
    EXPECT_CALL(eMock, getEnv(StrEq("GPIOCHIP"), Pair(temp, five)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(StrEq("GPIO"), Pair(temp, five)))
        .WillOnce(Return(""));

    /* Always calls GAIN and OFFSET, can use ON_CALL instead of EXPECT_CALL */
    EXPECT_CALL(eMock, getEnv(StrEq("GAIN"), Pair(temp, five)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(StrEq("OFFSET"), Pair(temp, five)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(StrEq("REMOVERCS"), Pair(temp, five)))
        .WillOnce(Return(""));

    auto sensor =
        std::make_unique<sensor::Sensor>(sensorKey, hwmonio_mock.get(), path);
    EXPECT_FALSE(sensor == nullptr);
}

TEST_F(SensorTest, SensorRequiresGpio)
{
    /* Constructor test with only the GPIO chip set. */

    StrictMock<EnvMock> eMock;
    envIntf = &eMock;

    StrictMock<GpioHandleMock> gMock;
    gpioIntf = &gMock;

    /* The following piece of code can probably be copied above once it's
     * working.
     */
    auto handleMock = std::make_unique<gpioplus::test::HandleMock>();

    auto sensorKey = std::make_pair(temp, five);
    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();
    std::string path = "/";

    EXPECT_CALL(eMock, getEnv(StrEq("GPIOCHIP"), Pair(temp, five)))
        .WillOnce(Return("chipA"));
    EXPECT_CALL(eMock, getEnv(StrEq("GPIO"), Pair(temp, five)))
        .WillOnce(Return("5"));

    EXPECT_CALL(gMock, build(StrEq("chipA"), StrEq("5")))
        .WillOnce(Invoke([&](const std::string& chip, const std::string& line) {
            return std::move(handleMock);
        }));

    /* Always calls GAIN and OFFSET, can use ON_CALL instead of EXPECT_CALL */
    EXPECT_CALL(eMock, getEnv(StrEq("GAIN"), Pair(temp, five)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(StrEq("OFFSET"), Pair(temp, five)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(StrEq("REMOVERCS"), Pair(temp, five)))
        .WillOnce(Return(""));

    auto sensor =
        std::make_unique<sensor::Sensor>(sensorKey, hwmonio_mock.get(), path);
    EXPECT_FALSE(sensor == nullptr);
}

TEST_F(SensorTest, SensorAddValueReThrowsReadFailure)
{
    /* When read fails and throws ReadFailure, we expect addValue to rethrow it
     */

    StrictMock<EnvMock> eMock;
    envIntf = &eMock;
    hwmonio::HwmonIOMock hwmonio_mock;

    auto sensorKey = std::make_pair(temp, five);
    std::string path = "/";

    /* Always calls GPIOCHIP and GPIO checks, returning empty string. */
    EXPECT_CALL(eMock, getEnv(StrEq("GPIOCHIP"), Pair(temp, five)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(StrEq("GPIO"), Pair(temp, five)))
        .WillOnce(Return(""));

    EXPECT_CALL(eMock, getEnv(StrEq("GAIN"), Pair(temp, five)))
        .WillOnce(Return("10"));
    EXPECT_CALL(eMock, getEnv(StrEq("OFFSET"), Pair(temp, five)))
        .WillOnce(Return("15"));
    EXPECT_CALL(eMock, getEnv(StrEq("REMOVERCS"), Pair(temp, five)))
        .WillOnce(Return(""));

    auto sensor =
        std::make_unique<sensor::Sensor>(sensorKey, &hwmonio_mock, path);
    EXPECT_FALSE(sensor == nullptr);

    RetryIO retryIO;
    ObjectInfo info;

    EXPECT_CALL(hwmonio_mock, read(_, _, _, _, _))
        .WillOnce(Throw(sdbusplus::xyz::openbmc_project::Sensor::Device::Error::
                            ReadFailure()));
    EXPECT_THROW(
        sensor->addValue(retryIO, info),
        sdbusplus::xyz::openbmc_project::Sensor::Device::Error::ReadFailure);
}
