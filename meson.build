project(
    'phosphor-hwmon',
    'cpp',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++17'
    ],
    license: 'Apache-2.0',
    version: '1.0',
)

fmt = dependency('fmt')

conf = configuration_data()
conf.set_quoted('BUSNAME_PREFIX', get_option('busname-prefix'))
conf.set_quoted('SENSOR_ROOT', get_option('sensor-root'))
conf.set10('NEGATIVE_ERRNO_ON_FAIL', get_option('negative-errno-on-fail'))
conf.set10('UPDATE_FUNCTIONAL_ON_FAIL', get_option('update-functional-on-fail'))

sysfs_headers = include_directories('.')

sysfs_deps = [
    fmt,
]

sysfs_lib = static_library(
    'sysfs',
    'sysfs.cpp',
    include_directories: sysfs_headers,
    dependencies: sysfs_deps)

sysfs_dep = declare_dependency(
    dependencies: sysfs_deps,
    include_directories: sysfs_headers,
    link_with: sysfs_lib)

hwmon_headers = include_directories('.')

hwmon_deps = [
    fmt,
    dependency('gpioplus'),
    dependency('phosphor-dbus-interfaces'),
    dependency('phosphor-logging'),
    dependency('sdbusplus'),
    dependency('sdeventplus'),
    dependency('stdplus'),
    sysfs_dep,
]

hwmon_lib = static_library(
    'hwmon',
    'average.cpp',
    configure_file(output: 'config.h', configuration: conf),
    'env.cpp',
    'fan_pwm.cpp',
    'fan_speed.cpp',
    'gpio_handle.cpp',
    'hwmon.cpp',
    'hwmonio.cpp',
    'mainloop.cpp',
    'sensor.cpp',
    'sensorset.cpp',
    dependencies: hwmon_deps,
    include_directories: hwmon_headers)

hwmon_dep = declare_dependency(
    dependencies: hwmon_deps,
    include_directories: hwmon_headers,
    link_with: hwmon_lib)

# CLI11 might not have a pkg-config. It is header only so just make
# sure we can access the needed symbols from the header.
cli11_dep = dependency('cli11', required: false)
has_cli11 = meson.get_compiler('cpp').has_header_symbol(
    'CLI/CLI.hpp',
    'CLI::App',
    dependencies: cli11_dep,
    required: false)
if not has_cli11
    cli11_proj = subproject('cli11', required: false)
    assert(cli11_proj.found(), 'CLI11 is required')
    cli11_dep = cli11_proj.get_variable('CLI11_dep')
endif

executable(
    'phosphor-hwmon-readd',
    'readd.cpp',
    dependencies: [
        cli11_dep,
        hwmon_dep,
    ],
    install: true)

subdir('msl')
if not get_option('tests').disabled()
  subdir('test')
endif
subdir('tools')