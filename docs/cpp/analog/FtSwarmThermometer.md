---
title: FtSwarmThermometer
parent: Analog Sensors
grand_parent: Writing C++ Applications
nav_order: 2
---

<div class="ftimgdetail"> <img src="../../../assets/img/analog/sensor-ntc.png"><div>Image source: fischertechnik</div></div>

## FtSwarmThermometer

*fishertechnik* NTC 1.5kOhm sensors could be connected to all inputs `A1`.. `A4` of ftSwarm.
At the ftSwarmControl use input `A1`.

You could use industrie standard NTCs as well:
- R25: 1.5kOhm
- R/T-curve: No. 1013

BETA
{: .label .label-purple .float-right}
### API Reference

#### FtSwarmThermometer(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmThermometer object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_A1`, `FTSWARM_A2`, `FTSWARM_A3` or `FTSWARM_A4`. ftSwarmControl supports `FTSWARM_A1` only.

#### FtSwarmThermometer(const char *name)

Constructor to create a FtSwarmThermometer object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- name: Alias name of the IO port.

####  float Celcius()

Returns the measured resistance in °Celcius.

####  float Fahrenheit()

Returns the measured resistance in °Fahrenheit.

####  float Kelvin()

Returns the measured resistance in °Kelvin.

#### int32_t getValue()

Returns the raw reading of the ADC.
