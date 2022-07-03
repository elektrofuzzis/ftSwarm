---
title: Event Controlled Programming
parent: Your First Application
grand_parent: Writing C++ Applications
nav_order: 3
---

## Event Controlled Programming

Many applications are quite simple. A switch controls a motor. You could code it easily - create switch and motor in `setup` and check the switch state in `loop`.
But why should you code this in detail? The firmware event controlled programming option will minimize your coding.

Every sensor - input, button or joystick - is read every 25ms. So the firmware could detect signal changes.
Based on a signal change, the firmware could invoke an actor - motor, servo or LED.

You could apply three types of triggers to a sensor:
- *TriggerUp:* This event triggers, if the signal of a digital sensor is changing from low to high. E.g. by pressing a button or switch.
- *TriggerDown:* This event is the composite of TriggerUp. It raises when a switch/button is released.
- *ChangeValue:* This event is raised, whenever the signal changes. It's intened to be used with the joysticks.

Every trigger invokes an action. This is the motor speed, the servo's position or the LED color. You could apply a static value to a trigger event or you could use 
the sensor's actual reading.

```cpp
void setup() {
  
  // start swarm
  ftSwarm.begin();
  
  // define needed IOs
  FtSwarmSwitch   switch   = new FtSwarmSwitch( "switch" );
  FtSwarmJoystick joystick = new FtSwarmJoystick( "joystick" );
  FtSwarmXMotor   motor    = new FtSwarmXMotor( "motor" );
  FtSwarmLamp     lamp     = new FtSwarmLamp( "lamp" );
  
  // switch controls the lamp
  switch.onTrigger( FTSWARM_TRIGGERUP, lamp, 255 );
  switch.onTrigger( FTSWARM_TRIGGERDOWN, lamp, 0 );
  
  // joystick uses it's readings to set the motor's speed
  joystick.onTriggerLR( FTSWARM_TRIGGERVALUE, motor );
  
}

void loop() {
}
```

<br>
[Explore the different motor types](../../motors/motors){: .btn .float-right }
<br>