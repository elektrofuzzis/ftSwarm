---
title: FtSwarmLightBarrier
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Lichtschranken können mit einem Fototransistor und einer Linsenlampe gebaut werden. In dieser Klasse wird der Fototransistor als digitaler Sensor verwendet. Fällt das Licht der Linsenlampe auf den Sensor, so ist die Lichtschranke offen.<br><br>
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/switches/photo-transistor-api.png">Fototransistor (36134)</div>
</div>

Sollen LEDs eingesetzt werden, so müssen die speziell für Lichtschrankenanwendungen ausgewiesenen LEDs verwendet werden. Da Fototransistoren vom Umgebungslicht beeinflusst werden, sollten diese immer mit einer Tubuskappe vor Streulicht geschützt werden.

Beim Anschluss des Fototransistors muss auf die richtige Polung beachtet werden. Der rot markierte Pin muss an das rote Kabel bei ftSwarm/ftSwarmControl bzw. dem Eingangs-Pin am ftSwarmRS angeschlossen werden.

**ftSwarm**: Beim ftSwarm können Fototransistoren an allen Eingängen angeschlossen werden.<br>
**ftSwarmControl**: Hier können analoge Werte nur am Eingang **A1** ausgelesen werden.
{: .notice--info}

#### FtSwarmLightBarrier(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true)

Constructor um ein FtSwarmLightBarrier Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Portnummer, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**. ftSwarmControl unterstützt nur **FTSWARM_A1**.
- normallyOpen (optional): **true**, wenn die Lichtschranke Lichteinfall 0 liefern soll. **false** liefert bei Lichteinfall den Wert 1.

#### FtSwarmLightBarrier(const char *name, bool normallyOpen = true)

Constructor um ein FtSwarmLightBarrier Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.
- normallyOpen (optional): **true**, wenn die Lichtschranke Lichteinfall 0 liefern soll. **false** liefert bei Lichteinfall den Wert 1.

#### bool isPressed()

TRUE, wenn die Lichtschranke unterbrochen ist.

#### bool isReleased()

TRUE, wenn die Lichtschranke frei ist.

#### bool hasToggledUp()

Diese Funktion ist TRUE, wenn die Lichtschranke seit dem letzten Toggle-Aufruf unterbrochen wurde.

#### bool hasToggledDown()

Diese Funktion ist TRUE, wenn die Lichtschranke seit dem letzten Toggle-Aufruf frei wurde.

#### bool getState()

TRUE, wenn die Lichtschranke unterbrochen ist.

#### FtSwarmToggle_t getToggle()

Die Funktion gibt zurück:
- **FTSWARM_TOGGLEDOWN**, wenn die Lichtschranke seit dem letzten Toggle-Aufruf frei wurde.
- **FTSWARM_TOGGLEUP**, wenn die Lichtschranke seit dem letzten Toggle-Aufruf unterbrochen wurde.
- **FTSWARM_NOTOGGLE**, wenn die Lichtschranke seit dem letzten Toggle-Aufruf nicht verändert wurde.