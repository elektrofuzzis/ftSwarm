---
title: FtSwarmXMMotor
parent: Motor Types
grand_parent: Writing C++ Applications
nav_order: 2
---
<div class="ftimgdetail"> <img src="../../../assets/img/motor/motor-xm.png"><div>Image source: fischertechnik</div></div>

## FtSwarmXMMotor

`setSpeed(speed)` is used to start & stop your motor. A positive speed value will run the motor clockwise, a negative one will run the motor counterclockwise. 
Depenend on you mechanical load, the motor needs a minimum speed to turn. 

`setSpeed(0)` will stop the motor. The output will be released and you could turn the motor manualy. 

`getSpeed()` returns the actual motor speed.

Stopping your motor, you could distiguish between coast and brake mode. 

- `coast` will release you motor output. The motor could be turned manually. Your model might not stay in place.
- `brake` will stop the motor, but the output is not released. You couldn't turn the manually and model will keep in place.

*Since fischertechnik XM motors need up to 950mA, please check your power budget of 2A.*

### API Reference

#### FtSwarmXMMotor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmXMMotor object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_M1` or `FTSWARM_M2`

#### FtSwarmXMMotor( const char *name )

Constructor to create a FtSwarmXMMotor object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- name: Alias name of the IO port.

#### void setSpeed( int16_t speed )

Sets the motors speed.

- speed: speed value in the range from -256 to 256

#### uint16_t getSpeed()

Returns the motor's speed.

#### void setMotionType( FtSwarmMotion_t motionType )

Set the motor's motion type. Setting `FTSWARM_COAST` or `FTSWARM_BRAKE` stops the motor, but `getSpeed` will return the last speed value. Setting `FTSWARM_OM` again,
the motor will run with the old speed value.

- motionType: `FTSWARM_COAST`, `FTSWARM_BRAKE` or `FTSWARM_ON`.

#### FtSwarmMotion_t getMotionType()

Returns the motor's motion type;

#### void coast( void )

Set the motor's motion type to `FTSWARM_COAST`.

#### void brake( void )

Set the motor's motion type to `FTSWARM_BRAKE`.
