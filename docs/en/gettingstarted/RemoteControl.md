---
title: Remote control without a line of code
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---
Using this feature, you're able to use your ftSwarmControl as a remote control, without writing any code.

This feature is only available on ftSwarmControl. If you don't have one, please read the following explanations - you will need them in our next example about events.

The idea of the remote control is quite easy. The sensor - input, button or joystick - is read every 25ms. So the firmware could detect signal changes. Based on a signal change, the firmware could invoke an actor - motor, servo or LED.

You could set three types of triggers:
- **TriggerUp:** This event triggers, if the signal of a digital sensor is changing from low to high. E.g. by pressing a button or switch.
- **TriggerDown:** This event is the composite of TriggerUp. It raises when a switch/button is released.
- **ChangeValue:** This event is raised, whenever the signal changes. It's intended to be used with the joysticks.

Every trigger invokes an action. This is the motor speed, the servo's position or the LED color. You could apply a static value to a trigger event or you could use the sensor's actual reading.

The hardware setup is based on a ftSwarmControl and a ftSwarm:

ftSwarmControl:
- Runs standard firmware.
- Connect a 9V power supply to PWR.
- Set 1st joystick's alias name to "joystick". 
- Set Button S1 alias name to "button".

ftSwarm:
- Runs standard firmware.
- Connect a motor to M1.
- Connect a lamp to M2.
- Connect a 9V power supply to PWR.
- Set M1's alias name to "motor" 
- Set M2's alias name to "lamp" 

On the ftSwarmControl, the button should now switch the lamp on and off. The joystick controls the motor. For this purpose, the following events must be set in **Remote Control**:

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

Now you can test your remote control. Pressing the button controls the lamp on.Moving the joystick left and right, the motor will turn.

The settings are stored in the NVS of your ftSwarmControl. During reboot, the ftSwarmControl will try to connect to the referenced sensors and actors. If you are using sensors/actors of other swarm devices, the firmware will wait until they are online as well.
