# Initialization
AC_PREREQ([2.69])
AC_INIT([phosphor-hwmon], [1.0], [https://github.com/openbmc/phosphor-hwmon/issues])
AC_LANG([C++])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign dist-xz])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CXX
AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES([SDBUSPLUS], [sdbusplus], [], [AC_MSG_ERROR(["sdbusplus required and not found."])])
PKG_CHECK_MODULES([PHOSPHOR_DBUS_INTERFACES], [phosphor-dbus-interfaces], [], [AC_MSG_ERROR(["phosphor-dbus-interfaces required and not found."])])
PKG_CHECK_MODULES([PHOSPHOR_LOGGING], [phosphor-logging], [], [AC_MSG_ERROR(["phosphor-logging required and not found."])])

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX_14([noext])
AX_APPEND_COMPILE_FLAGS([-fpic -Wall -Werror], [CXXFLAGS])

# Checks for header files.
AC_CHECK_HEADER(sdbusplus/server.hpp, ,[AC_MSG_ERROR([Could not find sdbusplus/server.hpp...sdbusplus development package required])])
AC_CHECK_HEADER(experimental/filesystem, ,[AC_MSG_ERROR([Could not find experimental/filesystem...libstdc++fs development package required])])

# Checks for library functions.
LT_INIT

# Check/set gtest specific functions.
AX_PTHREAD([GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=1"],[GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=0"])
AC_SUBST(GTEST_CPPFLAGS)

AC_ARG_ENABLE([oe-sdk],
    AS_HELP_STRING([--enable-oe-sdk], [Link testcases absolutely against OE SDK so they can be ran within it.])
)
AC_ARG_VAR(OECORE_TARGET_SYSROOT,
    [Path to the OE SDK SYSROOT])
AS_IF([test "x$enable_oe_sdk" == "xyes"],
    AS_IF([test "x$OECORE_TARGET_SYSROOT" == "x"],
          AC_MSG_ERROR([OECORE_TARGET_SYSROOT must be set with --enable-oe-sdk])
    )
    AC_MSG_NOTICE([Enabling OE-SDK at $OECORE_TARGET_SYSROOT])
    [
        testcase_flags="-Wl,-rpath,\${OECORE_TARGET_SYSROOT}/lib"
        testcase_flags="${testcase_flags} -Wl,-rpath,\${OECORE_TARGET_SYSROOT}/usr/lib"
        testcase_flags="${testcase_flags} -Wl,-dynamic-linker,`find \${OECORE_TARGET_SYSROOT}/lib/ld-*.so | sort -r -n | head -n1`"
    ]
    AC_SUBST([OESDK_TESTCASE_FLAGS], [$testcase_flags])
)

# When a sensor access fails, just remove the sensor from dbus instead of exiting the daemon.
AC_ARG_ENABLE([remove-from-dbus-on-fail],
    AS_HELP_STRING([--enable-remove-from-dbus-on-fail], [Remove properties from D-Bus on access failures])
)

AC_ARG_VAR(REMOVE_ON_FAIL, [Remove properties from D-Bus on access failures])

AS_IF([test "x$enable_remove_from_dbus_on_fail" == "xyes"],
      [REMOVE_ON_FAIL="yes"]
      AC_DEFINE_UNQUOTED([REMOVE_ON_FAIL], ["$REMOVE_ON_FAIL"], [Remove properties from D-Bus on access failures])
)

# When a sensor read fails, set the Value on dbus with -errno.
# Incompatible with remove-from-dbus-on-fail.
AC_ARG_ENABLE([negative-errno-on-fail],
    AS_HELP_STRING([--enable-negative-errno-on-fail], [Set sensor value to -errno on read failures])
)

AC_ARG_VAR(NEGATIVE_ERRNO_ON_FAIL, [Set sensor value to -errno on read failures])

AS_IF([test "x$enable_negative_errno_on_fail" == "xyes"],
      [NEGATIVE_ERRNO_ON_FAIL="yes"]
      AC_DEFINE_UNQUOTED([NEGATIVE_ERRNO_ON_FAIL], ["$NEGATIVE_ERRNO_ON_FAIL"], [Set sensor value to -errno on read failures])
)

AC_ARG_VAR(BUSNAME_PREFIX, [The DBus busname prefix.])
AC_ARG_VAR(SENSOR_ROOT, [The DBus sensors namespace root.])
AS_IF([test "x$BUSNAME_PREFIX" == "x"], [BUSNAME_PREFIX="xyz.openbmc_project.Hwmon"])
AS_IF([test "x$SENSOR_ROOT" == "x"], [SENSOR_ROOT="/xyz/openbmc_project/sensors"])
AC_DEFINE_UNQUOTED([BUSNAME_PREFIX], ["$BUSNAME_PREFIX"], [The DBus busname prefix.])
AC_DEFINE_UNQUOTED([SENSOR_ROOT], ["$SENSOR_ROOT"], [The DBus sensors namespace root.])

# Create configured output
AC_CONFIG_FILES([Makefile test/Makefile msl/Makefile])
AC_OUTPUT
