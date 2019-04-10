# Arduino-u-blox-NINA-W10

u-blox NINA-W10 Arduino example + SHT31 temperature and humidity + Bluetooth BLE

[![GitHub version](https://img.shields.io/github/release/ldab/Arduino-u-blox-NINA-W10.svg)](https://github.com/ldab/Arduino-u-blox-NINA-W10/releases/latest)
[![Build Status](https://travis-ci.org/ldab/Arduino-u-blox-NINA-W10.svg?branch=master)](https://travis-ci.org/ldab/Arduino-u-blox-NINA-W10)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/ldab/Arduino-u-blox-NINA-W10/blob/master/LICENSE)

[![GitHub last commit](https://img.shields.io/github/last-commit/ldab/Arduino-u-blox-NINA-W10.svg?style=social)](https://github.com/ldab/Arduino-u-blox-NINA-W10)

[![EVK-NINA-W10](https://www.u-blox.com/sites/default/files/styles/product_full/public/products/NINA-W102_kombi.png)](https://www.u-blox.com/en/product/nina-W10-series)

## How to build PlatformIO based project

1. [Install PlatformIO Core](http://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/platformio/platform-nordicnrf52/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```
# Change directory to example
> cd platform-nordicnrf52/examples/Arduino-u-blox-NINA-W10

# Build project
> platformio run

# Upload firmware
> platformio run --target upload

# Build specific environment
> platformio run -e nina_W10

# Upload firmware for the specific environment
> platformio run -e nina_W10 --target upload

# Clean build files
> platformio run --target clean
```

## Why?

This example tries to implement some key functions and key PIN atributes in order to get you started with using Arduino and the Multiradio board u-blox NINA-W10.

Timer functionas are implemented intead of `delay()` and the PINs have been re-mapped on the `#define` section

## Partition Table and Flash size

* You can create a custom partitions table (CSV) following [ESP32 Partition Tables](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/partition-tables.html) documentation.

* Partitions available at [GitHub](https://github.com/espressif/arduino-esp32/tree/master/tools/partitions)

This examples uses `minimal.csv`

## Erase Flash

`pio run -t erase` all data replaced with 0xFF bytes.


## Bluetooth iOS and Android app 

You can download the sample Bluetooth low energy app - BLE App straight from u-blox wesite: [https://www.u-blox.com/en/product/bluetooth-ios-and-android-app](https://www.u-blox.com/en/product/bluetooth-ios-and-android-app)

![App example](https://raw.githubusercontent.com/ldab/Arduino-u-blox-NINA-W10/master/extras/Screenshot_20190328-130832_u-blox%20BLE.jpg)

## Credits

Github Shields and Badges created with [Shields.io](https://github.com/badges/shields/)

Adafruit [SHT31 Library](https://www.adafruit.com/product/2857)
