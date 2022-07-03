---
title: Your First Application
parent: Writing C++ Applications
nav_order: 2
has_children: yes
has_toc: false
---
## Your First Application

The first application is not about writing "Hello World" on a display. It's just to control a motor with a simple switch. 
Therefore you need a ftSwarm, a switch, a motor and a 9V power supply.

The hardware setup is easy:

- Connect a motor or a lamp to output M2 of your ftSwarm.
- Connect a switch to input A1.
- Connect a 9V power supply to PWR.
- Connect the ftSwarm via USB cable with your computer.

Write the following code or use the example file *MotorSwitch*. Upload it to your device. Whenever you press your switch, the motor starts running. If you release the switch, the motor stops.

```cpp
#include <ftSwarm.h>

FtSwarmSwitch *sw;
FtSwarmMotor  *mot;

void setup( ) {

  Serial.begin(115200);

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
	
  // get switch and motor instances
  sw  = new FtSwarmSwitch( local, FTSWARM_A1 );
  mot = new FtSwarmMotor( local, FTSWARM_M2 );

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

The application is quite easy to understand. `#include "ftSwarm.h">` includes the ftSwarm-Library to access the ftSwarm firmware. 
Afterwards two pointers for `switch` and `motor` are defined. In setup routine your swarm starts with 

```cpp
Serial.begin(115200);

// start the swarm
FtSwarmSerialNumber local = ftSwarm.begin( );
```

You need to initialize the serial communication since to send some startup and error messages if needed. 

The result is the serial number of your local controller. With this serial number, you could now instantiate switch and motor:

```cpp
sw  = new FtSwarmSwitch( local, FTSWARM_A1 );
mot = new FtSwarmMotor( local, FTSWARM_M2 );
```

Due to the architecture of the firmware, you could not access an IO before your swarm is started.
With instantiating an IO, the firmware waits until the IO joins the swarm. If the swarm is not started, your apllication will wait forever. 
To work with global objects, they need to be defined as pointers. So you are able to instantiate them after starting your swarm.

The main loop is just about querying the switch state and starting/stopping the motor:

```cpp
if ( sw->isPressed() )
  mot->setSpeed(255);
else
  mot->setSpeed(0);
```

Let's have a look on the swarm monitor. If you already added your ftSwarm device to your local wifi, you just need to access 
`http:\\ftSwarm<SerialNumber>` with your browser. Replace <SerialNumber> with your ftSwarm's serial number. If you press the switch,
you will see the status changes at the ftSwarm's monitor page:

![Monitoring ftSwarm](../../assets/img/ftSwarm_Monitor.png)

*Since the monitor page is written in vue.js, you need to enable script in your browser*

<br>
[Let's use the swarm](../MotorSwitchSwarm){: .btn .float-right }
<br>