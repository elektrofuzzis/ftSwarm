---
title: PlatformIO with VSCode
layout: category
lang: en
classes: wide
sidebar:
    nav: advanced-en
---
### Installation

Download and install the IDE [Visual Studio Code](https://code.visualstudio.com/)

Download and install the latest version of [PlatformIO](https://platformio.org/install/ide?install=vscode).

If you are a VS Code beginner, read the [quick start guide](https://docs.platformio.org/page/ide/vscode.html#quick-start)

Create a new PlatformIO project (use the home button in VS Code's purple status bar)

Board: **Espressif ESP 32 Dev Module**
Framework: **Arduino Framework**

Wait for VSCode setting up your project.

Depended on your ftSwarm Hardware, change the content of `platformio.ini` to the following content:

**ftSwarm & ftSwarmControl:**

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
board_build.mcu = esp32
framework = arduino
lib_deps = 
    bloeckchengrafik/ftSwarm
	fastled/FastLED@^3.5.0
	adafruit/Adafruit GFX Library@^1.11.5
	adafruit/Adafruit SSD1306@^2.5.7
	stm32duino/STM32duino LSM6DSR@^2.1.0
	adafruit/Adafruit BusIO@^1.14.1

build_flags = 
	-DARDUINO_EVENT_RUNNING_CORE=1 
	-DARDUINO_RUNNING_CORE=0
	-DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue
	-DCORE_DEBUG_LEVEL=5

monitor_filters = esp32_exception_decoder
monitor_speed = 115200
upload_speed = 921600
board_build.partitions = no_ota.csv
```

**ftSwarmRS:**

```ini
[env:esp32s3box]
platform = espressif32
board = lolin32
board_build.mcu = esp32s3
framework = arduino
lib_deps = 
    bloeckchengrafik/ftSwarm
	fastled/FastLED@^3.5.0
	adafruit/Adafruit GFX Library@^1.11.5
	adafruit/Adafruit SSD1306@^2.5.7
	stm32duino/STM32duino LSM6DSR@^2.1.0
	adafruit/Adafruit BusIO@^1.14.1

build_flags = 
	-DARDUINO_EVENT_RUNNING_CORE=1 
	-DARDUINO_RUNNING_CORE=0
	-DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue
	-DCORE_DEBUG_LEVEL=5

monitor_filters = esp32_exception_decoder
monitor_speed = 115200
upload_speed = 921600
board_build.partitions = no_ota.csv
```

### Writing Code

Start writing your code. Use the following elements of the blue statusbar:

<style>
img { vertical-align: middle;important! }
</style>

- ![Home](/assets/img/vs_home.png) opens platformIO home. 
- ![build](/assets/img/vs_build.png) builds your software.
- ![upload](/assets/img/vs_upload.png) uploads/flashes your software.
- ![clean](/assets/img/vs_clean.png) cleans up your cached files.
- ![serial](/assets/img/vs_serial.png) opens the serial monitor.
- ![terminal](/assets/img/vs_terminal.png) opens a terminal.
- ![switch](/assets/img/vs_switch.png) switches between your projects.
