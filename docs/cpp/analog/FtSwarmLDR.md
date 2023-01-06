---
title: FtSwarmLDR
parent: Analog Sensors
grand_parent: Writing C++ Applications
nav_order: 1
---

<div class="ftimgdetail"> <img src="../../../assets/img/analog/ldr.png"><div>Image source: fischertechnik</div></div>

## FtSwarmLDR

Photo transistors and resistors could be connected to all inputs `A1`.. `A4`. At the ftSwarmControl use input `A1`.

The red marked pin of the photo transistor needs to be connected to the inputs red cable.

### API Reference

#### FtSwarmLDR(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmLDR object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_A1`, `FTSWARM_A2`, `FTSWARM_A3` or `FTSWARM_A4`. ftSwarmControl supports `FTSWARM_A1` only.

#### FtSwarmLDR(const char *name)

Constructor to create a FtSwarmLDR object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### int32_t getValue()

Returns the raw reading of the LDR. The lower the measured value is, the higher the light intensity is.
