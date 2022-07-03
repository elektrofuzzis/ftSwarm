---
title: Use The Swarm
parent: Your First Application
grand_parent: Writing C++ Applications
nav_order: 1
---

## Use The Swarm

[Your First Application](../MotorSwitch) was nice to show how a ftSwarm application is coded. But it was just a simple "one-controller-application".
Now it's time to use the swarm. The application's idea is the same - control a motor by a switch. But in this application, switch and motor
are connected to different controllers:

- Use the setup of [Your First Application](../MotorSwitch).
- Add a second ftSwarm or ftSwarmControl device.
- Connect a 9V power supply to your second device.
- Move the motor to M2 of the new device.

*To run this program at the first controller, both ftSwarm devices need to be connected in a swarm. The second controller runs the standard firmware.*

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

- `#define remote=2` sets the serial number of your 2nd device. Please change the serial number to your 2nd device serial number.
- `FtSwarmMotor( remote, FTSWARM_M2 );` now uses the remote device serial number instead of the local serial number.

At the monitor page of your first device, you will now see both controllers.

Start the serial monitor and unplug the 9V power suplly from the second device. Restart the first one. 
With `FtSwarmMotor( remote, FTSWARM_M2 );` you will get the debug output `Waiting on device`. The firmware waits for the motor IO-pin joining the swarm.
Add the 9V power supply again. Once the second device is started, your application will continue.

This feature helps you starting a swarm based roboter. You don't need to think about the timing or sequence of starting your devices.

<br>
[Let's use alias names](../MotorSwitchAlias){: .btn .float-right }
<br>