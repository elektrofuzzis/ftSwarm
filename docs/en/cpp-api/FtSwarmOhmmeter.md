---
title: FtSwarmOhmmeter
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        This class could read the position of the fischertechnik potentiometer or any other resistance. 
        Mit der Klasse FtSwarmOhmmeter können die Postion des fischertechnik-Potentimeters als auch andere Widerstandswerte gemessen werden. The measuring range of the ftSwarmOhmmeter is 0 to 10 kOhm.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/analog/poti.png">Potentiometer (186184)</div>
</div>

The potentiometer has three connections. For measurement of the poti position, either the connection scheme green/black or red/black can be used. With red/green the position cannot be measuredcaptured, it returns the complete resistance of the potentiometer.

**ftSwarm**: Resistors can be connected to all inputs on the ftSwarm.<br>
**ftSwarmControl**: Here, analog values can only be read out at input **A1**.
{: .notice--info}

#### FtSwarmOhmmeter(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmOhmmeter object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_A1`, `FTSWARM_A2`, ..., `FTSWARM_A6`. ftSwarmControl supports `FTSWARM_A1` only.

#### FtSwarmOhmmeter(const char *name)

Constructor to create a FtSwarmOhmmeter object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

####  float getResistance()

Returns the measured resistance in Ohm.

#### int32_t getValue()

Returns the the raw reading of the ADC.