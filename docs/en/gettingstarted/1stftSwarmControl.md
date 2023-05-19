---
title: ftSwarmControl
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---

The ftSwarmControl is a nice litte control panel. Using the standard firmware, it could be used as a remote control within a swarm without writing any code. But you could use it as a kelda with an own program as well. 

Like the ftSwarm-controllers, all JST connectors are reversed polarity protected ports. Take care of using the pin header ports, because they are not protected.

|---------------|------|
| **ftSwarmControl** | | 
| **Pinout**    | <img alt="ftSwarmControl Pinout" src="/assets/img/ftSwarmControlPinout.png" width="250">  |
| **CPU**           | esp32-Wrover                               | 
| **Memory**        | 2 MB RAM, 4MB Flash                        | 
| **connectors**    | JST                                        | 
| **Motor outputs** | 2 x DC motors or lamps, 9V, max. 1A each   | 
| **Sensor inputs** | 1 x analog and 3 x digital sensors         | 
| **Display**       | 128x64 Pixel OLED display |
| **Buttons**       | eight onboard push buttons |
| **Joysticks**     | two onboard joysticks |
| **Gyro**          | optional MPU6040 gyro |
| **I²C interface** | 3.3V & 5V interface |
| **Communication** | wifi |

The device needs a 9V power supply. You could use a 9V cell block or a 9V external power supply. Using USB power only you could flash your program or firmware but the IO's won't work correctly,

An onboard 2A PTC fuse limts the device max. power budget. If you use all actor options, you need to calculate your power budget.

- *ftSwarmControl* device, max. 150mA
- *fischertechnik* XM Motor / 135485, max. 950mA
- *fischertechnik* XS Motor / 137096, max. 265mA
- *fischertechnik* Mini Motor / 32293, max. 300 mA
- *fischertechnik* Encoder Motor / 153422, max. 465 mA
- *fischertechnik* Encoder Motor Competition / 186175, max. 1.200mA
- *fischertechnik* all types of LEDs, max. 10mA
- *fischertechnik* Lens tip lamp / 37875, 150mA
- *fischertechnik* Compressor / 121470, 200mA
- *fischertechnik* 3/2-way solenoid valve / 35327, max. 133mA
- *fischertechnik* Micro Servo 4.8/6V / 132292, max. 250mA
