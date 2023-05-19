---
title: The Kelda Principle
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---
In classic robotics you work with one central control unit, which drives all actuators and sensors. You just need to write one application, which controls all IO's. The same was your first application: just a small program with a **setup** and a **loop** function. 

Using multiple controllers in a swarm, you get a distributed system. Technically you need to write programs on all controllers which communicate using the wifi or RS485 network. Since this is a quite complex approach, the ftSwarm firmware covers all problems.

The "The Wee Free Men" in Terry Pratchett's novel are a chaotic bunch, as each one acts for himself like our controllers. Over the people rules the Kelda. 
She is the only female being of the clan and provides the basics of cooperation: what the she says is law and has to be obeyed. So we just need a Kelda controlling our swarm.

ForSince all controllers have a powerful ESP32 processor, one controller in the swarm becomes a Kelda. It takes over the control and sends control commands to the other controllers via wifi or RS485. All other controllers report their sensor states back, so Kelda knows about everything.

You just need to write one simple program and run it on your Kelda. All other controllers are just running the standard "firmware". 
By using the ftSwarm-Library, your "Kelda" program could access all of the distributed actuators and sensors easyly.
