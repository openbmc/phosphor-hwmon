AM_CPPFLAGS = -I$(top_srcdir)/ $(GTEST_CFLAGS)
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(GTEST_CFLAGS) $(SDBUSPLUS_CFLAGS) $(PHOSPHOR_DBUS_INTERFACES_CFLAGS)
AM_LDFLAGS = $(GTEST_LIBS) $(PTHREAD_LIBS) -lgtest -lgmock $(OESDK_TESTCASE_FLAGS) $(SDBUSPLUS_LIBS) $(PHOSPHOR_DBUS_INTERFACES_LIBS)

# Run all 'check' test programs
check_PROGRAMS = addsensor_unittest
TESTS = $(check_PROGRAMS)

addsensor_unittest_SOURCES = addsensor_unittest.cpp
addsensor_unittest_LDADD = $(top_builddir)/sensor.o

