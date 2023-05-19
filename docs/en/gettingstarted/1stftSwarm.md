---
title: ftSwarm
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---

There are two types available: ftSwarm and ftSwarmRS. The ftSwarmRS is a little bit more powerful and has an additional RS485 interface to communicate within the swarm.

Both types are reversed polarity protected on all JST connectors or srew terminal ports. Take care of using the pin header ports, because they are not protected.

|---------------|------|------|
| | **ftSwarm** | **ftSwarmRS** |
| **Pinout**    | <img alt="ftSwarm Pinout" src="/assets/img/ftSwarmPinout.png" width="250"> | <img alt="ftSwarm Pinout" src="/assets/img/ftSwarmRSPinout.png" width="250"> |
| **CPU**           | esp32-Wrover                               | esp32-S3 |
| **Memory**        | 2 MB RAM, 4MB Flash                        | 2 MB RAM, 8MB Flash |
| **connectors**    | JST                                        | screw terminal |
| **Motor outputs** | 2 x DC motors or lamps, 9V, max. 1A each   | 2 x DC motors or lamps, 9V, max. 1A each |
| **Sensor inputs** | 4 x analog or digital sensors              | 6 x analog or digital sensors |
| **Servo outputs** | 1 x 6V servo                               | 2 x 6V servo |
| **RGB leds**      | 2 onboard leds<br>up to 16 external ftPixel via extension port| 2 onboard leds<br>up to 16 external ftPixel via extension port |
| **Gyro**          | optional MPU6040 gyro                    | onboard LSM6 gyro |
| **I²C interface** | 3.3V interface, if no gyro is installed. | 3.3V interface |
| **Communication** | wifi | wifi or RS485 |
| **USB**           | micro USB | USB-C |

The device needs an external 9V power supply. Using USB power only you could flash your program or firmware but the IO's won't work correctly.

An onboard 2A PTC fuse limts the device max. power budget. If you use all actor options, you need to calculate your power budget.
{: .notice--info}

- *ftSwarm* device, max. 270mA
- *fischertechnik* XM Motor / 135485, max. 950mA
- *fischertechnik* XS Motor / 137096, max. 265mA
- *fischertechnik* Mini Motor / 32293, max. 300 mA
- *fischertechnik* Encoder Motor / 153422, max. 465 mA
- *fischertechnik* Encoder Motor Competition / 186175, max. 1.200mA
- *fischertechnik* all types of LEDs, max. 10mA
- *fischertechnik* Lens tip lamp / 37875, max. 150mA
- *fischertechnik* Compressor / 121470, max. 200mA
- *fischertechnik* 3/2-way solenoid valve / 35327, max. 133mA
- *fischertechnik* Micro Servo 4.8/6V / 132292, max. 250mA
