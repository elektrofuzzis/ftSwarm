---
title: ftSwarmControl
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---
Der ftSwarmControl ist ein fertiges Steuerpult für den Schwarm. it der Standardfirmware kann der ftSwarmControl auch als Fernbedienung genutzt werden, ohne dabei eine Zeile Code schreiben zu müssen. Er kann aber auch als Kelda mit einem eigenen Programm verwendet werden.

Alle Anschlüsse auf den JST-Steckern sind gegen Verpolung geschützt. Die Anschlüsse auf den Steckerleisten sind nicht geschützt.

|---------------|------|
| **ftSwarmControl** | | 
| **Pinout**    | <img alt="ftSwarmControl Pinout" src="/assets/img/ftSwarmControlPinout.png" width="250">  |
| **CPU**           | esp32-Wrover                               | 
| **Speicher**        | 2 MB RAM, 4MB Flash                        | 
| **Anschlüsse**    | JST                                        | 
| **Motor Ausgänge** | 2 x DC motors or lamps, 9V, max. 1A each   | 
| **Sensor Eingänge** | 1 x analog and 3 x digital sensors         | 
| **Display**       | 128x64 Pixel OLED display |
| **Buttons**       | eight onboard push buttons |
| **Joysticks**     | two onboard joysticks |
| **Gyro**          | optional MPU6040 gyro |
| **I²C Interface** | 3.3V & 5V interface |
| **Kommunikation** | wifi |
| **USB Anschluss** | Micro-USB |

Für den Betrieb benötigt der Controller benötigt eine externe 9V Stromversorgung. Das Flashen von Firmware oder von Programmen ist im reinen USB-Betrieb möglich. Ohne 9V Versorgung funktionieren die Ein- und Ausgänge nicht.

Der Controller wird durch eine 2A Sicherung geschützt. Werden alle Ausgänge verwendet, kann es sein dass das Powerbudget von 2A überschritten wird. 

- *ftSwarm* Controller, max. 150mA
- *ftSwarm* RGB-LED, max. 60mA each
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
