---
title: Eine Fernbedienung ohne eine Zeile Code 
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---
In diesem Beispiel zeigen wir, wie man den Swarm als Fernbedienung nutzt, ohne dabei eine einzige Zeile Code zu schreiben.

Diese Funktion ist nur am ftSwarmControl verfügbar. Selbst wenn Sie keinen ftSwarmCOntrol haben, so sollten Sie die folgende Erklärung lesen. Sie brauchen sie für das nächste Beispiel zu eventgesteuerter Programmierung.

Die Idee hinter der Fernbedienung ist sehr einfach. Jeder Sensor - Eingang, Button oder Joystick - wird in der Firmware alle 25ms abgefragt. Die Firmware erkennt Signaländerungen und kann deshalb selbständig Motoren, Servos oder LEDs schalten.

Jeder Sensor kennt drei Trigger-Events:
- **TriggerUp:** Dieses Event wird ausgelöst, wenn das Signal eines digitalen Sensors von 0 auf 1 springt. Z.B. beim Drücken eines Buttons oder eines Tasters.
- **TriggerDown:** Dieses Event ist das Gegenteil von TriggerUp. Es wird ausgelöst, wenn Button oder Taster losgelassen werden.
- **ChangeValue:** Dieses Signal reagiert auf jede Statusänderung. Es ist dafür gedacht analoge Signale - wie z.B. einem Joystick - zur Steuerung der Motorgeschwindigkeit zu verwenden.

Jeder Trigger löst eine Aktion aus. Dies kann die Motorgeschwindigkeit, eine Servoposition oder die Farbe eine RGB-LED sein. Als Steuerwert kann entweder ein statischer Wert oder das aktuelle Eingangssignal verwendet werden.

Der Versuchsaufbau besteht aus einem ftSwarmControl und einem ftSwarm:

ftSwarmControl:
- Nutzt die Standardfirmware.
- Den Controller mit 9V versorgen.
- Den Aliasnamen des ersten Joysticks auf "joystick" setzen. 
- Den Aliasnamen des S1-Buttons auf "button" setzen. 

ftSwarm:
- Nutzt die Standardfirmware.
- Einen Motor am Ausgang M1 anschließen
- Eine Lampe oder eine LED am Ausgang M1´2 anschließen
- Den Controller mit 9V versorgen.
- Den Aliasnamen von M1 auf "motor" setzen. 
- Den Aliasnamen von M2 auf "lamp" setzen. 

Auf dem ftSwarmControl soll nun der Taster die Lampe ein- und ausschalten. Der Joystick soll den Motor steuern. Dazu müssen in **Remote Control** die folgenden Events eingestellt werden:

```
remote control menu:

     sensor             event       actor           value
( 1) switch             TriggerUp   lamp            255
( 2) switch             TriggerDown lamp            0
( 3) joystick        LR ChangeValue motor           SENSORVALUE

(4) add event
(5) delete event

(0) exit
```

Nun können Sie die Fernbedienung testen. Der Taster steuert die Lampe und der Joystick den Motor.

DIe EInstellungen werden im NVS-Speicher des fTSwarmControl gespeichert. Nach dem STart versucht der ftSwarmControl die angegebenen Sensoren und Aktoren anzusteuern. Sind diese an einem anderen Controller im Schwarm angeschlossen, so wartet ggf. die Firmware bis der Controller online ist.