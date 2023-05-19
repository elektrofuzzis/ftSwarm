---
title: FtSwarmServo
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Diese Klasse dient zur Ansteuerung eines fischertechnik Micro Servos (4.8/6V).<br>
        Servomotoren verwenden eine interne Steuerelektronik, die den Servohebel auszurichtet. Die Kommados der Klasse stellen deshalb den Winkel des Servohebels ein.
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/otherActors/motor-servo.png">Micro Servo (132292)</div>
</div>

Um die Servomotoren zu vereinfachen gibt es zwei Parameter:
- **Offset** wird dazu verwendet, um die Nullstellung des Servohebels zu trimmen. Es ist somit die elektrische Variante des Trimmrads am BT Control Set.
- **Position** wird zur eigentlichen Steuerung des Winkels in der Applikation verwendet.

Der Servoextensioport hat eine integrierte 5-V-Stromversorgung. Beachten Sie aber die Polarität: Verdrehte Stecker schädigen die Servos nicht, aber der Servo wird nicht funktionieren.

Servos sind nur bei **ftSwarm** verfügbar, der **ftSwarm** verfügt über einen und der **ftSwarmRS** über zwei Extensionports.
{: .notice--info}

#### FtSwarmServo(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmServo Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_SERVO1** oder **FTSWARM_SSERVO2**

#### FtSwarmServo( const char *name )

Constructor um ein FtSwarmServo Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### void setPosition( int16_t position )

Setzt die Position bzw. den Winkel des Servoarms.

- position: Wert zwischen -256 und 256.

#### int16_t getPosition()

Gibt die Position bzw. den Winkel des Servoarms zurück.

#### void setOffset( int16_t offset )

Setzt die Nullstellung des Servoarms. Der Defaultwert ist 128 für eine mittlere Position.

- offset: Wert zwischen 0 und 256.

#### int16_t getOffset()

Gibt den Offest bzw. die Nullstellung des Servoarms zurück.
