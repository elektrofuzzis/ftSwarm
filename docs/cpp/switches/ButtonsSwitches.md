---
title: Buttons & Switches
parent: Writing C++ Applications
nav_order: 5
has_children: yes
has_toc: no
---
## Buttons & Switches

A button is a builtin switch at the ftSwarmControl.
Switches are externally connected to the four independend input channels (A1..A4).

### Normally closed & normally open

Mini switches have 3 pins. Using pins 1 & 3, the switch is closed by pressing the button. Connecting pins 1 & 2: pressing the button opens the switch.

So the switch is "normally closed" or "normally open". Calling the constructor, you could specify your wiring optionally.
Afterwards you don't have worry about "normally closed" and "normally open". If your switch is pressed, it's state will be 1 or pressed.

### Toggles

In many applications your're not interested in the state of a switch, you want to know if the switch has changed it's state. You could wait actively using

```cpp
while (switch.isReleased());
```

This works fine. But you block a core and it's a little bit difficult to do other things while waiting. Since the firmware polls regulary the switches state in the background,
it notices status changes, too.

```cpp
void loop() {

  if (switch.hasToggledUp()) motor.setSpeed(100);
  if (switch.hasToggledDown()) motor.setSpeed(0);
  
  // use the time and do someting else
```

is smarter, because you're not blocking your application.

Keep in mind, every call of a *Toggle* command resets the toggle.

```cpp
if ( (switch.getToggle() != FTSWARM_NOTOGGLE) && (switch.hasToggeledUp()) ) ...
```

is always false. Even if `switch.getToggle()` returns `FTSWARMTOGGLEUP` or `FTSWARMTOGGLEDOWN`, the internal toggle state is resetted to `FTSWARM_NOTOGGLE`.
So the following `switch.hasToggeledUp()` will be always `false`.

## Class Reference

<div class="flex-imgs">
	<div class="ftblock">
		<a href="../switch/">
			<img class="ftimg" src="../../../assets/img/switches/switch-mini.png">
			<p class="fttext">Mini Switch (37783)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../switch/">
			<img class="ftimg" src="../../../assets/img/switches/switch-reversing.png">
			<p class="fttext">Reversing switch (36708)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../photo-transistor/">
			<img class="ftimg" src="../../../assets/img/switches/photo-transistor.png">
			<p class="fttext">Photo Transistor (36134)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../switch-reed/">
			<img class="ftimg" src="../../../assets/img/switches/switch-reed.png">
			<p class="fttext">Reed contact (36120)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../button/">
			<img class="ftimg" src="../../../assets/img/switches/ftSwarmControl.png">
			<p class="fttext">Buttons ftSwarmControl</p>
		</a>
	</div>
</div>

Image source: fischertechnik
[Analog sensors](../../analog/analog){: .btn .float-right }
<br>