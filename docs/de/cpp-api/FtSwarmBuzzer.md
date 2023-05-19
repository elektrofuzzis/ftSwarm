---
title: FtSwarmBuzzer
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Der Summer von fischertechnik kommt nur selten zum Einsatz, da er nur ein Signal wiedergeben kann. Von fischertechnik gibt es leider keine Angaben zum Stromverbrauch.  
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/otherActors/buzzer.png">Summer (36119)</div>
</div>

#### FtSwarmBuzzer(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmBuzzer Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_M1** oder **FTSWARM_M2**

#### FtSwarmBuzzer( const char *name )

Constructor um ein FtSwarmBuzzer Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### void on( void )

Schaltet den Summer ein.

#### void off( void )

Schaltet den Summer aus.