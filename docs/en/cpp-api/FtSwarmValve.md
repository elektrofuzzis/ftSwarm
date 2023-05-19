---
title: FtSwarmValve
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        The valve is used to control pneumatic cylinders. Additionally you need a compressor to generate the needed air ans some air tanks to cover air flow variations.
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/otherActors/valve.png">Valve (35327)</div>
</div>

#### FtSwarmValve(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmValve object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_M1** or **FTSWARM_M2**

#### FtSwarmValve( const char *name )

Constructor to create a FtSwarmValve object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void on( void )

Set valve on. Your pneumatic cylinder will move on.

#### void off( void )

Set valve off. Your pneumatic cylinder will move back.