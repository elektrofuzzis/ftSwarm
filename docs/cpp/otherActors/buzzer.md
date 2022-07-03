---
title: FtSwarmBuzzer
parent: Other Actors
grand_parent: Writing C++ Applications
nav_order: 1
---
<div class="ftimgdetail"> <img src="../../../assets/img/otherActors/buzzer.png"><div>Image source: fischertechnik</div></div>

## FtSwarmBuzzer

### API Reference

#### FtSwarmBuzzer(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmBuzzer object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_M1` or `FTSWARM_M2`

#### FtSwarmBuzzer( const char *name )

Constructor to create a FtSwarmBuzzer object. If the referenced controller isn't conected to the swarm, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void on( void )

Set buzzer on.

#### void off( void )

Set buzzer off.