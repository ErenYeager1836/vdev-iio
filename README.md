# vdev-iio

A loadable Linux kernel module (`.ko`) in **C** implementing a synthetic Industrial I/O (IIO) 3-axis accelerometer and temperature device driver.

`vdev-iio` serves as a reference implementation for kernel driver development and automated sensor testing, exposing real-time hardware telemetry through standard Linux `/sys/bus/iio/devices/` sysfs attributes without physical hardware.

---

## Features

- **Standard Linux IIO Subsystem Registration**:
  - Allocates and registers an IIO device via `devm_iio_device_alloc()` and `devm_iio_device_register()`.
  - Binds to a synthetic `platform_device` and `platform_driver`.
- **Multi-Channel Sensor Specifications (`iio_chan_spec`)**:
  - `IIO_ACCEL` for X, Y, and Z axes (`IIO_MOD_X`, `IIO_MOD_Y`, `IIO_MOD_Z`).
  - `IIO_TEMP` for ambient temperature.
- **Sysfs ABI Exposure**:
  - `in_accel_x_raw`, `in_accel_y_raw`, `in_accel_z_raw`: 3-axis gravitational acceleration in milli-g ($1g \approx 9806$).
  - `in_accel_scale`: Micro-unit scale multiplier.
  - `in_temp_raw`: Temperature in milli-degrees Celsius ($25^\circ\text{C} \approx 25000$).
- **Concurrency & Resource Safety**:
  - Managed device resources (`devm_*`) preventing kernel resource leaks upon unbind.
  - Mutex-guarded hardware state reads preventing race conditions.

---

## Quick Start

### 1. Build Kernel Module
```bash
make
```

### 2. Load Module into Running Kernel
```bash
sudo insmod vdev_iio.ko
```

### 3. Verify in Dmesg & Sysfs
```bash
dmesg | tail -n 5
# [vdev_iio: registered synthetic 3-axis accelerometer and temperature sensor]

# Find the registered IIO device
cd /sys/bus/iio/devices/iio:device0/

# Read acceleration & temperature sysfs channels
cat in_accel_z_raw
cat in_temp_raw
```

### 4. Unload Module
```bash
sudo rmmod vdev_iio
```

---

## License
GPL-2.0-or-later
