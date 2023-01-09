---
title: FtSwarmLightBarrier
parent: Buttons & Switches
grand_parent: Writing C++ Applications
nav_order: 3
---

<div class="ftimgdetail"> <img src="../../../assets/img/switches/photo-transistor.png"><div>Image source: fischertechnik</div></div>

## FtSwarmLightBarrier

You could build a light barrier using a lense lamp and a photo transistor. Connect you lamp to a power supply.
The photo transistor could be connected to all inputs `A1`.. `A4`. The red marked pin needs to be connected to the inputs red cable.

Using the FtSwarmLightBarrier class, the photo transistor is used as digital sensor. A high light intensity means the light barrier is open.


### API Reference

#### FtSwarmLightBarrier(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true)

Constructor to create a FtSwarmLightBarrier object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_A1`, `FTSWARM_A2`, `FTSWARM_A3` or `FTSWARM_A4`
- normallyOpen (optional): Set `true`, if your switch is normally open (pins 1-3). Use `false` if your switch is normally closed.

#### FtSwarmLightBarrier(const char *name, bool normallyOpen = true)

Constructor to create a FtSwarmLightBarrier object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- name: Alias name of the IO port.
- normallyOpen (optional): Set `true`, if your switch is normally open (pins 1-3). Use `false` if your switch is normally closed.

#### bool isPressed()

This function returns true, if the switch is actually pressed.

#### bool isReleased()

This function returns true, if the switch is actually released.

#### bool hasToggledUp()

This function is true, if the switch changed from closed to open since your last \*Toggle\*-call.

#### bool hasToggledDown()

This function is true, if the switch changed from open to closed since your last \*Toggle\*-call.

#### bool getState()

This function returns true, if the switch is actually pressed.

#### FtSwarmToggle_t getToggle()

This function returns
- `FTSWARM_TOGGLEDOWN`, if the state of the switch changed from open to closed since your last \*Toggle\*-call.
- `FTSWARM_TOGGLEUP`, if the state of the switch changed from closed to open since your last \*Toggle\*-call.
- `FTSWARM_NOTOGGLE`, if the state of the switch didn't change since your last \*Toggle\*-call.
