---
title: FtSwarmXMotor
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        In der Ansteuerung sind Mini-Motor und XS-Motor gleich. Sie unterscheiden sich nur in Leistung und Größe.
        Deshalb gibt es nur eine Klasse um die beiden Typen anzusteuern.<br><br>
        Diese Klasse kann ebenfalls für alle grauen Motoren genutzt werden.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/motor/kombi.png">XS Motor (137096) & Mini Motor (32293)</div>
</div>

#### FtSwarmMotor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmMotor Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_M1** oder **FTSWARM_M2**

#### FtSwarmMotor( const char *name )

Constructor um ein FtSwarmEncoderMotor Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### void setSpeed( int16_t speed )

Setzt die Motorgeschwindigkeit.

- speed: Geschwindigkeit im Bereich von -256 bis 256.

#### uint16_t getSpeed()

Gibt die Motorgeschwindigkeit zurück.