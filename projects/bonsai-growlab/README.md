## Bonsai GrowLab

bonsai-growlab is a highly configurable R&D firmware for various experiments and research on plant growth. Please note that some features may not be available on some platforms.

## Platforms

### ESP32

**Features**

- Soil status [monitoring](../../control-components/docs/soil_monitor.md)
- System status [monitoring](../../control-components/docs/system_monitor.md)
- Graceful rebooting process
- Builtin [HTTP server](../../control-components/docs/httpserver.md) for continuous monitoring of many environmental parameters (humidity, temperature, soil moisture, light)
- [mDNS](../../control-components/docs/mdns.md) to simplify application network discovery

**Supported Sensors**

- Soil analog relay [sensor](../../control-components/docs/sensors/soil_analog_relay.md):
    - YL-69 Soil Moisture Sensor
- Soil analog [sensor](../../control-components/docs/sensors/soil_analog.md):
    - Capacitive Soil Moisture Sensor V1.2
- DS18B20 temperature [sensor](../../control-components/docs/sensors/ds18b20.md)
- SHT41 [sensor](../../control-components/docs/sensors/sht41.md)
- BME280 [sensor](../../control-components/docs/sensors/bme280.md)
- LDR [sensor](../../control-components/docs/sensors/ldr.md)

**Installation Instructions**

- [ESP32](../../docs/install/esp32.md)
