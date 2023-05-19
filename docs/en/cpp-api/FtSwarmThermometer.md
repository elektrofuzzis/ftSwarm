---
title: FtSwarmThermometer
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        There are different NTC resistors from fischertechnik. This class supports the currently available NTC with 1.5kOhm or any other NTC with the same resistance value and R/T curve 1023.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/analog/sensor-ntc-api.png">NTC resistor 1.5 kΩ (36437)</div>
</div>

**ftSwarm**: NTC resistors can be connected to all inputs on the ftSwarm.<br>
**ftSwarmControl**: Here, analog values can only be read out at input **A1**.
{: .notice--info}

#### FtSwarmThermometer(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmThermometer object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_A1`, `FTSWARM_A2`, ..., `FTSWARM_A6`. ftSwarmControl supports `FTSWARM_A1` only.

#### FtSwarmThermometer(const char *name)

Constructor to create a FtSwarmThermometer object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

####  float Celcius()

Returns the measured resistance in °Celcius.

####  float Fahrenheit()

Returns the measured resistance in °Fahrenheit.

####  float Kelvin()

Returns the measured resistance in °Kelvin.

#### int32_t getValue()

Returns the the raw reading of the ADC.