check_PROGRAMS =
noinst_PROGRAMS = phosphor-hwmon-test

phosphor_hwmon_test_SOURCES = \
	test.cpp
phosphor_hwmon_test_LDFLAGS = $(SYSTEMD_LIBS) $(PTHREAD_CFLAGS)
phosphor_hwmon_test_CFLAGS = $(SYSTEMD_CFLAGS)
phosphor_hwmon_test_LDADD = ${top_builddir}/libhwmon.la
