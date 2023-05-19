---
title: Switches & Buttons
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---

Es gibt nur einen kleinen Unterschied zwischen Buttons und Switches. Buttons sind die am ftSwarmControl fest verbauten Taster. Switches sind externe, an die Eingänge A1 bis A6 angeschlossene Taster.

### Normally closed & normally open

*fischertechnik* mini switches haben 3 Anschlüsse. Im Standard werden Pin 1 und Pin 3 am mini switch angeschlossen. Der Taster ist "normally open". Beim Drücken des Tasters wird der Schalter geschlossen. Verwendet man Pin  1 & 2 ist der Taster "normally closed" und öffnet beim Tastendruck. Diese Variante ist z.B. bei Notabschaltungen besser, da ein gebrochenes Kabel oder ein schlechter Stecker ebenfalls die Notabschaltung auslöst.

```cpp
normallyOpen   = FtSwarmSwitch( 4711, FTSWARM_A1 );
normallyClosed = FtSwarmSwitch( 4711, FTSWARM_A2, false );
```

### Toggles

In den meisten Fällen ist das Signal eines Eingangs bzw. eines Tasters egal, nur der Statuswechsel ist von interesse. Mit

```cpp
while (switch.isReleased());
```

kann man aktiv darauf warten, dass der Taster gedrückt wird. Aber durch die Schleife wird ein Core belegt und der Controller kann keine anderen Aktionen ausführen, während er auf den Taster wartet. Die Firmware liest im Hintergrund die Eingänge regelmäßig aus und kann so Statuswechse erkennen. Der Code

```cpp
void loop() {

  if (switch.hasToggledUp()) motor.setSpeed(100);
  if (switch.hasToggledDown()) motor.setSpeed(0);
  
  // use the time and do someting else
```

ist cleverer, da die CPU beim Warten nicht blockiert wird.

Jeder Aufruf der *Toggle* Befehle setzt den Togglestatus zurück. Zwei two Toggle-Befehle in einer Bedingung

```cpp
if ( (switch.getToggle() != FTSWARM_NOTOGGLE) && (switch.hasToggeledUp()) ) ...
```

is always false. Even if `switch.getToggle()` returns `FTSWARMTOGGLEUP` or `FTSWARMTOGGLEDOWN`, the internal toggle state is reseted to `FTSWARM_NOTOGGLE`. So `switch.hasToggeledUp()` will be always `false`.
