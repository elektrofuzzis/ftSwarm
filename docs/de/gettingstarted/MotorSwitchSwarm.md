---
title: Ein erster Schwarm
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---

[Your First Application](../MotorSwitch) zeigt wie eine ftSwarm-Applikation aussieht. Aber es ist auch nur eine einfache "Ein-Controller-Applikation".

Es soll nun ein Schwarm aus zwei COntrollern genutzt werden. Die Idee der Anwendung ist die gleiche - die Steuerung eines Motors durch einen Taster. Aber in dieser Anwendung sind Taster und Motor an verschiedene Controller angeschlossen.

Erster Controller:
- Der Taster wird am Eingang A1 angeschlossen.
- Ein 9V Netzteil wird an PWR angeschlossen.

Zweiter Controller:
- Ein Motor oder eine Lampe werden am Ausgang M2 angeschlossen.
- Ein 9V Netzteil wird an PWR angeschlossen.


### Einen Schwarm bilden

Um das Beispielprogramm auf dem ersten Controller zu starten, müssen zunächst beide Controller zu einem Schwarm verbunden werden. Auf dem zweiten Controller wird nur die Standardfirmware benötigt.

Auf dem ersten Controller muss für die Konfiguration die Standardfirmware **Examples\ftSwarm\firmware** geflashed werden**.

Nun wird ein Terminalprogramm gestartet und der Reset-Taster gedrückt. Drücken Sie eine Taste, um in das Firmwaremenü zu wechseln:

```
ftSwarmOS 0.5.0

(C) Christian Bergschneider & Stefan Fuss

Main Menu

(1) wifi settings
(2) webserver settings
(3) swarm settings
(4) alias names
(5) factory settings

(0) exit
main>
```

Die WLAN-Einstellungen sind bereits erledigt, somit müssen nur Schwarmeinstellungen gemacht werden. Verwenden Sie **(3)**, um das Schwarmmenü aufzurufen:

```
swarm menu

This device is connected to swarm "ftSwarm1" with 1 member(s) online.
Swarm PIN is 1.
(1) swarm communication: wifi
(2) create a new swarm
(3) join another swarm
(4) list swarm members

(0) main
swarm>
```

Mit **(2)** können sie einen neuen Schwarm mit eigenem Namen und neuer Pin erzeigen. Sie können aber auch die Standardeinstelungen verwenden (Schwarmname ftSwarm1 mit Pin 1 im obigen Beispiel).

Verbinden sie sich nun mit dem zweiten Controller. Stellen Sie seine WLAN Einstellungen, so dass er auch in ihrem lokalen WLAN arbeitet. Nun können sie ihn mit dem ersten Controller mit **(3) join another swarm** zu einem Schwarm zusammenschalten. Dabei werden Sie nach dem Schwarmnamen und dem Pin gefragt. 

### Eine verteilte Anwendung

Sobald beide COntroller einen Schwarm gebildet haben, kann das Biespiel **Examples\ftSwarm\MotorSwitchSwarm** gestartet werden:

```cpp
#include <ftSwarm.h>

// serial number of the second controller - change it to your 2nd device serial number
#define remote 2

FtSwarmSwitch *sw;
FtSwarmMotor  *mot;

void setup( ) {

  Serial.begin(115200);

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
	
  // get switch and motor instances
  sw  = new FtSwarmSwitch( local, FTSWARM_A1 );
  mot = new FtSwarmMotor( remote, FTSWARM_M2 );

}

void loop( ) {

  // check if switch is pressed or released
  if ( sw->isPressed() )
    mot->setSpeed(255);
  else
    mot->setSpeed(0);
	
  // wait some time
  delay(100);

}
```

Das Programm unterscheidet sich kaum vom [ersten Programm](../MotorSwitch). Es gibt nur zwei kleine Änderungen:

- **#define remote 2** definiert die Seriennummer des zweiten Controllers. Ändern Sie diese entsprechend Ihrer Controller ab.
- **FtSwarmMotor( remote, FTSWARM_M2 );** nutzt nun die Sieriennummer des zweiten Controllers. 

Auf der Statusseite werden nun beide COntroller angezeigt.

### Was passiert, wenn der zweite Controller nicht online ist?

Verbinden sie ein Terminalprogramm mit dem ersten Controller und trennen Sie am zweiten Controller die Stromversorgung. Starten Sie nun den ersten Controller. Bei **FtSwarmMotor( remote, FTSWARM_M2 );** kommt es nun zu einer Warning: ***Waiting on device***. Beider onboard RGB Leds werden blau. Die Firmware wartet darauf, dass der zweite Controller mit dem Motor online geht.

Schalten Sie am zweiten Controller die Stromversorgung jetzt wieder ein. Sobald das zweite Device gestartet ist, registriet es sich bei der Kelda und das Prgramm kann fortgesetzt werden.

Dieses Feature vereinfacht den Start Ihres Roboters. Es ist völlig egal, in welche Reihenfolge de Controller gestartet werden
