---
title: FtSwarmVotmeter
parent: Analog Sensors
grand_parent: Writing C++ Applications
nav_order: 2
---

<div class="ftimgdetail"> <img src="../../../../assets/img/analog/voltmeter.png"><div>Image source: fischertechnik</div></div>

## FtSwarmVotmeter

Voltage measurement can be done using input `A2` on the ftSwarm. ftSwarmControl doesn't support voltage measurements.

Measuring range of the ohmmeter is 0V to 4V.

BETA
{: .label .label-purple .float-right}
### API Reference

#### FtSwarmVotmeter(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmVotmeter object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_A2`

#### FtSwarmVotmeter(const char *name)

Constructor to create a FtSwarmOhmmeter object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

####  float getVoltage()

Returns the measured voltage.

#### int32_t getValue()

Returns the the raw reading of the ADC.