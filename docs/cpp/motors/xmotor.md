---
title: FtSwarmMotor
parent: Motor Types
grand_parent: Writing C++ Applications
nav_order: 1
---
<div class="ftimgdetail"> <img src="../../../assets/img/motor/kombi.png"><div>Image source: fischertechnik</div></div>

## FtSwarmMotor

Use FtSwarmMotor with all kind of old grey motors, mini motors and XS motors.

`setSpeed(speed)` is used to start & stop your motor. A positive speed value will run the motor clockwise, a negative one will run the motor counterclockwise. 
Dependent on the mechanical load, the motor needs a minimum speed to turn. 

`setSpeed(0)` will stop the motor. The output will be released and you could turn the motor manualy. 

`getSpeed()` returns the actual motor speed.

### API Reference

#### FtSwarmMotor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmMotor object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_M1` or `FTSWARM_M2`

#### FtSwarmMotor( const char *name )

Constructor to create a FtSwarmMotor object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- name: Alias name of the IO port.

#### void setSpeed( int16_t speed )

Sets the motors speed.

- speed: speed value in the range from -256 to 256

#### uint16_t getSpeed()

Returns the motor's speed.
