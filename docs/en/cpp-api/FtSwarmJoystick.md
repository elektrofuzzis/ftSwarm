---
title: FtSwarmJoystick
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        ftSwamControl has two built-in Joysticks.<br><br>
        Reading the joystick's position, movements left and forward are represented as negative values, right and back movements gets positive values.
    </div>
    <div class="apiright apiimg"><img title="image source: Bildnachweis: fischertechnik" src="/assets/img/analog/joystick.png">ftSwarmControl</div>
</div>

In rest postion, you will get 0/0. In the firmware is an option to calibrate the rest position.

#### FtSwarmJoystick(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmJoystick object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_JOY1** or **FTSWARM_JOY2**.

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