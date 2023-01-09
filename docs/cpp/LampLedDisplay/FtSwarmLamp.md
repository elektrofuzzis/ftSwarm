---
title: FtSwarmLamp
parent: Lamp, Led & Display
grand_parent: Writing C++ Applications
nav_order: 1
---
<div class="ftimgdetail"> <img src="../../../assets/img/LampLedDisplay/kombi.png"><div>Image source: fischertechnik</div></div>

## ftSwarmLamp

This class is used to connect *fischertechnik* lamps and LEDs to the outputs M1 and M2. Use [FtSwarmLED](../FtSwarmLED) class for ftSwarm's builtin RGB LEDs or additional ftRGBLEDs.

Connect the marked pin of your LED to the red cable.

### API Reference

#### FtSwarmLamp(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmLamp object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_M1` or `FTSWARM_M2`

#### FtSwarmLamp( const char *name )

Constructor to create a FtSwarmLamp object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- name: Alias name of the IO port.

#### void on( uint8_t power = 255 )

This function set the lamp or LED "on". Brightness could be specified optionally..

#### void off( void )

Set the lamp or LED off. Same as `on(0)`.
