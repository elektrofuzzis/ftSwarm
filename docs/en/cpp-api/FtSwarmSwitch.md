---
title: FtSwarmSwitch
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---

<div class="apicontainer">
    <div class="apileft">
        This class represents all possible switches. These can be mini-buttons, pole-changing switches, but also switches constructed from steel axles.<br><br>
        Depending on the application, the switches are operated "normally open" or "normally closed". 
    </div>
    <div class="apiright apiimg"><img title="Image source: fischertechnik" src="/assets/img/switches/kombi.png">Mini Switch (37783) and Reversing switch (36708)</div>
</div>

**Mini Switch** has three connections. Depending on the connection scheme, the switch can be used as "normally open" (pins 1+3) or as "normally closed" (pins 1+2). With **Reversing Switch** one pin of the left and one pin of the right side is used.

**Normally open** is the classic connection. If the button is pressed, the circuit is closed and the FtSwarmSwitch in the program reports "pressed".

Normally Open switches can be connected in parallel to "save" inputs. If any button is pressed, the circuit is closed and the FtSwarmSwitch in the program reports "pressed".

**Normally closed** is used when the signal is to interrupt a function. As long as the button is closed or the circuit is not interrupted, e.g. a motor should run. This is the classic emergency stop function: If the pushbutton interrupts the circuit or if the circuit is interrupted by a cable fault, the motor stops.

Normally Closed switches are connected in series to realize the emergency stop function with several switches. One pin of the first and one pin of the last switch are connected to the input of the ftSwarm. If any button is pressed, the circuit is interrupted and the FtSwarmSwitch in the program reports "pressed".

#### FtSwarmSwitch(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true)

Constructor to create a FtSwarmSwitch object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**
- normallyOpen (optional): **true** if you use pin 1+3 on the mini-button. **false** to use pins 1+2.

#### FtSwarmSwitch(const char *name, bool normallyOpen = true)

Constructor to create a FtSwarmSwitch object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.
- normallyOpen (optional): Set **true** if you use pin 1+3 on the mini-button. **false** to use pins 1+2.

#### bool isPressed()

This function returns true, if the switch is actually pressed.

#### bool isReleased()

This function returns true, if the switch is actually released.

#### bool hasToggledUp()

This function is true, if the switch changed from closed to open since your last toggle-call.

#### bool hasToggledDown()

This function is true, if the switch changed from open to closed since your last toggle-call.

#### bool getState()

This function returns true, if the switch is actually pressed.

#### FtSwarmToggle_t getToggle()

This function returns
- **FTSWARM_TOGGLEDOWN**, if the state of the switch changed from open to closed since your last toggle-call.
- **FTSWARM_TOGGLEUP**, if the state of the switch changed from closed to open since your last toggle-call.
- **FTSWARM_NOTOGGLE**, if the state of the switch didn't change since your last toggle-call.