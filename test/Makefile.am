@CODE_COVERAGE_RULES@

AM_CPPFLAGS = \
	-I$(top_srcdir)/ \
	$(GTEST_CFLAGS) \
	$(GMOCK_CFLAGS) \
	$(GTEST_MAIN_CFLAGS) \
	$(SDBUSPLUS_CFLAGS) \
	$(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
	$(CODE_COVERAGE_CPPFLAGS)
AM_CFLAGS = \
	$(CODE_COVERAGE_CFLAGS)
AM_CXXFLAGS = \
	$(CODE_COVERAGE_CXXFLAGS)
AM_LDFLAGS = \
	$(GTEST_LIBS) \
	$(GMOCK_LIBS) \
	-lgmock_main \
	$(OESDK_TESTCASE_FLAGS) \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_DBUS_INTERFACES_LIBS) \
	$(CODE_COVERAGE_LIBS)

# Run all 'check' test programs
check_PROGRAMS = hwmon_unittest fanpwm_unittest sensor_unittest
TESTS = $(check_PROGRAMS)

hwmon_unittest_SOURCES = hwmon_unittest.cpp
hwmon_unittest_LDADD = $(top_builddir)/hwmon.o

fanpwm_unittest_SOURCES = fanpwm_unittest.cpp
fanpwm_unittest_LDADD = $(PHOSPHOR_LOGGING_LIBS) $(top_builddir)/fan_pwm.o

sensor_unittest_SOURCES = sensor_unittest.cpp
sensor_unittest_CXXFLAGS = \
	$(PHOSPHOR_LOGGING_CFLAGS) \
	$(GPIOPLUS_CFLAGS)
sensor_unittest_LDADD = \
	-lstdc++fs \
	$(PHOSPHOR_LOGGING_LIBS) \
	$(GPIOPLUS_LIBS) \
	$(top_builddir)/sensor.o \
	$(top_builddir)/hwmon.o \
	env.o \
	gpio.o
