---
title: FtSwarmJoystick
parent: Analog Sensors
grand_parent: Writing C++ Applications
nav_order: 1
---

<div class="ftimgdetail"> <img src="../../../assets/img/analog/joystick.png"><div>Image source: fischertechnik</div></div>

## FtSwarmJoystick

fTSwarmControl has two builtin joysticks.

### API Reference

#### FtSwarmJoystick(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmJoystick object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_JOY1` or `FTSWARM_JOY2`. f

#### FtSwarmJoystick(const char *name)

Constructor to create a FtSwarmJoystick object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### int16_t getFB()

Gets the joystick's forward/backward reading. 

#### int16_t getLR()

Gets the joystick's left/right reading. 

#### bool getButtonState()

Gets the joystick's button sate. 

#### void getValue( int16_t *FB, int16_t *LR, bool *buttonState )

Get all jostick parameters in in statement.
