---
title: FtSwarmLDR
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Fototransistoren und -Widerstände messen die Lichtintensität. In vielen Anwendungen werden sie deshalb als Lichtschranken verwendet. Da sie vom Umgebungslicht beeinflusst werden, sollten die beiden Typen immer mit einer Tubuskappe vor Streulicht geschützt werden.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/analog/ldr-api.png">Fotowiderstand (32698) und Fototransistor (36134)</div>
</div>

Der Fototransistor misst nur bestimmte Wellenlängen. Deshalb funktionieren normale *fischertechnik*-LEDs als Lichtquelle nicht. Am besten geeignet sind Linsenlampen und die für Lichtschranken spezifizierten LEDs.

Beim Anschluss des Fototransistors muss auf die richtige Polung beachtet werden. Der rot markierte Pin muss an das rote Kabel bei ftSwarm/ftSwarmControl bzw. dem Eingangspin am ftSwarmRS angeschlossen werden.

**ftSwarm**: Beim ftSwarm können Fototransistoren und -Widerstände an allen Eingängen angeschlossen werden.<br>
**ftSwarmControl**: Hier können analoge Werte nur am Eingang **A1** ausgelesen werden.
{: .notice--info}

#### FtSwarmLDR(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmLDR Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Portnummer, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**. ftSwarmControl unterstützt nur **FTSWARM_A1**.

#### FtSwarmLDR(const char *name)

Constructor um ein FtSwarmLDR Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Alias name of the IO port.

#### int32_t getValue()

Gibt den Messwert des Transistors oder Widerstandes zurück. Je geringer der Wert, desto höher ist die Lichtintensität.