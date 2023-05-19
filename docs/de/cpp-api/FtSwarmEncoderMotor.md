---
title: FtSwarmEncoderMotor
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Der Encoder-Motor ist weit mehr als nur ein leistungsstarker Motor: er verfügt über einen Encoder, der die Drehbewegung der Achse erfassen kann. Die Integration des Encoders ist allerdings noch nicht verfügbar, sie ist für eine spätere Firmwareversion geplant. Nutzen Sie den Encoder Motor solange als normalen Traktormotor.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/motor/motor-encoder.png">Encoder Motor (186175)</div>
</div>

Bitte lesen Sie auch die Erklärung zu Speed, Coast & Brake in der [Motorübersicht](../motor/).

#### FtSwarmEncoderMotor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmEncoderMotor Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_M1** oder **FTSWARM_M2**

#### FtSwarmEncoderMotor( const char *name )

Constructor um ein FtSwarmEncoderMotor Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

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