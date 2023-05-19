---
title: FtSwarmXMotor
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        There is no difference in running a Mini-Motor or a XS-Motor. They just differ in power & size.
        So there is only one class to handle both of them.<br><br>
        Use this class for any type of gray motors as well.
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/motor/kombi.png">XS Motor (137096) & Mini Motor (32293)</div>
</div>

#### FtSwarmMotor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmMotor object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_M1** or **FTSWARM_M2**

#### FtSwarmMotor( const char *name )

Constructor to create a FtSwarmMotor object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void setSpeed( int16_t speed )

Sets the motors speed.

- speed: speed value in the range from -256 to 256

#### uint16_t getSpeed()

Returns the motor's speed.