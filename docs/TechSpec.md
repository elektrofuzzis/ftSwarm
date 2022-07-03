---
nav_order: 100
---
# Technical Specifications

|---------------|------|------|
|               | ![ftSwarm](/assets/img/ftSwarm_small.png)|![ftSwarmControl](/assets/img/ftSwarmControl_small.png)|
| Motor outputs | 2 DC motors or lamps, 9V, max. 1A each   | 2 DC motors or lamps, 9V, max. 1A each |
| Sensor inputs | 4 analog or digital sensors              | 4 digital sensors |
| Servo outputs | 1 6V servo                               | n/a |
| RGB leds      | 2 onboard leds<br>up to 14 external leds via extension port| n/a |
| Display       | n/a                                      | 128x64 Pixel OLED display |
| Buttons       | use sensor inputs                        | eight onboard push buttons |
| Joysticks     | use sensor inputs                        | two onboard joysticks |
| Gyro          | optional MPU6040 gyro                    | optional MPU6040 gyro |
| I²C interface | 3.3V interface, if no gyro is installed. | 3.3V and 5V onboard interface |

## Power Supply

The device needs an external 9V power supply. 

An onboard 2A PTC fuse limts the device max. power. If you use all actor options, you need to calculate your power budget.

- *ftSwarm* device, max. 150mA
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