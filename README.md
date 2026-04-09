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

## Configuration File Path Selection

The `start_hwmon.sh` script called from the udev rules file
[70-hwmon.rules](70-hwmon.rules) converts the incoming paths from udev into the
config file path and is controlled by three mutually exclusive meson build
options.

### Default Behavior (No Meson Options)

By default, with no meson options set:

1. If the `OF_FULLNAME` environment variable exists, it is used
2. Otherwise, the full `/sys/devices` path (from `%p`) is used

#### `always-use-devpath`

Forces the use of `/sys/devices` paths for all devices.

**Example:**

- udev `DEVPATH`:
  `/devices/platform/ahb/1e780000.apb/1e780000.apb:bus@1e78a000/1e78a200.i2c/i2c-3/3-0068`
- **Required config:**
  `/etc/default/obmc/hwmon/devices/platform/ahb/1e780000.apb/1e780000.apb--bus@1e78a000/1e78a200.i2c/i2c-3/3-0068.conf`

#### `override-with-devpath`

Selectively uses `/sys/devices` paths for specific device names, while using OF
paths for others.

**Behavior:**

- Uses `OF_FULLNAME` by default when present
- For devices in the override list, uses `/sys/devices` path instead

**Example with override list:** `['power-supply@68']`

- udev `OF_FULLNAME`: `/ahb/apb@1e780000/bus@1e78a000/i2c@200/power-supply@68`
- Device name: `power-supply@68` (in the override list)
- udev `DEVPATH`:
  `/devices/platform/ahb/1e780000.apb/1e780000.apb:bus@1e78a000/1e78a200.i2c/i2c-3/3-0068`
- **Required config:**
  `/etc/default/obmc/hwmon/devices/platform/ahb/1e780000.apb/1e780000.apb--bus@1e78a000/1e78a200.i2c/i2c-3/3-0068.conf`

#### `use-bus-device`

Forces the use of bus device identifiers instead of full device paths or OF
paths.

**Behavior:**

- Extracts the bus device ID from the `/sys/devices` path
- Format: `<bustype>,<device>` (e.g., `i2c,3-0068`)
- Currently supports I2C devices; other bus types fall back to full devpath

**Example:**

- udev `DEVPATH`:
  `/devices/platform/ahb/1e780000.apb/1e780000.apb:bus@1e78a000/1e78a200.i2c/i2c-3/3-0068`
- Extracted bus device: `i2c,3-0068`
- **Required config:** `/etc/default/obmc/hwmon/i2c,3-0068.conf`
