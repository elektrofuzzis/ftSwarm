---
title: Install your IDE 
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---

There are different platforms and IDEs to write your C++ code. If you are a beginner please use the Arduino IDE as described below.

1. [Arduino](https://www.arduino.cc/)
   is the most common platform for DIY projects. It started 2005 as a project to design an open source microcontroller board, 
   today the platform supports all common types of microcontrollers. The Arduino environment is an IDE with integrated framework for embedded systems.

2. [PlattformIO](https://platformio.org)
   is a framework to develop embedded systems, too. This framework is not part of an IDE like Arduino and you could choose among different editors/IDEs.
   In the first time, PlatformIO is a bit more complicated than Arduino IDE, but it has many advantages. See [Visual Studio Code](../vscode) for more information.

3. [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) is the core framework for ESP-microcontrollers, 
   powered by *espressif*. This framework is under the hood on Arduino and PlatformIO. Today, our firmware uses some 3rd party libraries from the 
   Arduino/PlatformIO community. So it's easier to work with Arduino or Platform. If you like, you could spend some time on ESP-IDF as well.

### Arduino IDE Installation

Please use at least version 1.8.19 or the newest 2.x version.

To install and configure your Arduino IDE, please use the following steps: 

1. Download and install the latest version of Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software).

2. Start arduino ide, select **File/Preferences**. In the preferences dialog, you need to add at **Additional Board Manager URLs** the following URL :
<br>
**https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json**


3. Now open **Tools/Boards Manager** and install **esp32 by espressif systems** You need at least version 2.02. Check Boards Manager URL above, if only 1.x versions are listed.

4. Our ftSwarm firmware and library has some dependencies. Please install the following libraries using **Tools\Manage Libraries**:
    - **Adafruit GFX Library**, at least version 1.10.12
 	 - **Adafruit SSD1306**, at least version 2.5.3
    - **FastLED** by Daniel Garcia, at least version 3.4.0
	
5. Install the **ftSwarm** library. Download latest ftswarm.zip from [github](https://github.com/elektrofuzzis/ftSwarm/releases). 
   Use **Sketch\Include Library\Add .ZIP Library** to add the downloaded library.

6. Now you need to set the boards parameters in **Tools**.

<table>
   <tr><td>Option</td><td>ftSwarm</td><td>ftSwarmRS</td><td>ftSwarmControl</td></tr>
   <tr><td>Tools\Board\ESP32 Arduino</td><td>ESP32 Dev Module</td><td>ESP32S3 Dev Module</td><td>ESP32 Dev Module</td></tr>
   <tr><td>Tools\PSRAM</td><td>Enabled</td><td>Enabled</td><td>Enabled</td></tr>
   <tr><td>Tools\Core Debug Level:</td><td>none</td><td>none</td><td>none</td></tr>
   <tr><td>Tools\Arduino Runs On:</td><td>core 0</td><td>core 0</td><td>core 0</td></tr>
   <tr><td>Tools\Events Run On:</td><td>core 1</td><td>core 1</td><td>core 1</td></tr>
   <tr><td>Tools\Port:</td><td>select COM-Port</td><td>select COM-Port</td><td>select COM-Port</td></tr>
</table>

 
### Writing and uploading code

Arduino code is a sketch and stored as **.ino** files in a folder with the same name.

<style>
img { vertical-align: middle;important! }
</style>

- ![build](/assets/img/arduino_compile.png) compiles your software.
- ![upload](/assets/img/arduino_upload.png) uploads/flashes your software.
- ![serial](/assets/img/arduino_serial.png) opens the serial monitor.

Examples could be accessed using **File/Examples/Examples From Custom Libraries/ftSwarm**