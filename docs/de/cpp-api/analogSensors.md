---
title: Übersicht analoge Sensoren
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
gallery:
 - url: "/de/cpp-api/FtSwarmLDR/"
   image_path: "/assets/img/analog/photo-resistor.png"
   title: "Fotowiderstand (32698)"
 - url: "/de/cpp-api/FtSwarmLDR/"
   image_path: "/assets/img/analog/photo-transistor.png"
   title: "Fototransistor (36134)"
 - url: "/de/cpp-api/FtSwarmOhmmeter/"
   image_path: "/assets/img/analog/poti.png"
   title: "Potentiometer (186184)"
 - url: "/de/cpp-api/FtSwarmVoltmeter/"
   image_path: "/assets/img/analog/voltmeter.png"
   title: "Spannungsmessung"
 - url: "/de/cpp-api/FtSwarmThermometer/"
   image_path: "/assets/img/analog/sensor-ntc.png"
   title: "NTC Widerstand 1.5 kΩ (36437)"
 - url: "/de/cpp-api/FtSwarmJoystick/"
   image_path: "/assets/img/analog/joystick.png"
   title: "Joystick"
---

Diese Sensoren messen analoge Signale wie z.B. die Lichtstärke mit dem Fototransistor oder die Temperatur über den NTC Widerstand.

Die meisten analogen Sensoren von *fischertechnik* haben zwei Anschlüsse. Diese werden an die beiden Anschlüsse des Eingangs am Controller angeschlossen. Bei einigen Sensoren ist ein Anschluß rot markiert. Dieser Anschluß muss am beim ftSwarm/ftSwarmControl an das rote Kabel angeschlossen werden, beim ftSwarmRS an den Signalpin.

Die Anschlüsse von [ftSwarm](/en/gettingstarted/1stftSwarm) und [ftSwarmControl](/en/gettingstarted/1stftSwarmControl) sind auf den Pinout-Seiten beschrieben.

**ftSwarmControl:** Hier können analoge Sensoren nur am Eingang FTSWARM_A1 verwendet werden.<br><br>
**ftSwarm:** Beim ftSwarm können analoge Sensoren an allen Eingängen verwendet werden. Zusätzlich kann am Eingang FTSWARM_A2 Spannung gemessen werden. 
{: .notice--info}

{% include gallery caption="Bildnachweis: fischertechnik & elektrofuzzis" %}
