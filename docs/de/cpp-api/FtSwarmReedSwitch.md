---
title: FtSwarmReedSwitch
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Reedkontakte reagieren auf starke Magnetfelder.
        Mit dem Magnetbaustein von fischertechnik oder den Neodymmagneten von fischerfriendsman lassen sich kontaktlose Schalter bauen.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/switches/switch-reed-api.png">Reedkontakt (36120)</div>
</div>

Reedkontakte werden an beide Anschlüsse des Eingangs angeschlossen. Eine 9V-Stromversorgung wird nicht benötigt.

#### FtSwarmReedSwitch(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true)

Constructor um ein FtSwarmReedSwitch Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Portnummer, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**.
- normallyOpen (optional): **true**, wenn der Magnet am Reedkontakt den Wert 1 auslösen soll. Mit **false** liefert der Reedkontakt ohne Magnet den Wert 1.

#### FtSwarmReedSwitch(const char *name, bool normallyOpen = true)

Constructor um ein FtSwarmReedSwitch Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.
- normallyOpen (optional): **true**, wenn der Magnet am Reedkontakt den Wert 1 auslösen soll. Mit **false** liefert der Reedkontakt ohne Magnet den Wert 1.

#### bool isPressed()

TRUE, wenn der Magnet den Reedkontakt aktiviert.

#### bool isReleased()

TRUE, wenn der Magnet den Reedkontakt nicht aktiviert.

#### bool hasToggledUp()

TRUE, wenn seit dem letzten Toggle-Call der Magnet den Reedkontakt ausgelöst hat.

#### bool hasToggledDown()

TRUE, wenn seit dem letzten Toggle-Call der Magnet den Reedkontakt freigegeben hat.

#### bool getState()

TRUE, wenn der Magnet den Reedkontakt aktiviert.

#### FtSwarmToggle_t getToggle()

Die Funktion gibt zurück:
- **FTSWARM_TOGGLEDOWN**, wenn der Magnet seit dem letzten Toggle-Aufruf den Reedkontakt freigegeben hat.
- **FTSWARM_TOGGLEUP**, wenn der Magnet seit dem letzten Toggle-Aufruf den Reedkontakt ausgelöst hat.
- **FTSWARM_NOTOGGLE**, wenn der Magnet seit dem letzten Toggle-Aufruf den Reedkonakt nicht verändert hat.