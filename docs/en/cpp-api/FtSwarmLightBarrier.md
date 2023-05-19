---
title: FtSwarmLightBarrier
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---

<div class="apicontainer">
    <div class="apileft">
        Light barriers can be built with a phototransistor and a lens lamp. In this class, the phototransistor is used as a digital sensor. If the light from the lens lamp falls on the sensor, the light barrier is open.<br><br>
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/switches/photo-transistor-api.png">Photo Transistor (36134)</div>
</div>

If LEDs are to be used, the LEDs specifically designated for light barrier applications must be used. Since phototransistors are affected by ambient light, they should always be protected from stray light with a tube cap.

When connecting the phototransistor, the correct polarity must be observed. The red marked pin must be connected to the red cable at ftSwarm/ftSwarmControl or the input pin at ftSwarmRS.

**ftSwarm**: Phototransistors and resistors can be connected to all inputs on the ftSwarm.<br>
**ftSwarmControl**: Here, analog values can only be read out at input **A1**.
{: .notice--info}

#### FtSwarmLightBarrier(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true)

Constructor to create a FtSwarmLightBarrier object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**
- normallyOpen (optional): If set **true**, the light barrier will return 0 if it's open. Using **false** it delivers 1 if it's open.

#### FtSwarmLightBarrier(const char *name, bool normallyOpen = true)

Constructor to create a FtSwarmLightBarrier object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.
- normallyOpen (optional): If set **true**, the light barrier will return 0 if it's open. Using **false** it delivers 1 if it's open.

#### bool isPressed()

This function returns true, if the light barrier is interrupted.

#### bool isReleased()

This function returns true, if the light barrier is free.

#### bool hasToggledUp()

This function is TRUE if the light barrier has been interrupted since the last toggle call.

#### bool hasToggledDown()

This function is TRUE if the light barrier has become free since the last toggle call.

#### bool getState()

TRUE, if the light barrier is interrupted.

#### FtSwarmToggle_t getToggle()

The function returns:
- **FTSWARM_TOGGLEDOWN** if the light barrier has been released since the last toggle call.
- **FTSWARM_TOGGLEUP** if the light barrier has been interrupted since the last toggle call.
- **FTSWARM_NOTOGGLE**, if the light barrier has not been changed since the last toggle call.