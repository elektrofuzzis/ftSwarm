---
title: FtSwarmVotmeter
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        The ftSwarm can measure an externally applied voltage at the <stong>A2</stong> input.<br>The measuring range of the FtSwarmVoltmeter is 0 to 4 volts.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: elektrofuzzis" src="/assets/img/analog/voltmeter-api.png">Spannung</div>
</div>

All other analog inputs or measuring classes determine the resistance connected to the input.

Voltage measurement is only possible at input **A1** of the **ftSwarm**.
{: .notice--info}

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