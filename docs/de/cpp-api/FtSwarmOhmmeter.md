---
title: FtSwarmOhmmeter
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Mit der Klasse FtSwarmOhmmeter können die Postion des fischertechnik-Potentimeters als auch andere Widerstandswerte gemessen werden. Der Messbereich des ftSwarmOhmmeter beträgt 0 bis 10 kOhm.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/analog/poti.png">Potentiometer (186184)</div>
</div>

Das Potentiometer hat drei Anschlüsse. Für die Widerstansmessung der Position kann entweder das Anschlussschema grün/schwarz oder rot/schwarz verwendet werden. Bei rot/grün kann die Position nicht gemessen werden, sie gibt den Komplettwiderstand des Potentiometers zurück. 

**ftSwarm**: Beim ftSwarm können Widerstände an allen Eingängen angeschlossen werden.<br>
**ftSwarmControl**: Hier können analoge Werte nur am Eingang **A1** ausgelesen werden.
{: .notice--info}

#### FtSwarmOhmmeter(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmOhmmeter Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Portnummer, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**. ftSwarmControl unterstützt nur **FTSWARM_A1**.

#### FtSwarmOhmmeter(const char *name)

Constructor um ein FtSwarmOhmmeter Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

####  float getResistance()

Gemessener Widerstandswert in Ohm.

#### int32_t getValue()

Eigentlicher Messwert des Analogwandlers (raw value).