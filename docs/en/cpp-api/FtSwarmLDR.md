---
title: FtSwarmLDR
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        Phototransistors and resistors measure light intensity. In many applications, they are used as light barriers. Since they are affected by ambient light, the two types should always be protected from stray light with a tube cap.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/analog/ldr-api.png">Resistor (32698) and Transistor (36134)</div>
</div>

The phototransistor is working with certain wavelengths only. Therefore, normal *fischertechnik*-LEDs won't work as a light source. Lens lamps and LEDs specified for light barriers are best suited.

When connecting the phototransistor, the correct polarity must be observed. The red marked pin must be connected to the red cable at ftSwarm/ftSwarmControl or the input pin at ftSwarmRS.

**ftSwarm**: Phototransistors and resistors can be connected to all inputs on the ftSwarm.<br>
**ftSwarmControl**: Here, analog values can only be read out at input **A1**.
{: .notice--info}

#### FtSwarmLDR(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmLDR object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**. ftSwarmControl supports **FTSWARM_A1** only.

#### FtSwarmLDR(const char *name)

Constructor to create a FtSwarmLDR object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### int32_t getValue()

Returns the the raw reading of the LDR. The lower the measured value is, the higher the light intensity is.