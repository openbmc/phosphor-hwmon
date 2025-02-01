gtest = dependency('gtest', main: true, disabler: true, required: false)
gmock = dependency('gmock', disabler: true, required: false)
if not gtest.found() or not gmock.found()
    gtest_opts = import('cmake').subproject_options()
    gtest_opts.add_cmake_defines({'CMAKE_CXX_FLAGS': '-Wno-pedantic'})
    gtest_proj = import('cmake').subproject(
        'googletest',
        options: gtest_opts,
        required: false,
    )
    if gtest_proj.found()
        gtest = declare_dependency(
            dependencies: [
                dependency('threads'),
                gtest_proj.dependency('gtest'),
                gtest_proj.dependency('gtest_main'),
            ],
        )
        gmock = gtest_proj.dependency('gmock')
    else
        assert(not get_option('tests').allowed(), 'Googletest is required')
    endif
endif

tests = [
    'average_unittest',
    'env_unittest',
    'fanpwm_unittest',
    'hwmon_unittest',
    'hwmonio_default_unittest',
    'sensor_unittest',
]

foreach t : tests
    test(
        t,
        executable(
            t.underscorify(),
            t + '.cpp',
            'gpio.cpp',
            implicit_include_directories: false,
            dependencies: [hwmon_dep, gtest, gmock],
        ),
    )
endforeach
