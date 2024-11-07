## HTTP API

All examples below are run from the terminal and use the [httpie](https://httpie.io/docs/cli) CLI tool to make HTTP requests.

**Receive telemetry data**

```bash
http "bonsai-firmware.local/telemetry"
```

```json
{
    "BME280": {
        "humidity": 50.48,
        "pressure": 1017.42,
        "temperature": 20.15
    },
    "SHT41": {
        "humidity": 58.66,
        "temperature": 20.71
    },
    "c_sys_lifetime": 3584745,
    "c_sys_uptime": 671404,
    "outside_temp": 21,
    "soil_capacitive": {
        "curr_status": "Wet",
        "curr_status_dur": 300,
        "moisture": 64,
        "prev_status": "Saturated",
        "prev_status_dur": 300,
        "raw": 347,
        "status_len": 75,
        "status_pos": 32,
        "voltage": 1267,
        "write_count": 1146
    },
    "soil_ldr": {
        "lightness": 53,
        "raw": 528,
        "voltage": 1854
    },
    "soil_temp": 21.25,
    "system_memory_heap": 185776,
    "system_memory_heap_internal": 185688,
    "system_memory_heap_min": 171840,
    "system_reset_reason": "RST_POWERON"
}
```

**Receive registration data**

```bash
http "bonsai-firmware.local/registration"
```

```json
{
    "fw_name": "bonsai-growlab",
    "fw_version": "0.0.1",
    "network_ip": "192.168.1.144",
    "network_rssi": -46,
    "network_signal_strength": "excellent",
    "network_ssid": "SSID",
    "version_esp_idf": "v5.2.1"
}
```

**Receive system report**

```bash
http "bonsai-firmware.local/system/report"
```

```json
{
    "tasks": [
        {
            "core_id": 2147483647,
            "name": "httpd",
            "number": 11,
            "priority": 5,
            "runtime_abs": 228479,
            "runtime_rel": 0.01,
            "stack_free": 1432,
            "state": "Running"
        },
        {
            "core_id": 2147483647,
            "name": "tiT",
            "number": 8,
            "priority": 18,
            "runtime_abs": 540341679,
            "runtime_rel": 17.25,
            "stack_free": 2036,
            "state": "Blocked"
        },
        {
            "core_id": 0,
            "name": "main",
            "number": 4,
            "priority": 1,
            "runtime_abs": 3453028217,
            "runtime_rel": 110.23,
            "stack_free": 1772,
            "state": "Blocked"
        }
    ]
}
```

**Reboot system**

http "bonsai-firmware.local/system/reboot"

```txt
Rebooting...
```

#### DS18B20 Sensor

[DS18B20](https://www.analog.com/media/en/technical-documentation/data-sheets/DS18B20.pdf) is a highly accurate 1-Wire digital thermometer provided by the Analog Devices company. Many DS18B20 sensors can be connected to the single GPIO.

DS18B20 sensor has the following APIs:

```bash
http "bonsai-firmware.local/sensor/ds18b20/scan?gpio=<GPIO_NUM>"
```

- `<GPIO_NUM>` - GPIO number to which sensor is connected.

```bash
http "bonsai-firmware.local/sensor/ds18b20/read_configuration?gpio=<GPIO_NUM>&sensor_id=<SENSOR_ID>"
```

- `<GPIO_NUM>` - GPIO number to which sensor is connected.
- `<SENSOR_ID>` - unique sensor identifier, used internally by firmware, see the description below.

```bash
http "bonsai-firmware.local/sensor/ds18b20/write_configuration?gpio=<GPIO_NUM>&sensor_id<SENSOR_ID>=&seria
l_number=<SERIAL_NUMBER>&resolution=<RESOLUTION>"
```

- `<GPIO_NUM>` - GPIO number to which sensor is connected
- `<SENSOR_ID>` - unique sensor identifier, defined internally by firmware
- `<SERIAL_NUMBER>` can be obtained with the `scan` API, mentioned previously
- `<RESOLUTION>` - how precise the temperature would be measured

`<RESOLUTION>` values:
- 9, single digit precision, 123.1,
- 10, two digit precision, 123.23
- 11, three digit precision, 123.335
- 12, four digit precision, 123.2034

The more accurate the measurement, the more time it takes, see the table below for more details:

Resolution | Max Measurement Time  |
---------- | --------------------- |
12         | 750.0ms (tconv)       |
11         | 375.0ms (tconv / 2)   |
10         | 187.5ms (tconv / 4)   |
9          | 93.75ms (tconv / 8)   |

By default all sensors aren't configured. Let's connect all the sensors to the same GPIO (27) and see if the firmware can detect them:

```bash
http "bonsai-firmware.local/sensor/ds18b20/scan?gpio=27"
```

```json
{
    "rom_codes": [
        "1C:AB:87:00:00:00",
        "85:BB:87:00:00:00"
    ],
    "sensors": [
        {
            "configured": false,
            "id": "soil_temp"
        },
        {
            "configured": false,
            "id": "outside_temp"
        }
    ]
}
```

As we can see, none of the sensors are configured. Let's now configure them all so that we can read the the temperature values from them:

```bash
http "bonsai-firmware.local/sensor/ds18b20/write_configuration?gpio=27&sensor_id=outside_temp&serial_number=1C:AB:87:00:00:00&resolution=10"
```

```json
{
    "resolution": "Bit_10",
    "serial_number": "1C:AB:87:00:00:00"
}
```

```bash
http "bonsai-firmware.local/sensor/ds18b20/write_configuration?gpio=27&sensor_id=soil_temp&serial_number=85:BB:87:00:00:00&resolution=10"
```

```json
{
    "resolution": "Bit_10",
    "serial_number": "85:BB:87:00:00:00"
}
```

Now we can ensure that all sensors are configured:

```bash
http "bonsai-firmware.local/sensor/ds18b20/scan?gpio=27"
```

```json
{
    "rom_codes": [
        "1C:AB:87:00:00:00",
        "85:BB:87:00:00:00"
    ],
    "sensors": [
        {
            "configured": true,
            "id": "soil_temp"
        },
        {
            "configured": true,
            "id": "outside_temp"
        }
    ]
}
```

We can also read configuration for each sensor individually:

```bash
http "bonsai-firmware.local/sensor/ds18b20/read_configuration?gpio=27&sensor_id=outside_temp"
```

```json
{
    "resolution": "Bit_10",
    "serial_number": "1C:AB:87:00:00:00"
}
```

```bash
http "bonsai-firmware.local/sensor/ds18b20/read_configuration?gpio=27&sensor_id=soil_temp"
```

```json
{
    "resolution": "Bit_10",
    "serial_number": "85:BB:87:00:00:00"
}
```

If one of the sensors has broken, or you've decided for some reason to change the configuration of the sensors, you can erase the configuration for each sensor individually and then reconfigure it. Let's erase the configuration for the "outside_temp" sensor:

```bash
http "bonsai-firmware.local/sensor/ds18b20/erase_configuration?gpio=27&sensor_id=outside_temp"
```

```json
{
    "resolution": "Bit_10",
    "serial_number": "1C:AB:87:00:00:00"
}
```

And now read its configuration:

```bash
http "bonsai-firmware.local/sensor/ds18b20/read_configuration?gpio=27&sensor_id=outside_temp"
```

```json
{
    "resolution": "<none>",
    "serial_number": "00:00:00:00:00:00"
}
```

Also lets do the scan to ensure the sensor isn't configured:

```bash
http "bonsai-firmware.local/sensor/ds18b20/scan?gpio=27"
```

```json
{
    "rom_codes": [
        "1C:AB:87:00:00:00",
        "85:BB:87:00:00:00"
    ],
    "sensors": [
        {
            "configured": true,
            "id": "soil_temp"
        },
        {
            "configured": false,
            "id": "outside_temp"
        }
    ]
}
```

Once the sensors have been serviced, they can be reconfigured and continue to operate without problems. The configuration is persisted on the flash.

Please note that the DS18B20 uses the 1-Wire protocol which is very time sensitive. If communication with the sensor is interrupted in any way, the API call may fail. In this case simply retry the HTTP request. The firmware tries to minimise the influence of other firmware components (WiFi, mDNS) on the 1-wire protocol to ensure it is as stable as possible.

## Firmware Configuration Options

- BONSAI_FIRMWARE_HTTP_TELEMETRY_BUFFER_SIZE
- BONSAI_FIRMWARE_HTTP_REGISTRATION_BUFFER_SIZE
