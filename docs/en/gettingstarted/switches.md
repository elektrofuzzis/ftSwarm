---
title: Switches & Buttons
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---

There is no big difference between buttons and switches. Buttons are built-in switches at ftSwarmControl. Switches are externally connected to the six independent input channels (A1..A6) and have more functionality.

### Normally closed & normally open

*fischertechnik* mini switches have 3 pins. In a standard configuration, use pin 1 & 3. The switch "normally open" and closes pressing the button. Using pin 1 & 2 the switch is normally closed and opens by pressing the button. This configuration is useful for e.g. emergency stops - a broken cable will activate the emergency stop as well.

```cpp
normallyOpen   = FtSwarmSwitch( 4711, FTSWARM_A1 );
normallyClosed = FtSwarmSwitch( 4711, FTSWARM_A2, false );
```

### Toggles

In many applications your're not interested in the state of a switch, you want to know if the switch has changed it's state. You could wait actively using

```cpp
while (switch.isReleased());
```

This works fine. But you block a core and it's a little bit difficult to do other things while waiting. Since the firmware polls regulary all inputs states in the background, it notices status changes, too.

```cpp
void loop() {

  if (switch.hasToggledUp()) motor.setSpeed(100);
  if (switch.hasToggledDown()) motor.setSpeed(0);
  
  // use the time and do someting else
```

is smarter, because you're not blocking your application.

Keep in mind, every call of a *Toggle* command resets the toggle.

Avoid two Toggle-Commands in an if condition.

```cpp
if ( (switch.getToggle() != FTSWARM_NOTOGGLE) && (switch.hasToggeledUp()) ) ...
```

is always false. Even if **switch.getToggle()** returns **FTSWARMTOGGLEUP** or **FTSWARMTOGGLEDOWN**, the internal toggle state is reseted to **FTSWARM_NOTOGGLE**. So **switch.hasToggeledUp()** will be always **false**.
