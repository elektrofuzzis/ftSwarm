---
title: Alias Names
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---

Up to now, the IO's were identified by the serial number and the name of the port. This is nice in a small setup, but it could be confusing in a bigger robot. It would be much easier to identify the IO based on it's function:

```cpp
  SwitchWithSN     = new FtSwarmSwitch( 4711, FTSWARM_A1 );  // Old style: with serial number and port
  SwitchWithAlias  = new FtSwarmSwitch( "MotorEndStop" );    // New style: call my alias name!
```

It's the same hardware setup as used before.
 
1st Controller:
- Connect a switch to input A1.
- Connect a 9V power supply to PWR.

2nd Controller:
- Connect a motor or a lamp to output M2 of your ftSwarm.
- Connect a 9V power supply to PWR.


First, you need to set the alias names for switch and motor. Therefore you need to upload the standard firmware and enter the firmware menu. Use **(4) alias names** in main menu:

```
alias controler menu:

( 1) hostname ftSwarm100 - 
( 2) A1   - switch                                 
( 3) A2   -                                 
( 4) A3   -                                 
( 5) A4   -                                 
( 6) A5   -                                 
( 7) A6   -                                 
( 8) M1   -                                 
( 9) M2   -                                 
(10) LED1 -                                 
(11) LED2 -                                 
(12) SERVO -                                 

(0) exit
alias>
```

Set `A1 - switch` at the 1st controller running the example program and `M2 - motor` at the 2nd controller. Use `Example/MotorSwitchAlias`:

```cpp
#include <ftSwarm.h>

FtSwarmSwitch *sw;
FtSwarmMotor  *mot;

void setup( ) {

  Serial.begin(115200);

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
  
  // get switch and motor instances
  sw  = new FtSwarmSwitch( "switch" );
  mot = new FtSwarmMotor( "motor" );

}

void loop( ) {

  // check if switch is pressed or released
  if ( sw->isPressed() )
    mot->setSpeed(255);
  else
    mot->setSpeed(0);
  
  // wait some time
  delay(100);

}
```

Let's look at the monitor page of your swarm. The IO's show their alias names as well.

Finally, modify your hardware setup. Move your motor to port **M2** of your first device. Set the alias **motor** at your first device. Start the application again. It's working without any changes!

But keep in mind, an alias name needs to be unique in your swarm! In this example it doesn't matter to define **motor** twice. 
The firmware always checks the local device alias names first. But using multiple remote devices, the result depends on the boot sequence of the remote devices.
{: .notice--info}

