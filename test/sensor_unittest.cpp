#include "env_mock.hpp"
#include "gpio_mock.hpp"
#include "hwmonio_mock.hpp"
#include "sensor.hpp"

#include <gpioplus/test/sys.hpp>
#include <memory>
#include <utility>

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
};

using ::testing::Eq;
using ::testing::Return;
using ::testing::StrictMock;

TEST_F(SensorTest, BasicConstructorTest)
{
    /* Constructor test with nothing in an rcList or GPIO chip. */

    StrictMock<EnvMock> eMock;
    envIntf = &eMock;

    auto sensorKey = std::make_pair(std::string("temp"), std::string("5"));
    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();
    std::string path = "/";

    /* Always calls GPIOCHIP and GPIO checks, returning empty string. */
    EXPECT_CALL(eMock, getEnv(Eq("GPIOCHIP"), Eq(sensorKey)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(Eq("GPIO"), Eq(sensorKey))).WillOnce(Return(""));

    /* Always calls GAIN and OFFSET, can use ON_CALL instead of EXPECT_CALL */
    EXPECT_CALL(eMock, getEnv(Eq("GAIN"), Eq(sensorKey))).WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(Eq("OFFSET"), Eq(sensorKey)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(Eq("REMOVERCS"), Eq(sensorKey)))
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
    uint32_t lineOffset = 5;
    unsigned int id = 0;
    gpioplus::test::SysMock gpioplusSys;
    gpioplus::Chip chip(id, &gpioplusSys);

    std::vector<gpioplus::Handle::Line> lines = {{lineOffset, 0}};
    gpioplus::HandleFlags flags(chip.getLineInfo(lineOffset).flags);
    flags.output = true;
    auto handleMocked = std::make_unique<gpioplus::Handle>(
        chip, lines, flags, "phosphor-hwmon-test");

    auto sensorKey = std::make_pair(std::string("temp"), std::string("5"));
    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();
    std::string path = "/";

    EXPECT_CALL(eMock, getEnv(Eq("GPIOCHIP"), Eq(sensorKey)))
        .WillOnce(Return("chipA"));
    EXPECT_CALL(eMock, getEnv(Eq("GPIO"), Eq(sensorKey))).WillOnce(Return("5"));

    EXPECT_CALL(gMock, build(Eq("chipA"), Eq("5")))
        .WillOnce(Return(std::move(handleMocked)));

    /* Always calls GAIN and OFFSET, can use ON_CALL instead of EXPECT_CALL */
    EXPECT_CALL(eMock, getEnv(Eq("GAIN"), Eq(sensorKey))).WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(Eq("OFFSET"), Eq(sensorKey)))
        .WillOnce(Return(""));
    EXPECT_CALL(eMock, getEnv(Eq("REMOVERCS"), Eq(sensorKey)))
        .WillOnce(Return(""));

    auto sensor =
        std::make_unique<sensor::Sensor>(sensorKey, hwmonio_mock.get(), path);
    EXPECT_FALSE(sensor == nullptr);
}
