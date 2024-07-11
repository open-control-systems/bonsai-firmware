# Soil Moisture Control

This project is designed to monitor soil moisture levels using an ESP32 board and a soil moisture sensor. The setup includes periodic readings and controlled power management for the sensor to increase its longevity. For more details, see the [article](https://dshil.net/blog/soil_control_system/).

## Installation

**Clone repository**

```
git clone git@github.com:open-control-systems/soil-control-system.git --recursive
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

### Soil Control System Configuration

| Config Option                          | Description                                     | Default Value | Help                                               |
|----------------------------------------|-------------------------------------------------|---------------|----------------------------------------------------|
| `CONFIG_SMC_SENSOR_ADC_CHANNEL`        | Moisture sensor ADC channel                     | 6             | ESP32 ADC channel to read soil moisture values.    |
| `CONFIG_SMC_SENSOR_THRESHOLD`          | Moisture sensor threshold                       | 800           | Threshold to determine if the soil is wet/dry.     |
| `CONFIG_SMC_RELAY_GPIO`                | Relay control GPIO                              | 26            | ESP32 GPIO to control the relay.                   |
| `CONFIG_SMC_READ_INTERVAL`             | Soil moisture read interval (seconds)           | 1800          | Interval for reading soil moisture value.          |
| `CONFIG_SMC_POWER_ON_DELAY_INTERVAL`   | Power on delay interval (seconds)               | 1             | Delay after the control relay is energized.        |

### esp-components Configuration

| Config Option                             | Description                     | Default Value | Help                                            |
|-------------------------------------------|---------------------------------|---------------|-------------------------------------------------|
| `CONFIG_OCS_NETWORK_WIFI_STA_SSID`        | WiFi SSID                       |               | Set the WiFi SSID for network connection.       |
| `CONFIG_OCS_NETWORK_WIFI_STA_PASSWORD`    | WiFi Password                   |               | Set the WiFi password for network connection.   |
| `CONFIG_OCS_NETWORK_WIFI_STA_RETRY_COUNT` | WiFi Retry Count                | 5             | Number of retries for WiFi connection attempts. |
| `CONFIG_OCS_NETWORK_HTTP_SERVER_PORT`     | HTTP Server Port                | 80            | Port for the HTTP server.                       |
| `CONFIG_OCS_NETWORK_MDNS_HOSTNAME`        | mDNS Hostname                   |               | Set the mDNS hostname.                          |
| `CONFIG_OCS_NETWORK_MDNS_INSTANCE_NAME`   | mDNS Hostname                   |               | Set the mDNS instance name.                     |

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
