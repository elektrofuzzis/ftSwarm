---
title: FtSwarmOhmmeter
parent: Analog Sensors
grand_parent: Writing C++ Applications
nav_order: 2
---

<div class="ftimgdetail"> <img src="../../../assets/img/analog/poti.png"><div>Image source: fischertechnik</div></div>

## FtSwarmOhmmeter

Resistors could be connected to all inputs `A1`.. `A4` of ftSwarm. At the ftSwarmControl use input `A1`.

The measuring range of the ohmmeter is 0 to 10 kOhm.

BETA
{: .label .label-purple .float-right}
### API Reference

#### FtSwarmOhmmeter(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmOhmmeter object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_A1`, `FTSWARM_A2`, `FTSWARM_A3` or `FTSWARM_A4`. ftSwarmControl supports `FTSWARM_A1` only.

#### FtSwarmOhmmeter(const char *name)

Constructor to create a FtSwarmOhmmeter object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

####  float getResistance()

Returns the measured resistance in Ohm.

#### int32_t getValue()

Returns the raw reading of the ADC.
