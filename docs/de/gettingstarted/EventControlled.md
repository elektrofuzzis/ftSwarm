---
title: Einfache Programmierung durch Events
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---

Die meisten Modelle brauchen nur ein sehr einfache Programme. Ein Taster schaltet einen Motor. In **setup** werden Taster und Motor definiert. Der Zustand des Tasters wird in **loop** überwacht und der Motor geschaltet. 

Der Versuchsaufbau ist identisch zu "Remote Control":

ftSwarmControl:
- Den Controller mit 9V versorgen.
- Den Aliasnamen des ersten Joysticks auf "joystick" setzen. 
- Den Aliasnamen des S1-Buttons auf "button" setzen. 

ftSwarm:
- Einen Motor am Ausgang M1 anschließen
- Eine Lampe oder eine LED am Ausgang M2 anschließen
- Den Controller mit 9V versorgen.
- Den Aliasnamen von M1 auf "motor" setzen. 
- Den Aliasnamen von M2 auf "lamp" setzen. 

Kontrollieren Sie, ob Sie alle Events in "Remote Control" gelöscht haben.

Den Quellcode finden Sie unter ftSwarm/Examples: **EventControlled**

```cpp
void setup() {
  
  // start swarm
  ftSwarm.begin();
  
  // define needed IOs
  FtSwarmButton   button   = new FtSwarmButton( "button" );
  FtSwarmJoystick joystick = new FtSwarmJoystick( "joystick" );
  FtSwarmXMotor   motor    = new FtSwarmXMotor( "motor" );
  FtSwarmLamp     lamp     = new FtSwarmLamp( "lamp" );
  
  // button S1 controls the lamp
  button.onTrigger( FTSWARM_TRIGGERUP, lamp, 255 );
  button.onTrigger( FTSWARM_TRIGGERDOWN, lamp, 0 );
  
  // joystick uses it's readings to set the motor's speed
  joystick.onTriggerLR( FTSWARM_TRIGGERVALUE, motor );
  
}

void loop() {
}
```

Auf den ersten Blick ist die leere **loop**-Funktion verwirred. Dennoch hat das Beispiel die gleiche Funktion wie im Beispiel mit der Fernbedienung. 

Die komplette Funktion steckt im **setup** Code:

- Als erstes werden die IOs über Aliasnamen definiert.
- Im zweiten Abschnitt werden Trigger auf den Button gelegt. Ist der Button gedrückt (TRIGGERUP), wird die Lampe/LED eingeschaltet. Lässt man den Button los (TRIGGERDOWN), so wird die Lampe ausgeschaltet.
- Der dritte Block nutzt den L/R-Wert des Joysticks um die Motorgeschwindigkeit zu setzen. Wird der Joystick nach links bewegt, ist das Joystick-Signal negativ und der Motor dreht gegen den Uhrzeigersinn. Bewegt man den Jocstick nach rechts wird das Signal positiv und der Motor ändert seine Drehrichtung. Steht der Joystick in der Mitte - auf 0 - so ist der Motor aus.