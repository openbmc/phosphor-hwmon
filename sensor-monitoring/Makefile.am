AM_DEFAULT_SOURCE_EXT = .cpp
AM_CPPFLAGS = -I${top_src_dir}

sbin_PROGRAMS = phosphor-sensor-monitor

phosphor_sensor_monitor_SOURCES = \
	main.cpp \
	monitor.cpp \
	functor.cpp \
	generated.cpp
phosphor_sensor_monitor_LDFLAGS = \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_LOGGING_LIBS)
phosphor_sensor_monitor_CXXFLAGS = \
	$(SDBUSPLUS_CFLAGS) \
	$(PHOSPHOR_LOGGING_CFLAGS)

generated.cpp:
	$(AM_V_GEN)$(PYTHON) ${srcdir}/gen-sensor-group-defs.py -y ${srcdir}/example/sensor_monitoring_defs.yaml -o ${builddir}
