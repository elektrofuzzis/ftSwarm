---
title: FtSwarmButton
parent: Buttons & Switches
grand_parent: Writing C++ Applications
nav_order: 2
---
<div class="ftimgdetail"> <img src="../../../assets/img/switches/ftSwarmControl.png"></div>

## FtSwarmButton

ftSwarmControl has eight builtin buttons: 
- Four standard buttons on top between reset and power buttons. 
- One in each joystick.
- Two front side buttons.

Use this class for builtin buttons only.

ftSwarmControl
{: .label .label-blue .float-right}
### API Reference

#### FtSwarmButton(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmButton object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Use `FTSWARM_S1`, `FTSWARM_S2`, `FTSWARM_S3`, `FTSWARM_S4` for standard buttons, `FTSWARM_J1` & `FTSWARM_J2` for joystick buttons 
  and `FTSWARM_F1` & `FTSWARM_F2` for frontside buttons.
  
#### FtSwarmButton(const char *name)

Constructor to create a FtSwarmButton object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

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