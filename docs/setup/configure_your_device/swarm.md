---
title: Swarm Settings
parent: Configure Your Device
grand_parent: Setup
nav_order: 2
---
## Swarm Settings

In this menu you create your swarm. After resetting a device to factory settings, every device has it's own swarm. To run several devices as a swarm, 
you need to `create a new swarm` at the first device and connect all other devices using `join another swarm`.

```
Swarm menu

This device is connected to swarm "ftSwarm1" with 1 member(s) online.
Swarm PIN is 1.
(1) create a new swarm
(2) join another swarm
(3) list swarm members

(0) main
```

### Create a new swarm

This menu item creates a new swarm.

First, you need to enter the new swarm's name. The swarm name is to identify easily, which devices are connected in a swarm.

In the second step, you have to enter a new swarm pin. This is the pairing pin for all devices in the swarm and is used to secure the web UI as well.
It's best practise to a non-trivial pin.

Afterwards, the device leaves the old swarm and creates the new swarm.

### Join another swarm

After you created a new swarm, you could apply additional devices to the swarm. You're asked to enter the swarm's name and pin.

Afterwards, the device leaves the old swarm and joins the new one. To check swarm name and pin, at least another swarm device needs to be online.

### List swarm members

This feature is to show all swarm members, which are actually online.

<br>
*Congratulations! You have successfully succeded the needed setup work. Yes, there are some other options available like setting an alias name or running a factory reset. 
But these features are advanced options. Read them later.*

*Let's continue understanding the swarms basics and read about the kelda principle.*

[The Kelda Principle](../../../kelda){: .btn .float-right }
<br>