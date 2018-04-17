check_PROGRAMS =
noinst_PROGRAMS = \
	find-callout-path \
	find-hwmon \
	hwmonio \
	phosphor-hwmon-test

find_callout_path_SOURCES = \
	find_callout_path.cpp
find_callout_path_LDFLAGS = \
	-static
find_callout_path_LDADD = \
	-lstdc++fs \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_LOGGING_LIBS) \
	${top_builddir}/sysfs.o

find_callout_path_CXXFLAGS =

find_hwmon_SOURCES = \
	find_hwmon.cpp
find_hwmon_LDFLAGS = \
	-static
find_hwmon_LDADD = \
	-lstdc++fs \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_LOGGING_LIBS) \
	${top_builddir}/sysfs.o

find_hwmon_CXXFLAGS =

hwmonio_SOURCES = \
	hwmonio.cpp
hwmonio_LDFLAGS = \
	-static
hwmonio_LDADD = \
	-lstdc++fs \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_LOGGING_LIBS) \
	${top_builddir}/sysfs.o

hwmonio_CXXFLAGS =

phosphor_hwmon_test_SOURCES = \
	test.cpp
phosphor_hwmon_test_LDADD = \
	${top_builddir}/libhwmon.la \
	$(SDBUSPLUS_LIBS) \
	$(PTHREAD_LIBS)

phosphor_hwmon_test_CXXFLAGS = \
	$(SDBUSPLUS_CFLAGS) \
	$(PTHREAD_CFLAGS)
