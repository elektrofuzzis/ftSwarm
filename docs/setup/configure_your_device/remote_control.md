---
title: Remote Control
parent: Configure Your Device
grand_parent: Setup
nav_order: 5
---
## Remote Control

Using this feature, your're able to use yout ftSwarmControl as a remote control, without writing any code.
This feature is only available on ftSwarmControl.

The idea of the remote control is quite easy. The sensor - input, button or joystick - is read every 25ms. So the firmware could detect signal changes.
Based on a signal change, the firmware could invoke an actor - motor, servo or LED.

You could set three types of triggers:
- *TriggerUp:* This event triggers, if the signal of a digital sensor is changing from low to high. E.g. by pressing a button or switch.
- *TriggerDown:* This event is the composite of TriggerUp. It raises when a switch/button is released.
- *ChangeValue:* This event is raised, whenever the signal changes. It's intened to be used with the joysticks.

Every trigger invokes an action. This is the motor speed, the servo's position or the LED color. You could apply a static value to a trigger event or you could use 
the sensor's actual reading.

```
remote control menu:

     sensor             event       actor           value
( 1) switch             TriggerUp   lamp            255
( 2) switch             TriggerDown lamp            0
( 3) joystick        LR ChangeValue motor           SENSORVALUE

(4) add event
(5) delete event

(0) exit
```

Pressing the switch turns the lamp on. Releasing it, the lamp goes out. Moving the joystick left and right, the motor will turn.

To use the remote control feature, you need to do some configuration work:
- [setup your swarm](../swarm) first. 
- Apply [alias names](../alias_names) to all needed sensors and actors.
- Add triggers to your sensors and define the invoked actions.
- Save your changes by exiting the menu.

The settings are stored in the NVS of your ftSwarmControl. During reboot, the ftSwarmControl will try to connect to the refernced sensors and actors.
If you are using sensors/actors of other swarm devices, the firmware will wait until they are online as well.
