# Soil Moisture Monitor

This project aims to monitor soil moisture levels using an ESP32 board and a soil moisture sensor. The setup includes periodic readings and controlled power management for the sensor to enhance its longevity.

## Features

- **Periodic Sensor Readings:** Powers the soil moisture sensor every 30 minutes to take readings.
- **Data Logging:** Logs the sensor data for further analysis.
- **Efficient Power Management:** Uses a relay to control the sensor's power, ensuring it is only on when needed.

## Installation

**Clone repository**

```
git clone git@github.com:open-control-systems/soil-moisture-monitor.git --recursive
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

## Usage

**Power On the Device:**

Ensure the ESP32 board is powered on and connected to your computer.

**Monitor the Serial Output:**

```bash
idf.py monitor
```

This will display the moisture readings logged by the sensor.

## License

This project is licensed under the MIT License - see the LICENSE file for details

## Contact

For any questions or feedback, please reach out via GitHub Issues.
