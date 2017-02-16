sbin_PROGRAMS = phosphor-hwmon-readd

phosphor_hwmon_readd_SOURCES = readd.cpp
phosphor_hwmon_readd_LDADD = libhwmon.la
phosphor_hwmon_readd_LDFLAGS = $(SDBUSPLUS_LIBS)
phosphor_hwmon_readd_CFLAGS = $(SDBUSPLUS_CFLAGS)

noinst_LTLIBRARIES = libhwmon.la
libhwmon_la_LDFLAGS = $(SDBUSPLUS_LIBS) $(PHOSPHOR_DBUS_INTERFACES_LIBS)
libhwmon_la_CFLAGS = $(SDBUSPLUS_CFLAGS) $(PHOSPHOR_DBUS_INTERFACES_CFLAGS)

libhwmon_la_SOURCES = \
	argument.cpp \
	directory.cpp \
	sensorset.cpp \
	mainloop.cpp \
	sysfs.cpp \
	env.cpp

SUBDIRS = . test
