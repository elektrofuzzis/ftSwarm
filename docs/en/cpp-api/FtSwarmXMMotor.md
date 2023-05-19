---
title: FtSwarmXMMotor
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        The XM motor is very powerful, since it has an included gear. Since fischertechnik XM motors need up to 950mA, please check your power budget of 2A.
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/motor/motor-xm.png">XM Motor (135485)</div>
</div>

Please check out the details about speed, coast & brake in the [motor overview](../motors/), too.

#### FtSwarmXMMotor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmXMMotor object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_M1** or **FTSWARM_M2**

#### FtSwarmXMMotor( const char *name )

Constructor to create a FtSwarmXMMotor object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void setSpeed( int16_t speed )

Sets the motors speed.

- speed: speed value in the range from -256 to 256.

#### uint16_t getSpeed()

Returns the motor's speed.

#### void setMotionType( FtSwarmMotion_t motionType )

This function sets the motor driver's operating mode:
- **FTSWARM_COAST** the motor is stopped but can be adjusted manually.
- **FTSWARM_BRAKE** the motor is stopped but actively braked. The motor cannot be adjusted manually.
- **FTSWARM_ON** switches the power stage on again at the original speed.

#### FtSwarmMotion_t getMotionType()

Returns the motor's motion type;

#### void coast( void )

Set the motor's motion type to **FTSWARM_COAST**.

#### void brake( void )

Set the motor's motion type to **FTSWARM_BRAKE**.