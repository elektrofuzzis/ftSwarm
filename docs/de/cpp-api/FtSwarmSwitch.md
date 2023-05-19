---
title: FtSwarmSwitch
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Diese Klasse bildet alle möglichen Schalter ab. Dies können Mini-Taster, Polwendeschalter aber auch aus Stahlachsen selbst konstruierte Schalter sein.<br><br>
        Je nach Anwendungsfall werden die Schalter "normally open" oder "normally closed" betrieben. 
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/switches/kombi.png">Mini-Taster (37783) und Polwendeschalter (36708)</div>
</div>

Der **Mini-Taster** hat drei Anschlüsse. Je nach Anschlußschema kann der Schalter als "normally open" (Pins 1+3) oder als "normally closed" (Pins 1+2) verwendet werden. Beim **Polwendeschalter** wird je ein Pin der linken und ein Pin der rechten Seite verwendet. 

**Normally open** ist der Anschlussklassiker. Wird der Taster gedrückt, so wird der Stromkreis geschlossen und der FtSwarmSwitch im Programm meldet "pressed".

Normally Open Schalter können parallel geschalten werden um Eingänge zu "sparen". Wird ein beliebiger Taster gedrückt, so wird der Stromkreis geschlossen und der FtSwarmSwitch im Programm meldet "pressed".

**Normally closed** wird verwendet, wenn das Signal eine Funktion unterbrechen soll. Solange der Taster geschlossen bzw. der Stromkreis nicht unterbrochen ist, soll z.B. ein Motor laufen. Dies ist die klassische Notaus-Funktion: Unterbricht der Taster nun den Stromkreis oder wird der Stromkreis druch einen Kabelfehler unterbrochen, so stoppt der Motor.

Normally Closed Schalter werden in Reihe geschaltet, um die Notausfunktion mit mehreren Schaltern zu realisiseren. Dabei werden je ein Pin des ersten und des letzten Schalters mit dem Eingang des ftSwarm verbunden. Wird ein beliebiger Taster gedrückt, so wird der Stromkreis unterbrochen und der FtSwarmSwitch im Programm meldet "pressed".

#### FtSwarmSwitch(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true)

Constructor um ein FtSwarmSwitch Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Portnummer, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**.
- normallyOpen (optional): **true**, wenn Sie Pin 1+3 am Mini-Taster verwenden. **false** um die Pins 1+2 zu verwenden.

#### FtSwarmSwitch(const char *name, bool normallyOpen = true)

Constructor um ein FtSwarmSwitch Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.
- normallyOpen (optional): **true**, wenn Sie Pin 1+3 am Mini-Taster verwenden. **false** um die Pins 1+2 zu verwenden.

#### bool isPressed()

TRUE, wenn der Taster gedrückt ist.

#### bool isReleased()

TRUE, wenn der Taster nicht gedrückt ist.

#### bool hasToggledUp()

Die Funktion ist TRUE, wenn seit dem letzten Toggle-Call der Taster gedrückt wurde.

#### bool hasToggledDown()

Die Funktion ist TRUE, wenn seit dem letzten Toggle-Call der Taster losgelassen wurde.

#### bool getState()

TRUE, wenn der Taster gedrückt ist.

#### FtSwarmToggle_t getToggle()

Diese Funktion liefert
- **FTSWARM_TOGGLEDOWN**, wenn seit dem letzten Toggle-Call der Taster losgelassen wurde.
- **FTSWARM_TOGGLEUP**, wenn seit dem letzten Toggle-Call der Taster losgelassen wurde.
- **FTSWARM_NOTOGGLE**, wenn sich der Zustand des Schalters seit dem letzten \*Toggle\*-Call nicht geändert hat.