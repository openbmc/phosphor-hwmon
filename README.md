# phosphor-hwmon

Exposes generic hwmon entries as DBus objects. More information can be found at
[Sensor Architecture](https://github.com/openbmc/docs/blob/master/architecture/sensor-architecture.md)

## To Build

To build this package, do the following steps:

1. meson setup build
2. ninja -C build

To clean the repository run `rm -rf build`.

## D-Bus bus names

To enable the use of Linux features like cgroups prioritization and udev/systemd
control, one instance of phosphor-hwmon is intended to be run per hwmon sysfs
class instance.

This requires an algorithm for selecting a stable, well-known D-Bus busname.

The algorithm is `<PREFIX>-<ID>.Hwmon<N>` where PREFIX is a meson configurable
prefix (`BUSNAME_PREFIX=xyz.openbmc_project` by default), ID is either a
`std::hash` of the `/sys/devices` path backing the hwmon class instance or
provided suffix value from the command line, and N is the implemented
phosphor-hwmon D-Bus API version.
