---
title: FtSwarmReedSwitch
parent: Buttons & Switches
grand_parent: Writing C++ Applications
nav_order: 4
---
<div class="ftimgdetail"> <img src="../../../assets/img/switches/switch-reed.png"><div>Image source: fischertechnik</div></div>

## FtSwarmReedSwitch

Use this class when using a reed switch.

Like mechanical switches, connect the reed switch to both pins of the input. Using a +9V external power supply won't work.

### API Reference

#### FtSwarmReedSwitch(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true)

Constructor to create a FtSwarmReedSwitch object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_A1`, `FTSWARM_A2`, `FTSWARM_A3` or `FTSWARM_A4`
- normallyOpen (optional): Set `true`, if your switch is normally open (pins 1-3). Use `false` if your switch is normally closed.

#### FtSwarmReedSwitch(const char *name, bool normallyOpen = true)

Constructor to create a FtSwarmReedSwitch object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

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