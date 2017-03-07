check_PROGRAMS =
noinst_PROGRAMS = phosphor-hwmon-test

phosphor_hwmon_test_SOURCES = \
	test.cpp
phosphor_hwmon_test_LDADD = \
	${top_builddir}/libhwmon.la \
	$(SDBUSPLUS_LIBS) \
	$(PTHREAD_LIBS)

phosphor_hwmon_test_CXXFLAGS = \
	$(SDBUSPLUS_CFLAGS) \
	$(PTHREAD_CFLAGS)
