---
title:  Alias Names
parent: Configure Your Device
grand_parent: Setup
nav_order: 3
---
## Alias Names

The ftSwarm API knows different ways to identify an IO in your code. The simplest method is using serial number an port name. 

```
FtSwarmSwitch doorClosed = new FtSwamSwitch( 10, FTSWARM_A01 );
```

creates a switch instance `doorClosed` using IO-pin A1 on the ftSwarm-device with serial number 10. 
This is easy to understand, but confusing in complex models using multiple ftSwarm devices.

```
FtSwarmSwitch doorClosed = new FtSwamSwitch( "DoorSwitch" );
```

identifies the IO pin by an alias name. When you are building your model, you could set an alias for each IO-pin in the local controller menu.
Afterwards you don't have to think about which actor/sensor uses which physical IO-pin. The swarm's online monitor shows the alias names as well.

*But keep in mind, an alias name needs to be unique in your swarm!*
