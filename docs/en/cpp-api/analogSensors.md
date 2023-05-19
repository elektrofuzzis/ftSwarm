---
title: Analog Sensor Overview
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
gallery:
 - url: "/en/cpp-api/FtSwarmLDR"
   image_path: "/assets/img/analog/photo-resistor.png"
   title: "Photo Resistor (32698)"
 - url: "/en/cpp-api/FtSwarmLDR"
   image_path: "/assets/img/analog/photo-transistor.png"
   title: "Photo Transistor (36134)"
 - url: "/en/cpp-api/FtSwarmOhmmeter"
   image_path: "/assets/img/analog/poti.png"
   title: "Potentiometer (186184)"
 - url: "/en/cpp-api/FtSwarmVoltmeter"
   image_path: "/assets/img/analog/voltmeter.png"
   title: "Voltage"
 - url: "/en/cpp-api/FtSwarmThermometer"
   image_path: "/assets/img/analog/sensor-ntc.png"
   title: "NTC resistor 1.5 kΩ (36437)"
 - url: "/en/cpp-api/FtSwarmJoystick"
   image_path: "/assets/img/analog/joystick.png"
   title: "Joystick"
---

These sensors are measuring analog signals like light intensity using the photo transistor or temperature by NTC.

Most of *fischertechnik* analog sensors have to pins. These need to be connected to the controllers input pins. Some sensors have a red marked pin. Connect the marked pin to the red cable at ftSwarm/ftSwarmControl, use the signal input pin at ftSwarmRS.

Check [ftSwarm](/en/gettingstarted/1stftSwarm) and [ftSwarmControl](/en/gettingstarted/1stftSwarmControl) pinouts for more details.

**ftSwarmControl:** You could just use FTSWARM_A1 with analog sensors.<br><br>
**ftSwarm:** You could use all inputs with analog sensors. Additionally you could measure voltage at FTSWARM_A2. 
{: .notice--info}

{% include gallery caption="Image source: fischertechnik & elektrofuzzis" %}



