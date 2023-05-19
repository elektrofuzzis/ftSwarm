---
title: FtSwarmCompressor
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Der fischertechnik Kompressor wird zur Bereitstellung von Druckluft für Pneumatikzyliner benötigt. Die Zylinder werden nie direkt über den Kompressor gesteuert, dafür werden zusätzlich ein paar <a href="../valve/">Ventile</a> geschaltet. Um Schwankungen in der Luftversorgung auszugleichen, sollten Sie immer ein paar Lufttanks einbauen.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/otherActors/compressor.png">Kompressor (121470)</div>
</div>

#### FtSwarmCompressor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmCompressor Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_M1** oder **FTSWARM_M2**

#### FtSwarmCompressor( const char *name )

Constructor um ein FtSwarmCompressor Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### void on( void )

Schaltet den Kompressor ein.

#### void off( void )

Schaltet den Kompressor aus.