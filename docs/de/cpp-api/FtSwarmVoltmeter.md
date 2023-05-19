---
title: FtSwarmVotmeter
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Der ftSwarm kann am Eingang <stong>A2</stong> eine extern anliegende Spannung messen.<br>Der Messbereich des FtSwarmVoltmeter beträgt 0 bis 4 Volt.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: elektrofuzzis" src="/assets/img/analog/voltmeter-api.png">Spannung</div>
</div>

Alle anderen analogen Eingänge bzw. Messklassen bestimmen den am Eingang angeschlossenen Widerstand.

Spannungsmessung ist nur am Eingang **A1** des **ftSwarm** möglich.
{: .notice--info}

#### FtSwarmVotmeter(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmVotmeter Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Portnummer, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**. ftSwarmControl unterstützt nur **FTSWARM_A1**.

#### FtSwarmVotmeter(const char *name)

Constructor um ein FtSwarmVotmeter Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

####  float getVoltage()

Gibt de gemessene Spannung zurück.

#### int32_t getValue()

Eigentlicher Messwert des Analogwandlers (raw value).