# Bonsai Firmware

[![Build Bonsai Firmware](https://github.com/open-control-systems/bonsai-firmware/actions/workflows/esp32.yml/badge.svg)](https://github.com/open-control-systems/bonsai-firmware/actions/workflows/esp32.yml)

# Introduction

This project is designed to monitor soil moisture levels using an ESP32 board and a soil moisture sensor. The setup includes periodic readings and controlled power management for the sensor to increase its longevity. For more details, see the [article](https://dshil.net/blog/soil_control_system/).

## Installation

**Clone repository**

```
git clone git@github.com:open-control-systems/bonsai-firmware.git --recursive
```

**Set Up the Hardware:**

- Connect the soil moisture sensor to the ESP32 board.
- Connect the relay to control the power to the sensor.

**Configure the Software:**

- Ensure you have the ESP-IDF framework installed.
- Configure your `sdkconfig` as per your hardware setup.

**Build and Flash:**

```bash
idf.py build
idf.py flash
```

## Configuration

The firmware can be configured using the following command:

```bash
idf.py menuconfig
```

There are many configuration options, see options prefixed with "OCS_" or "BONSAI_FIRMWARE_".

## Usage

**Power On the Device:**

Ensure the ESP32 board is powered on and connected to your computer.

**Monitor the Serial Output:**

```bash
idf.py monitor
```

This will display the moisture readings logged by the sensor.

## License

This project is licensed under the MPL 2.0 License - see the LICENSE file for details.
