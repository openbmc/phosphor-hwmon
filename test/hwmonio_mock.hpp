#pragma once

#include <chrono>
#include <string>

#include "gmock/gmock.h"

#include "hwmonio.hpp"


namespace hwmonio {

class HwmonIoMock : public HwmonIoInterface {
  public:
    virtual ~HwmonIoMock() {};

    MOCK_CONST_METHOD5(read,
                       int64_t(const std::string&,
                               const std::string&,
                               const std::string&,
                               size_t,
                               std::chrono::milliseconds));
    MOCK_CONST_METHOD6(write,
                       void(uint32_t,
                            const std::string&,
                            const std::string&,
                            const std::string&,
                            size_t,
                            std::chrono::milliseconds));
    MOCK_CONST_METHOD0(path, std::string());
};

} // namespace hwmonio
