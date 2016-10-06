bin_PROGRAMS = phosphor-hwmon-readd

phosphor_hwmon_readd_SOURCES = readd.cpp
phosphor_hwmon_readd_LDADD = libphosphor-hwmon.la

libphosphor_hwmondir = ${libdir}
libphosphor_hwmon_LTLIBRARIES = libphosphor-hwmon.la
libphosphor_hwmon_la_SOURCES = \
	argument.cpp \
	directory.cpp \
	sensorset.cpp
libphosphor_hwmon_la_LDFLAGS = -version-info 0:0:0 -shared

#SUBDIRS = test
