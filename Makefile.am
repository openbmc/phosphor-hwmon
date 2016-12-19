sbin_PROGRAMS = phosphor-hwmon-readd

phosphor_hwmon_readd_SOURCES = readd.cpp
phosphor_hwmon_readd_LDADD = libhwmon.la

noinst_LTLIBRARIES = libhwmon.la
libhwmon_la_SOURCES = \
	argument.cpp \
	directory.cpp \
	sensorset.cpp \
	mainloop.cpp
