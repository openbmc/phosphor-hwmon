average_unittest = executable(
    'average_unittest',
    'average_unittest.cpp',
    dependencies: [
        gmock,
        gtest,
        hwmon_dep,
    ],
)

env_unittest = executable(
    'env_unittest',
    'env_unittest.cpp',
    dependencies: [
        gmock,
        gtest,
        hwmon_dep,
    ],
)

fanpwm_unittest = executable(
    'fanpwm_unittest',
    'fanpwm_unittest.cpp',
    dependencies: [
        gmock,
        gtest,
        hwmon_dep,
    ],
)

hwmon_unittest = executable(
    'hwmon_unittest',
    'hwmon_unittest.cpp',
    dependencies: [
        gmock,
        gtest,
        hwmon_dep,
    ],
)

hwmonio_default_unittest = executable(
    'hwmonio_default_unittest',
    'hwmonio_default_unittest.cpp',
    dependencies: [
        gtest,
        gmock,
        hwmon_dep,
    ],
)

sensor_unittest = executable(
    'sensor_unittest',
    'gpio.cpp',
    'sensor_unittest.cpp',
    dependencies: [
        gtest,
        gmock,
        hwmon_dep,
    ],
)
