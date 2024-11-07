## Installation

**Install ESP-IDF**

Make sure ESP-IDF is properly installed, see to the official [documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html). Espressif provides a very clear and detailed explanation of each installation step.

**Clone repository**

```
git clone git@github.com:open-control-systems/bonsai-firmware.git --recursive
```

**Configure the Firmware**

```bash
idf.py menuconfig
```

There are many configuration options, see options prefixed with "OCS_" or "BONSAI_FIRMWARE_".

**Build and Flash:**

```bash
idf.py build
idf.py flash
```
