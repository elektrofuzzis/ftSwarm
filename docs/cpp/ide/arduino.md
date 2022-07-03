---
title: Arduino IDE
parent: Choose your IDE
grand_parent: Writing C++ Applications
nav_order: 1
---
## Arduino IDE

### Installation

The easiest way to write some c++ code for your ftSwarm is using the arduino IDE. Please use at least version 1.8.19. 2.x beta versions are compatible as well. 

To install and configure your arduino IDE, please use the following steps: 

1. Download and install the latest version of arduino ide from [arduino.cc](https://www.arduino.cc/en/software).

2. Start arduino ide, select `File/Preferences`. In the preferences dialog, you need to add at `Additional Board Manager URLs` the following URL :
<br>
`https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`


3. Now open `Tools/Boards Manager` and install `esp32 by espressif systems` You need at least version 2.02. Check Boards Manager URL abobe, if only 1.x versions are listed.

4. Our ftSwarm firmware and library has some dependencies. Please install the following libraries using `Tools\Manage Libraries`:
    - `Adafruit GFX Library`, at least version 1.10.12
	- `Adafruit SSD1306`, at least version 2.5.3
    - `FastLED` by Daniel Garcia, at least version 3.4.0
	
5. Install the `ftSwarm` library. Download latest ftswarm.zip from [github](https://github.com/elektrofuzzis/ftSwarm/releases). 
   Use `Sketch\Include Library\Add .ZIP Library` to add the downloaded library.

6. Now you need to set the boards parameters in `Tools`.
- Choose `ESP32 Dev Module` in `Tools\Board\ESP32 Arduino`
- Set PSRAM to `Enabled`
- Set `Core Debug Level: none`.
- Set `Arduino Runs On` to core 0 - this is the default value.
- Set `Events Run On`` to core 1.
- Select serial port of your device in `Tools\Port`.

 
### Writing and uploading code

Arduino code is a sketch and stored as `.ino` files in a folde with the same name.

<style>
img { vertical-align: middle;important! }
</style>

- ![build](../../../assets/img/arduino_compile.png) compiles your software.
- ![upload](../../../assets/img/arduino_upload.png) uploads/flashes your software.
- ![serial](../../../assets/img/arduino_serial.png) opens the serial monitor.

Examples could be accessed using `File/Examples/Examples From Custom Libraries/ftSwarm`

<br>
[Write your first application: motor&switch](../../YourFirstApplication/MotorSwitch){: .btn .float-right }
<br>