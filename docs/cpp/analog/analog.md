---
title: Analog Sensors
parent: Writing C++ Applications
nav_order: 10
has_children: yes
has_toc: no
---
## Analog Sensors

The firmware supports a bunch of analog sensors.

In standard, technically the analaog sensor is a resistor. These sensors could be connected to every of ftSwarm's inputs. At ftSwarmControl you could just use `FTSWARM_A1`.

ftSwarm is able to measure voltages at input `FTSWARM_A2` as well. 

## Class Reference

<div class="flex-imgs">
	<div class="ftblock">
		<a href="../FtSwarmLDR/">
			<img class="ftimg" src="../../../assets/img/analog/photo-resistor.png">
			<p class="fttext">Photo Resistor (32698)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../FtSwarmLDR/">
			<img class="ftimg" src="../../../assets/img/analog/photo-transistor.png">
			<p class="fttext">Photo Transistor (36134)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../FtSwarmOhmmeter/">
			<img class="ftimg" src="../../../assets/img/analog/poti.png">
			<p class="fttext">Potentiometer (186184)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../FtSwarmVoltmeter/">
			<img class="ftimg" src="../../../assets/img/analog/voltmeter.png">
			<p class="fttext">Voltage</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../FtSwarmThermometer/">
			<img class="ftimg" src="../../../assets/img/analog/sensor-ntc.png">
			<p class="fttext">NTC resistor 1.5 kΩ (36437)</p>
		</a>
	</div>
	<div class="ftblock">
		<a href="../FtSwarmJoystick/">
			<img class="ftimg" src="../../../assets/img/analog/joystick.png">
			<p class="fttext">Joystick</p>
		</a>
	</div></div>

Image source: fischertechnik
