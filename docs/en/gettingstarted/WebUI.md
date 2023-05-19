---
title: wifi & status page
lang:  en
classes: wide
layout: category
sidebar:
    nav: gettingstarted-en
---

To get several ftSwarm controllers to act as a swarm, they must communicate with each other. This usually takes place via wifi, the ftSwarmRS is able to communicate via RS485, too. Additionally the status page of the controller can be accessed via wifi. Here the the input signals are shown; motors, servos and LEDs can be controlled manually.

Wifi configuration needs a USB connection. Start a terminal program as described before and boot the controller via the reset button.

The controller now shows its boot message. Press any key, so that you enter the configuration menu of the ftSwarm firmware:

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

Choose  **(1) wifi Settings**. Configure *CLIENT-MODE* and add you wifi's credentials:

```
wifi settings

(1) wifi:     CLIENT-MODE
(2) SSID:     <WLAN-Name/SSID>
(3) Password: <WLAN-Passwort>

(0) exit
wifi>
```

Use **(0)** to save your changes. The controller will restart and connect to your local wifi..

You could now access the statuspage by **`http:\\ftSwarm<SerialNumber>`**. Replace <SerialNumber> with the serial number of your ftSwarm controller.

![Monitoring ftSwarm](../../assets/img/ftSwarm_Monitor.png)

*In case of any problem, please check*
- *if your browser has java script enabled*
- *and your wifi allows a direct peer-to-peer communication.*

To operate the actuators such as motors, you need to log in to the controller using the LOGIN button. Thereby the swarm pin is requested, by default this is the 4-digit serial number of the controller (in the example 0100).