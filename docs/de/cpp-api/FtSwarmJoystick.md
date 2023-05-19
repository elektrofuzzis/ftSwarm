---
title: FtSwarmJoystick
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---

<div class="apicontainer">
    <div class="apileft">
        Der ftSwarmControl hat zwei eingebaute Joysticks.<br><br>
        Beim Auslesen der Position werden Bewegungen nach links oder nach oben als negative Werte, nach rechts und nach unten als positive Werte zurückgegeben.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/analog/joystick.png">ftSwarmControl</div>
</div>

Die Ruhestellung hat die Werte 0/0. In der Firmware kann die Ruhestellung der Joysticks kalibriert werden.

#### FtSwarmJoystick(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmJoystick Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Portnummer, **FTSWARM_JOY1** oder **FTSWARM_JOY2**.

#### FtSwarmJoystick(const char *name)

Constructor um ein FtSwarmJoystick Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### int16_t getFB()

Diese Funktion liefert den Wert der vor/zurück-Bewegung.

#### int16_t getLR()

Diese Funktion liefert den Wert der links/rechts-Bewegung.

#### bool getButtonState()

Diese Funktion liefert den Zustand des Tasters.

#### void getValue( int16_t *FB, int16_t *LR, bool *buttonState )

Diese Funktion liefert den kompletten Zustand des Joysticks in einer Funktion.