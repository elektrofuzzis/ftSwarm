---
title: A first Swarm
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---

[Your First Application](../MotorSwitch) was nice to show how a ftSwarm application is coded. But it was just a simple "one-controller-application".

Now it's time to use the swarm. The application's idea is the same - control a motor by a switch. But in this application, switch and motor
are connected to different controllers.

1st Controller:
- Connect a switch to input A1.
- Connect a 9V power supply to PWR.

2nd Controller:
- Connect a motor or a lamp to output M2 of your ftSwarm.
- Connect a 9V power supply to PWR.


### Build a swarm

To run this program at the first controller, both ftSwarm devices need to be connected in a swarm. The second controller runs the standard firmware.

Compile and Upload **Examples\ftSwarm\firmware** on the first device

Start a putty or arduino monitor and reset the device to get some output. Press any key to get into the firmware configuration menu:

```
ftSwarmOS 0.5.0

(C) Christian Bergschneider & Stefan Fuss

Main Menu

(1) wifi settings
(2) webserver settings
(3) swarm settings
(4) alias names
(5) factory settings

(0) exit
main>
```

wifi settings are already done, so we could focus on the swarm settings. Use **(3)** to get into the swarm menu:

```
swarm menu

This device is connected to swarm "ftSwarm1" with 1 member(s) online.
Swarm PIN is 1.
(1) swarm communication: wifi
(2) create a new swarm
(3) join another swarm
(4) list swarm members

(0) main
swarm>
```
If you like, you could set a swarm name and pin. But you could use the default settings as well (swarm name ftSwarm1 with Pin 1 in my example above).

Keep these settings and connect the second controller to your PC. Connect this controller to your local wifi, too. Now your could join the swarm using **(3) join another swarm**. You're asked for the swarm's name and pincode.

### Run a swarm based program

If both controllers are connected to the same swarm, you could use "Examples\ftSwarm\MotorSwitchSwarm":

```cpp
#include <ftSwarm.h>

// serial number of the second controller - change it to your 2nd device serial number
#define remote 2

FtSwarmSwitch *sw;
FtSwarmMotor  *mot;

void setup( ) {

  Serial.begin(115200);

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
	
  // get switch and motor instances
  sw  = new FtSwarmSwitch( local, FTSWARM_A1 );
  mot = new FtSwarmMotor( remote, FTSWARM_M2 );

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

Basically, the application is the same as [Your First Application](../MotorSwitch). There are only two changes:

- **#define remote 2** sets the serial number of your 2nd device. Please change the serial number to your 2nd device serial number.
- **FtSwarmMotor( remote, FTSWARM_M2 );** now uses the remote device serial number instead of the local serial number.

At the monitor page of your first device, you will now see both controllers.

### What happens, if the 2nd controller isn't online?

Start the serial monitor and unplug the 9V power supply from the second device. Restart the first one. 
With **FtSwarmMotor( remote, FTSWARM_M2 );** you will get the debug output **Waiting on device**. Both RGB Leds of this controller will be blue.
The firmware waits for the motor IO-pin joining the swarm.
Add the 9V power supply again. Once the second device is started, your application will continue.

This feature helps you starting a swarm based roboter. You don't need to think about the timing or sequence of starting your devices.
