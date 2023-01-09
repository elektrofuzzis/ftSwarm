---
title: Motor Types
parent: Writing C++ Applications
nav_order: 3
has_children: true
has_toc: false
---
## Motor Types

Both controllers have two independend motor drivers (M1 & M2). They could drive all type of *fischertechnik* 9V black DC motors. 
Old grey fischertechnik DC motors or other industrial 9V DC motors could be used, too.

### Speed

It's easy to move a motor. `setSpeed` will start or stop your motor:
- Setting a positive speed, the motor will turn.
- Setting a negative speed, the motor will turn in the other direction.
- Setting 0, the motor will stop.

Use `getSpeed` to read your speed settings.

### Brake & Coast

There are two general fischertechnik motor categories: spindle or axis. 

Using a "spindle" motor and some gear wheels in your robotics, the mechanics is self-locking. 
Cutting your power supply, the model will stop and the mechanics will keep in position. 

In case of an axis based motor, the mechanics isn't self-locking: the weight of your model might turn your motor.
Using an axis based motor, you have two additional motor commands:

- `coast` stops your motor and releases the motor power. The model could move the motor manually.
- `brake` stops your motor but keeps the motor power on. The motor is blocked and you couldn't move the motor manually.

In difference to `setSpeed(0)` both commands they will not change your speed setting. Using `setMotionType( FTSWARM_ON )` the motor will move again.

### Class Reference

<div class="flex-imgs">
	<div class="ftblock">
		<a href="../xmotor/">
			<img class="ftimg" src="../../../assets/img/motor/motor-xs.png">
			<p class="fttext">XS Motor (137096)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../xmotor/">
			<img class="ftimg" src="../../../assets/img/motor/motor-mini.png">
			<p class="fttext">Mini Motor (32293)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../xmmotor/">
			<img class="ftimg" src="../../../assets/img/motor/motor-xm.png">
			<p class="fttext">XM Motor (135485)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../tractor/">
			<img class="ftimg" src="../../../assets/img/motor/motor-tractor.png">
			<p class="fttext">Tractor Motor (151178)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../encoder/">
			<img class="ftimg" src="../../../assets/img/motor/motor-encoder.png">
			<p class="fttext">Encoder Motor (153422)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../encoder/">
			<img class="ftimg" src="../../../assets/img/motor/motor-encoder-competition.png">
			<p class="fttext">Encoder Motor (186175)</p>
		</a>
	</div>
</div>

Image source: fischertechnik
[Let's talk about switches and buttons](../../switches/ButtonsSwitches){: .btn .float-right }
<br>
