---
title: FtSwarmValve
parent: Other Actors
grand_parent: Writing C++ Applications
nav_order: 4
---
<div class="ftimgdetail"> <img src="../../../assets/img/otherActors/valve.png"><div>Image source: fischertechnik</div></div>

## FtSwarmValve

Using a *fischertechnik* valve you could control a pneumatic cylinder. 

### API Reference

#### FtSwarmValve(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmValve object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_M1` or `FTSWARM_M2`

#### FtSwarmValve( const char *name )

Constructor to create a FtSwarmValve object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void on( void )

Set valve on. Your pneumatic cylinder will move.

#### void off( void )

Set valve off. Your pneumatic cylinder will move back.