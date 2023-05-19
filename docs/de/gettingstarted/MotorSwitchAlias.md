---
title: Alias Names
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---

Bislang wurden die IOs in den Beispielen über die Seriennummer des ftSwarm und dem Portnamen angesprochen. Das ist praktisch in kleinen Modellen, aber in größeren Modellen verliert man schnell die Übersicht. Dort ist es einfacher, die IOs anhand deren Funktion anzusprechen:

```cpp
  SwitchWithSN     = new FtSwarmSwitch( 4711, FTSWARM_A1 );  // Old style: with serial number and port
  SwitchWithAlias  = new FtSwarmSwitch( "MotorEndStop" );    // New style: call my alias name!
```

Das Beispiel nutzt den gleichen Aufbau wie zuvor:

Erster Controller:
- Der Taster wird am Eingang A1 angeschlossen.
- Ein 9V Netzteil wird an PWR angeschlossen.

Zweiter Controller:
- Ein Motor oder eine Lampe werden am Ausgang M2 angeschlossen.
- Ein 9V Netzteil wird an PWR angeschlossen.

Es müssen nun zunächst Aliasnamen für den Switch und den Motor vergeben werden. Dazu muss die Standradfirmware geladen sein. Über die die Terminalemulation im Hautpmenü **(4) alias names** auswählen:

```
alias controler menu:

( 1) hostname ftSwarm100 - 
( 2) A1   - switch                                 
( 3) A2   -                                 
( 4) A3   -                                 
( 5) A4   -                                 
( 6) A5   -                                 
( 7) A6   -                                 
( 8) M1   -                                 
( 9) M2   -                                 
(10) LED1 -                                 
(11) LED2 -                                 
(12) SERVO -                                 

(0) exit
alias>
```

 Am ersten Controller muss für **A1** der Name **switch** eingestellt werden, am zweiten Controller für **M2** der Name **motor**. Das Beispielprogramm kann von **Example/MotorSwitchAlias** übernommen werden:

```cpp
#include <ftSwarm.h>

FtSwarmSwitch *sw;
FtSwarmMotor  *mot;

void setup( ) {

  Serial.begin(115200);

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
  
  // get switch and motor instances
  sw  = new FtSwarmSwitch( "switch" );
  mot = new FtSwarmMotor( "motor" );

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

Auf der Statusseite des Swarms werden die Aliasnamen ebenfalls angezeigt.

Wird das Modell nun verändert, z.B. indem der Motor auf den Anschluss **M2**  des ersten Controllers gesteckt wird, muss das Programm nicht verändert werden. Der Aliasname **motor** muss nur auf dem ersten Controller eingetragen werden.

Achten Sie darauf, die Aliasnamen im Schwarm eindeutig zu vergeben! In diesem Beispiel ist es egal, ob Sie **motor** doppelt vergeben. Die Firmware verwendet immer zunächst den lokalen Controller. Werden mehrere Remotecontroller verwendet, so hängt das Ergebnis von der Bootreihenfolge der Controller ab.
{: .notice--info}
