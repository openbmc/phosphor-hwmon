#include "sensor.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(AddSensorTest, InvalidType) {

  sensor::Attributes attrs;
  EXPECT_FALSE(sensor::getAttributes("invalid", attrs));

}

