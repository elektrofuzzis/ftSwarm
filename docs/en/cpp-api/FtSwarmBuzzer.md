---
title: FtSwarmBuzzer
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        The buzzer from fischertechnik is rarely used, as it can only reproduce one signal. There is no information from fischertechnik on power consumption available.  
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/otherActors/buzzer.png">Buzzer (36119)</div>
</div>

#### FtSwarmBuzzer(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmBuzzer object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_M1** or **FTSWARM_M2**

#### FtSwarmBuzzer( const char *name )

Constructor to create a FtSwarmBuzzer object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void on( void )

Set buzzer on.

#### void off( void )

Set buzzer off.