sbin_PROGRAMS = phosphor-hwmon-readd

phosphor_hwmon_readd_SOURCES = readd.cpp
phosphor_hwmon_readd_LDADD = $(SDBUSPLUS_LIBS) libhwmon.la
phosphor_hwmon_readd_CXXFLAGS = $(SDBUSPLUS_CFLAGS)

noinst_LTLIBRARIES = libhwmon.la
libhwmon_la_LDFLAGS = -static
libhwmon_la_LIBADD = \
	-lstdc++fs \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_DBUS_INTERFACES_LIBS) \
	$(PHOSPHOR_LOGGING_LIBS)
libhwmon_la_CXXFLAGS = \
	$(SDBUSPLUS_CFLAGS) \
	$(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
	$(PHOSPHOR_LOGGING_CFLAGS)

libhwmon_la_SOURCES = \
	argument.cpp \
	sensorset.cpp \
	mainloop.cpp \
	sysfs.cpp \
	env.cpp \
	fan_speed.cpp \
	fan_pwm.cpp \
	timer.cpp

SUBDIRS = . test msl
