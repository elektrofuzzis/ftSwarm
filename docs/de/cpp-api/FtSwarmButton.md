---
title: FtSwarmButton
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Der ftSwarmControl hat acht eingebaute Taster. Die Taster können im Modell frei verwendet werden.
        Vier liegen zwischen dem Reset- und dem Powerschalter, zwei sind an der Vorderseite und ein Taster ist in jedem Joystick. 
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/ftSwarmControl.png">ftSwarmControl</div>
</div>

Die Taster sind intern angeschlossen. Die externen Anschlüsse können bei der Nutzung der Taster frei verwendet werden. 

Die Klasse FtSwarmButton funktioniert nur mit den On-Board-Tastern des ftSwarmControl.
{: .notice--info}

#### FtSwarmButton(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmButton Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_S1**...**FTSWARM_S2** (Standardtasten), **FTSWARM_J1** & **FTSWARM_J2** (Joystick), **FTSWARM_F1** & **FTSWARM_F2** (Frontside)
  
  
#### FtSwarmButton(const char *name)

Constructor um ein FtSwarmButton Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des Tasters.

#### bool isPressed()

Diese Funktion ist true, wenn der Taster gedrückt wird.

#### bool isReleased()

Diese Funktion ist true, wenn der Taster nicht gedrückt wird.

#### bool hasToggledUp()

Diese Funktion ist true, wenn der Switch seit dem letzten *Toggle*-Call gedrückt wurde.

#### bool hasToggledDown()

Diese Funktion ist true, wenn der Switch seit dem letzten *Toggle*-Call losgelassen wurde.

#### bool getState()

Diese Funktion ist true, wenn der Taster gedrückt wird.

#### FtSwarmToggle_t getToggle()

Diese Funktion liefert
- **FTSWARM_TOGGLEDOWN**, wenn der Taster seit dem letzten *Toggle*-Call losgelassen wurde.
- **FTSWARM_TOGGLEUP**, wenn der Taster seit dem letzten *Toggle*-Call gedrückt wurde.
- **FTSWARM_NOTOGGLE**, wenn der Taster seit dem letzten *Toggle*-Call weder gedrückt noch losgelassen wurde.