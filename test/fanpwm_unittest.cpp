#include "fan_pwm.hpp"

#include "hwmonio_mock.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdbusplus/test/sdbus_mock.hpp>
#include <string>

TEST(FanPwmTest, BasicConstructorTest) {
  // Attempt to just instantiate one.

  // NOTE: This test's goal is to figure out what's minimally required to mock
  // to instantiate this object.
  sdbusplus::SdBusMock sdbus_mock;

  auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);
  std::string instancePath = "";
  std::string devPath = "";
  std::string id = "";
  std::string objPath = "asdf";
  bool defer = false;
  uint64_t target = 0x01;

  std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock = std::make_unique<hwmonio::HwmonIOMock>();

  hwmon::FanPwm f(std::move(hwmonio_mock), devPath, id, bus_mock, objPath.c_str(), defer, target);
}
