---
title: Use Events
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---

Many applications are quite simple. A switch controls a motor. You could code it easily - create switch and motor in **setup** and check the switch state in **loop**. But why should you code this in detail? The firmware event controlled programming feature will minimize your coding.

The hardware setup is the same as in "Remote Control":

ftSwarmControl:
- Connect a 9V power supply to PWR.
- Set 1st joystick's alias name to "joystick" 
- Set Button S1 alias name to "button" 

ftSwarm:
- Connect a motor to M1
- Connect a lamp to M2
- Connect a 9V power supply to PWR.
- Set M1's alias name to "motor" 
- Set M2's alias name to "lamp" 

If needed, please delete all events in "Remote Control".

Use the following example code **EventControlled**:

```cpp
void setup() {
  
  // start swarm
  ftSwarm.begin();
  
  // define needed IOs
  FtSwarmButton   button   = new FtSwarmButton( "button" );
  FtSwarmJoystick joystick = new FtSwarmJoystick( "joystick" );
  FtSwarmXMotor   motor    = new FtSwarmXMotor( "motor" );
  FtSwarmLamp     lamp     = new FtSwarmLamp( "lamp" );
  
  // switch controls the lamp
  button.onTrigger( FTSWARM_TRIGGERUP, lamp, 255 );
  button.onTrigger( FTSWARM_TRIGGERDOWN, lamp, 0 );
  
  // joystick uses it's readings to set the motor's speed
  joystick.onTriggerLR( FTSWARM_TRIGGERVALUE, motor );
  
}

void loop() {
}
```

At a first glance, the **loop** function is surprising. There is no code in it! But it has the same functionality as "Remote Control".

Let's check the **setup** code:

- First, there are some IO definitions using alias names.
- In the second section, we define some triggers on the button. If it is pressed (TRIGGERUP), the lamp gets on. Releasing the button (TRIGGERDOWN) turn the lamp off.
- The third section uses the value of the left/right-movement of the joystick. If the value changes, it's used to set the motor's speed. Moving the joystick to the left, the sensor data is negative and the motor moves clock counterwise. Changing the joystick to the right, the sensor data is positive and the motor moves counterwise. In the mid position the motor is turned off.