---
title: The Kelda Principle
nav_order: 3
---

## The Kelda Principle

In classic robotics you work with one central contol unit, which drives all actuators and sensors.
You just need to write one application, which controls all IO's. That's easy, in an Arduino environment you just have to write two procedures: 

- `setup` is just called one time and initializes all of your hardware.
- Functionality is coded in the procedure `loop`, which runs forever in a loop.

Complexity starts, if you start using interrupts and threads.

By using multiple controllers in a swarm, you get a distributed system. Now every controller needs it's own application. So you get a bunch of threads, 
who need to be coordinated via network.

The "The Wee Free Men" in Terry Pratchett's novel are a chaotic bunch, as each one acts for himself. Over the people rules the Kelda. 
She is the only female being of the clan and provides the basics of cooperation: what the she says is law and has to be obeyed.

As in the novel, it is very demanding to get all the small, free ftSwarm controllers under one hat.

For this, the ftSwarm has a simple solution using the Kelda approach: Since all controllers have a powerful ESP32-Wrover processor, 
one controller in the model becomes a Kelda. It takes over the control and sends control commands to the other controllers via wifi.
All other controllers report their sensor states back, the Kelda knows about everything.

So you just need to write one program with the two procedures `setup`and `loop` again. All other controllers run the standard "firmware". 
By using the ftSwarm-Library, your "Kelda" program could access all of the distributed actuators and sensors easyly.

[Start writing your first C++ application](../cpp/program){: .btn .float-right }
<br>