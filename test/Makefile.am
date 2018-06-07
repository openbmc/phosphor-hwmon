AM_CPPFLAGS = -I$(top_srcdir)/ $(GTEST_CFLAGS) $(GMOCK_CFLAGS)
AM_CXXFLAGS = $(PTHREAD_CFLAGS) \
	$(GTEST_CFLAGS) \
	$(GMOCK_CFLAGS) \
	$(SDBUSPLUS_CFLAGS) \
	$(PHOSPHOR_DBUS_INTERFACES_CFLAGS)
AM_LDFLAGS = $(GTEST_LIBS) \
	$(PTHREAD_LIBS) \
	$(GMOCK_LIBS) \
	$(OESDK_TESTCASE_FLAGS) \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_DBUS_INTERFACES_LIBS)

# Run all 'check' test programs
check_PROGRAMS = hwmon_unittest fanpwm_unittest
TESTS = $(check_PROGRAMS)

hwmon_unittest_SOURCES = hwmon_unittest.cpp
hwmon_unittest_LDADD = $(top_builddir)/hwmon.o

fanpwm_unittest_SOURCES = fanpwm_unittest.cpp
fanpwm_unittest_LDADD = $(PHOSPHOR_LOGGING_LIBS) $(top_builddir)/fan_pwm.o
