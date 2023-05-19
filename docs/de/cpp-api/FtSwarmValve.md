---
title: FtSwarmValve
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Das Ventil wird zur Steuerung von Pneumatikzylindern eingesetzt. Zusätzlich zum Ventil bruachen Sie einen Kompressor um die benötigte Luft zu erzeugen und einige Lufttanks um Schwankungen im Luftstrom auszugleichen.
    </div>
    <div class="apiright apiimg"><img title="Bldnachweis: fischertechnik" src="/assets/img/otherActors/valve.png">3/2-Wege Magnetventil (35327)</div>
</div>

#### FtSwarmValve(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmValve Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_M1** oder **FTSWARM_M2**

#### FtSwarmValve( const char *name )

Constructor um ein FtSwarmValve Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### void on( void )

Schaltet das Ventil ein. Der Pneumatikzylinder fährt aus.

#### void off( void )

Schaltet das Ventil aus. Der Pneumatikzylinder fährt zurück.