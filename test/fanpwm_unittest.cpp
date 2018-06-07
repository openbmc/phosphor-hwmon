#include "fan_pwm.hpp"

#include "hwmonio_mock.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdbusplus/test/sdbus_mock.hpp>
#include <string>

using ::testing::_;
using ::testing::Invoke;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrEq;

TEST(FanPwmTest, BasicConstructorDeferredTest) {
    // Attempt to just instantiate one.

    // NOTE: This test's goal is to figure out what's minimally required to
    // mock to instantiate this object.
    sdbusplus::SdBusMock sdbus_mock;
    auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

    std::string instancePath = "";
    std::string devPath = "";
    std::string id = "";
    std::string objPath = "asdf";
    bool defer = true;
    uint64_t target = 0x01;

    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();

    EXPECT_CALL(sdbus_mock,
                sd_bus_add_object_vtable(
                    IsNull(),
                    NotNull(),
                    StrEq("asdf"),
                    StrEq("xyz.openbmc_project.Control.FanPwm"),
                    NotNull(),
                    NotNull()))
    .WillOnce(Return(0));

    EXPECT_CALL(sdbus_mock,
                sd_bus_emit_properties_changed_strv(
                    IsNull(),
                    StrEq("asdf"),
                    StrEq("xyz.openbmc_project.Control.FanPwm"),
                    NotNull()))
    .WillOnce(Return(0));

    hwmon::FanPwm f(std::move(hwmonio_mock),
                    devPath,
                    id,
                    bus_mock,
                    objPath.c_str(),
                    defer,
                    target);
}

TEST(FanPwmTest, BasicConstructorNotDeferredTest) {
    // Attempt to just instantiate one.

    // NOTE: This test's goal is to figure out what's minimally required to
    // mock to instantiate this object.
    sdbusplus::SdBusMock sdbus_mock;
    auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

    std::string instancePath = "";
    std::string devPath = "";
    std::string id = "";
    std::string objPath = "asdf";
    bool defer = false;
    uint64_t target = 0x01;

    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();

    EXPECT_CALL(sdbus_mock,
                sd_bus_add_object_vtable(
                    IsNull(),
                    NotNull(),
                    StrEq("asdf"),
                    StrEq("xyz.openbmc_project.Control.FanPwm"),
                    NotNull(),
                    NotNull()))
    .WillOnce(Return(0));

    EXPECT_CALL(sdbus_mock,
                sd_bus_emit_properties_changed_strv(
                    IsNull(),
                    StrEq("asdf"),
                    StrEq("xyz.openbmc_project.Control.FanPwm"),
                    NotNull()))
    .WillOnce(Return(0));

    EXPECT_CALL(sdbus_mock,
                sd_bus_emit_object_added(IsNull(), StrEq("asdf")))
    .WillOnce(Return(0));

    EXPECT_CALL(sdbus_mock,
                sd_bus_emit_object_removed(IsNull(), StrEq("asdf")))
    .WillOnce(Return(0));

    hwmon::FanPwm f(std::move(hwmonio_mock),
                    devPath,
                    id,
                    bus_mock,
                    objPath.c_str(),
                    defer,
                    target);
}

TEST(FanPwmTest, WriteTargetValue) {
    // Create a FanPwm and write a value to the object.

    sdbusplus::SdBusMock sdbus_mock;
    auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

    std::string instancePath = "";
    std::string devPath = "devp";
    std::string id = "the_id";
    std::string objPath = "asdf";
    bool defer = true;
    uint64_t target = 0x01;

    std::unique_ptr<hwmonio::HwmonIOInterface> hwmonio_mock =
        std::make_unique<hwmonio::HwmonIOMock>();

    EXPECT_CALL(sdbus_mock,
                sd_bus_add_object_vtable(
                    IsNull(),
                    NotNull(),
                    StrEq("asdf"),
                    StrEq("xyz.openbmc_project.Control.FanPwm"),
                    NotNull(),
                    NotNull()))
    .WillOnce(Return(0));

    EXPECT_CALL(sdbus_mock,
                sd_bus_emit_properties_changed_strv(
                    IsNull(),
                    StrEq("asdf"),
                    StrEq("xyz.openbmc_project.Control.FanPwm"),
                    NotNull()))
    .WillOnce(Return(0));

    hwmonio::HwmonIOMock *hwmonio =
        reinterpret_cast<hwmonio::HwmonIOMock *>(hwmonio_mock.get());

    hwmon::FanPwm f(std::move(hwmonio_mock),
                    devPath,
                    id,
                    bus_mock,
                    objPath.c_str(),
                    defer,
                    target);

    target = 0x64;

    EXPECT_CALL(*hwmonio, write(static_cast<uint32_t>(target),
                                StrEq("pwm"),
                                StrEq("the_id"),
                                _,
                                hwmonio::retries,
                                hwmonio::delay));

    EXPECT_CALL(sdbus_mock,
                sd_bus_emit_properties_changed_strv(
                    IsNull(),
                    StrEq("asdf"),
                    StrEq("xyz.openbmc_project.Control.FanPwm"),
                    NotNull()))
    .WillOnce(
        Invoke([&](sd_bus *bus,
                   const char *path,
                   const char *interface,
                   char **names) {
            EXPECT_EQ(0, strncmp("Target", names[0], 6));
            return 0;
        })
    );

    EXPECT_EQ(target, f.target(target));
}
