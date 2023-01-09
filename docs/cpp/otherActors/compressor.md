---
title: FtSwarmCompressor
parent: Other Actors
grand_parent: Writing C++ Applications
nav_order: 2
---
<div class="ftimgdetail"> <img src="../../../assets/img/otherActors/compressor.png"><div>Image source: fischertechnik</div></div>

## FtSwarmCompressor

Using a *fischertechnik* compressor you could generate air to control a pneumatic cylinder. 

### API Reference

#### FtSwarmCompressor(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmCompressor object. If the referenced controller isn't conected to the swarm, the firmware will wait until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_M1` or `FTSWARM_M2`

#### FtSwarmCompressor( const char *name )

Constructor to create a FtSwarmCompressor object. If the referenced controller isn't conected to the swarm, the firmware will wait until the controller gets online.

- name: Alias name of the IO port.

#### void on( void )

Set compressor on. 

#### void off( void )

Set compressor off. 
