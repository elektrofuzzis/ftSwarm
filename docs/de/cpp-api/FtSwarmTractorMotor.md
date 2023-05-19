---
title: FtSwarmTractorMotor
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Durch sein integriertes 25:1 Getriebe ist der Traktormotor sehr leistungsstark. Leider stellt fischertechnik kein technisches Datenblatt zur Verfügung, deshalb sollten Sie bei der Berechnung Ihres Powerbudgets von mindestens 500mA ausgehen.
    </div>
    <div class="apiright apiimg"><img title="BildnachweisS: fischertechnik" src="/assets/img/motor/motor-tractor.png">Traktor Motor (151178)</div>
</div>

Bitte lesen Sie auch die Erklärung zu Speed, Coast & Brake in der [Motorübersicht](../motors/).

#### FtSwarmTractorMotor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmTractorMotor Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_M1** oder **FTSWARM_M2**

#### FtSwarmTractorMotor( const char *name )

Constructor um ein FtSwarmTractorMotor Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### void setSpeed( int16_t speed )

Setzt die Motorgeschwindigkeit.

- speed: Geschwindigkeit im Bereich von -256 bis 256.

#### uint16_t getSpeed()

Gibt die Motorgeschwindigkeit zurück.

#### void setMotionType( FtSwarmMotion_t motionType )

Diese Funktion stellt den Betriebsmodus der Endstufe ein:
- **FTSWARM_COAST** der Motor wird angehalten, lässt sich aber manuell verstellen.
- **FTSWARM_BRAKE** der Motor wird angehalten aber gebremst. Der Motor lässt sich manuell nicht verstellen.
- **FTSWARM_ON** schaltet die Endstufe mit der ursprünglichen Geschwindigkeit wieder ein.

#### FtSwarmMotion_t getMotionType()

Liefert den eingestellten MotionType aus.

#### void coast( void )

Stellt die Endstufe in den **FTSWARM_COAST** Modus.

#### void brake( void )

Stellt die Endstufe in den **FTSWARM_BRAKE** Modus.