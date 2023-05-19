---
title: FtSwarmCompressor
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        Using a fischertechnik compressor you could generate air to control some pneumatic cylinders. To control the cylinders use a <a href="../valve/">valve</a> and add some airtanks to cover air flow variations. 
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/otherActors/compressor.png">Compressor (121470)</div>
</div>

#### FtSwarmCompressor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmCompressor object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: **FTSWARM_M1** or **FTSWARM_M2**

#### FtSwarmCompressor( const char *name )

Constructor to create a FtSwarmCompressor object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void on( void )

Set compressor on.

#### void off( void )

Set compressor off.