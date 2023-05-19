---
title: ftSwarm
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---

Von diesem Controller gibt es zwei Varianten: ftSwarm und ftSwarmRS. Der ftSwarmRS ist etwas leistungsfähiger und kann zusätzlich die Kommunikation im Swarm über eine RS485 Schnittstelle.

Beide Typen sind gegen Verpolung an den JST- und Schraubklemmen geschützt. Ein Verpolungsschutz ist bei den Signalen an den Stiftleisten nicht möglich.

|---------------|------|------|
| | **ftSwarm** | **ftSwarmRS** |
| **Pinout**    | <img alt="ftSwarm Pinout" src="/assets/img/ftSwarmPinout.png" width="250"> | <img alt="ftSwarm Pinout" src="/assets/img/ftSwarmRSPinout.png" width="250"> |
| **CPU**             | esp32-Wrover                                      | esp32-S3 |
| **Speicher**        | 2 MB RAM, 4MB Flash                               | 2 MB RAM, 8MB Flash |
| **Anschlüsse**      | JST                                               | Schraubklemmen |
| **Motor Ausgänge**  | 2 DC Motoren or Lampen, 9V, max. 1A pro Ausgang   | 2 DC Motoren or Lampen, 9V, max. 1A pro Ausgang |
| **Sensor Eingänge** | 4 analoge oder digitale Sensoren                  | 6 x analoge oder digitale Sensoren |
| **Servo Ausgänge**  | 1 x 6V Servo                                      | 2 x 6V Servo |
| **RGB LEDs**        | 2 onboard LEDs<br>bis zu 16 externe ftPixel       | 2 2 onboard LEDs<br>bis zu 16 externe ftPixel |
| **Gyro**            | optional MPU6040 Gyro                             | onboard LSM6 Gyro |
| **I²C Interface**   | 3.3V Interface, sofern kein Gyro angeschlossen.   | 3.3V Interface |
| **Kommunikation**   | wifi                                              | wifi oder RS485 |
| **USB Anschluß**    | Micro-USB                                         | USB-C |

Der Controller benötigt eine externe 9V Stromversorgung. Wird nur die USB-Buchse verwendet, so können nur die Firmware oder Programme geflashed werden; die Ein- und Ausgänge funktionen dann nicht.

Der Controller wird durch eine 2A Sicherung geschützt. Der maximale Stromverbrauch eines Controllers ist so auf 2A begrenzt. Wenn alle Ausgänge benutzt werden, muss der Gesamtverbrauch berechnet werden.
{: .notice--info}

- *ftSwarm* Controller, max. 270mA
- *fischertechnik* XM Motor / 135485, max. 950mA
- *fischertechnik* XS Motor / 137096, max. 265mA
- *fischertechnik* Mini Motor / 32293, max. 300 mA
- *fischertechnik* Encoder Motor / 153422, max. 465 mA
- *fischertechnik* Encoder Motor Competition / 186175, max. 1.200mA
- *fischertechnik* alle LED-Varianten, max. 10mA
- *fischertechnik* Linsenlampe / 37875, max. 150mA
- *fischertechnik* Compressor / 121470, max. 200mA
- *fischertechnik* 3/2-Weg Ventil / 35327, max. 133mA
- *fischertechnik* Micro Servo 4.8/6V / 132292, max. 250mA
