---
title: FtSwarmServo
parent: Other Actors
grand_parent: Writing C++ Applications
nav_order: 3
---
<div class="ftimgdetail"> <img src="../../../assets/img/otherActors/motor-servo.png"><div>Image source: fischertechnik</div></div>

## FtSwarmServo

You could add a *fischertechnik* Micro Servo 4.8/6V at the servo plug of your ftSwarm controller. Take care on polarity! The plug provides a 5V max. 500mA power supply, 
which is suitable for most kinds of micro servo motors.

Servo motors use an internal control electronics to align the servo lever. So the commands are about setting the lever's angle.

To simplyfy setting up a servo, we implemented two parameters: postion and offset.
- Offset is used to trim a zero-position of your servo levers. It's the same idea as the trim wheel at the BT Control Set.
- Use position in your application to change the levers angle.

ftSwarm
{: .label .label-blue .float-right}
### API Reference

#### FtSwarmServo(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmServo object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_M1` or `FTSWARM_M2`

#### FtSwarmServo( const char *name )

Constructor to create a FtSwarmServo object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

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
