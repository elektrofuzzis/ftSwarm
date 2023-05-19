---
title: FtSwarmServo
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        This class is used to control a fischertechnik Micro Servo 4.8/6V.<br>
        Servo motors use an internal control electronics to align the servo lever. So the commands are about setting the lever's angle.
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/otherActors/motor-servo.png">Micro Servo (132292)</div>
</div>

To simplify setting up a servo, we implemented two parameters: position and offset.
- Offset is used to trim a zero-position of your servo levers. It's the same idea as the trim wheel at the BT Control Set.
- Use position in your application to change the levers angle.

The servo extension port includes a 5V power supply. Take care on polarity: reversing the plug won't harm the servo, but it won't work.

Servos are only available on **ftSwarm** whereby **ftSwarm** has one and **ftSwarmRS** has two extension ports. 
{: .notice--info}

#### FtSwarmServo(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmServo object. If the referenced controller isn't connected to the swarm, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: **FTSWARM_SERVO1** or **FTSWARM_SERVO2**

#### FtSwarmServo( const char *name )

Constructor to create a FtSwarmServo object. If the referenced controller isn't connected to the swarm, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void setPosition( int16_t position )

Set the position/angle of the lever.

- position: a value between -256 and 256.

#### int16_t getPosition()

Returns the position of the lever.

#### void setOffset( int16_t offset )

Set zero position of the lever. Offset default value is 128 to set a middle zero position.

- offset: a value between 0 and 256.

#### int16_t getOffset()

Returns the zero offset of the lever.
