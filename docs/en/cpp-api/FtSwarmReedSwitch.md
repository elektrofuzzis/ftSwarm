---
title: FtSwarmReedSwitch
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        Reed contacts respond on dtrong magnetic fields.
        Contactless switches can be built with the magnet building block from fischertechnik or the neodymium magnets from fischerfriendsman.
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/switches/switch-reed-api.png">Reed contact (36120)</div>
</div>

Reed contacts are connected to both terminals of the input. A 9V power supply is not required.

#### FtSwarmReedSwitch(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true)

Constructor to create a FtSwarmReedSwitch object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**
- normallyOpen (optional): **true**, a magnet at the reed contact will trigger the value 1. With **false** the reed contact without magnet delivers the value 1.

#### FtSwarmReedSwitch(const char *name, bool normallyOpen = true)

Constructor to create a FtSwarmReedSwitch object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.
- normallyOpen (optional): **true**, a magnet at the reed contact will trigger the value 1. With **false** the reed contact without magnet delivers the value 1.

#### bool isPressed()

TRUE, if the magnet activates the reed contact.

#### bool isReleased()

TRUE, if the magnet doesn't activate the reed contact.

#### bool hasToggledUp()

TRUE if the magnet has triggered the reed contact since the last toggle call.

#### bool hasToggledDown()

TRUE if the magnet has released the reed contact since the last toggle call.

#### bool getState()

TRUE, if the magnet activates the reed contact.

#### FtSwarmToggle_t getToggle()

The function returns:
- **FTSWARM_TOGGLEDOWN** if the magnet has released the reed contact since the last toggle call.
- **FTSWARM_TOGGLEUP** if the magnet has activated the reed contact since the last toggle call.
- **FTSWARM_NOTOGGLE**, if the magnet has not changed the reed contact since the last toggle call.