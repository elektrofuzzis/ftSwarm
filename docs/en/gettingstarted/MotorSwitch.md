---
title: My First Application
layout: category
land: en
classes: wide
sidebar:
    nav: gettingstarted-en
---
The first application is not about writing "Hello World" on a display. It's just to control a motor with a simple switch. 
Therefore you need a ftSwarm, a switch, a motor or lamp and a 9V power supply.

The hardware setup is easy:

- Connect a motor or a lamp to output M2 of your ftSwarm.
- Connect a switch to input A1.
- Connect a 9V power supply to PWR.
- Connect the ftSwarm via USB cable with your computer.

Write the following sketch or use the example file *MotorSwitch*. Upload it to your device. Whenever you press your switch, the motor starts running. If you release the switch, the motor stops.

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

The Sketch is quite easy to understand.

Like all Arduino-sketches, it uses the main functions **`setup`** and **`loop`**. 

- When the sketch starts, **`setup`** is called automatically. In standard you initialize your hardware in this function.
- Afterwards **`loop`** starts. If the function stops, the sketch calls it again and again. **`loop`** is used to control you model.

 **`#include "ftSwarm.h"`** includes the ftSwarm-Library to access the ftSwarm firmware. You need it to send some debug and error messages if needed. 

```cpp
Serial.begin(115200);
```

Afterwrards the swarm is started:

```cpp
// start the swarm
FtSwarmSerialNumber local = ftSwarm.begin( );
```

The result is the serial number of your local controller. With this serial number, you could now instantiate **`switch`** and **`motor`**:

```cpp
sw  = new FtSwarmSwitch( local, FTSWARM_A1 );
mot = new FtSwarmMotor( local, FTSWARM_M2 );
```

*Attention:* Due to the architecture of the firmware, you could not access an IO before your swarm is started.
With instantiating an IO, the firmware waits until the IO joins the swarm. If the swarm is not started, your application will wait forever. 
To work with global objects, they need to be defined as pointers. So you are able to instantiate them after starting your swarm.

The main loop is just about querying the switch state and starting/stopping the motor:

```cpp
if ( sw->isPressed() )
  mot->setSpeed(255);
else
  mot->setSpeed(0);
```
