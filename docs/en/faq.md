---
title: Frequently Asked Questions
layout: category
lang: en
sidebar:
    nav: faq-en
---
### Firmware

<details>
<summary>I used my controller as Kelda. Now I want to deploy the firmware again.</summary>
<p>Just run a [firmware update](../../update).</p>
</details>

<hr>

<details>
<summary>Device is not working after reinstalling or upgrading the firmware</summary>
<p>Connect a serial monitor using putty. Restart your device.
The device runs in an initial setup and asks for the hardware configuration.</p>
<p>Enter the needed information using the information on the device's name plate. Afterwards the device will reboot.</p>
<p>The basic information about your device is stored in the nvs partition. If you cleaned that partition or the firmware version isn't able to load the nvs data, you will be asked to enter this data once.</p>
</details>

<hr>

<details>
<summary>My device isn't able to join the swarm</summary>
<p>Check, if your controller is connected to the same wifi like your swarm:</p>
<ul>
<li>wifi active?</li>
<li>Correct SSID and password?</li>
<li>Could you access the status page of all controllers from your PC?</li>
</ul>
<p>Are you using the RS485 interface?</p>
<ul>
<li>Did you change the ftSwarm communication to RS485 on all devices?</li>
<li>Check cabling: Did you connect all cables? Did you mix up A/B? Did you build a loop?</li>
<li>Did you set the termination jumpers at the first an the last controller in you chain?</li>
</ul>
<p>Some users reported this issue using Arduino IDE 2.0. Please use the latest 1.X version.</p>
</details>

<hr>

<details>
<summary>My device sends an SSID even in client mode</summary>
<p>If you run a wifi ssid scan, you will see for each controller an own SSID e.g. ftSwarm4711. There is no difference between controllers running in AP and client mode.</p>
<p>Ignore the additional SSIDs. This is a known issue.</p>
</details>

### Programming

<details>
<summary>Watchdog is crashing my app</summary>
<p>All ESP32 controllers us a watchdog, to identify infinity loops and to stop the execution.</p>
<div class="language-plaintext highlighter-rouge"><div class="highlight"><pre class="highlight"><code>while (switch->isReleased());
motor->setSpeed(200);</code></pre></div></div>
<p>blocks a core until the switch is closed. The watchdog identifies this infinity loop and aborts the program. It's better to use</p>
<div class="language-plaintext highlighter-rouge"><div class="highlight"><pre class="highlight"><code>if (switch->ToggleUp()) motor->setSpeed(200);</code></pre></div></div>
<p>in your <strong>loop</strong> function. Since the loops is restarted again and again, the if statement is executed regularly. The motor starts as well. The watchdog is fine, because the <strong>loop</strong> function terminates. Using <strong>delay</strong> keeps your watchdog healthy, too.</p>
</details>

<hr>

<details>
<summary>Remote motors don't work correctly</summary>
<p>The webside shows the motor's state correctly (e.g. running), but the motor doesn't run.</p>
<p>This is a known issue, and fixed with firmware 0.4.1.</p>
</details>

<hr>

<details>
<summary>Inputs not working</summary>
<p>Please check your 9V power supply. To work properly, the device needs a 9V power supply. USB Power isn't sufficient.</p>
</details>

<hr>

<details>
<summary>Kelda hangs after reboot or flashing code</summary>
<p>Your Kelda is providing your swarm's SSID, all swarm members are connected correctly. After you flashed some code on your Kelda, the swarm isn't working correctly.</p>
<p>If you check in detail, your Kelda shows "waiting on hardware" while initializing some remote stuff. (ftSwarmControl writes an error at the OLED display, ftSwarm has blue LEDs.)</p>
<p>Flashing and/or rebooting your device will restart the wifi stack as well. Since your Kelda provides the SSID, all other devices in your swarm will loose their connection and are not able to reconnect. With restarting the swarm members, they will connect to the SSID again. Your Kelda's program connects now to the remote stuff and is working fine.</p>
<p>Since it's stupid to restart all swarm members after flashing/rebooting your Kelda, you should change the device providing the SSID. Every swarm member could provide the SSID as well. If possible use your home wifi and run all devices in client mode.</p>
</details>

<hr>

<details>
<summary>Suspicious toggle behavior</summary>
<p>To explain the behavior, imagine a ftSwarmControl running the following code. (This behavior fits to all kind of inputs.)</p>

<div class="language-plaintext highlighter-rouge"><div class="highlight"><pre class="highlight"><code>void loop() {

  switch ( local_S1->getToggle() ) {
    case FTSWARM_TOGGLEDOWN:  Serial.println("S1 released."); break;
    case FTSWARM_TOGGLEUP:    Serial.println("S1 pressed."); break;
  }
  
  delay(250);

}</code></pre></div></div>
<p>Running the program and pressing/releasing the button, you will get alternating <strong>S1 pressed.</strong> and <strong>S1 released.</strong> messages. But in some cases, you will get two <strong>S1 pressed.</strong> in sequences without no <strong>S1 released.</strong> message.</p>
<p>The swarm checks all 25ms the state of all inputs. Getting the state of S1, the firmware compares the actual state (e.g. pressed) with the last state (e.g. released.). Now the input's toggle state is set to <strong>FTSWARM_TOGGLEUP</strong>.</p>
<p>Keep the button pressed, max. 250ms later your loop checks the state and sends <strong>S1 pressed</strong> to the serial monitor.
<strong>local_S1->getToggle()</strong> resets the input's toggle state to <strong>FTSWARM_NOTOGGLE</strong>. Next time the loop evaluates <strong>local_S1->getToggle()</strong>, there is nothing to display.</p>
<p>After you released the button, the toggle state switches internally to <strong>FTSWARM_TOGGLEDOWN</strong>. Your loop will send <strong>S1 released</strong> to the serial console.</p>
<p>But if you release and press the button during the 250ms wait time, the input's toggle state changes two times. Releasing the button to <strong>FTSWARM_TOGGLEDOWN</strong>. Pressing the button again, it will change to <strong>FTSWARM_TOGGLEUP</strong>. Now the delay time ends, the switch statement tests on the input's toggle state and sees <strong>FTSWARM_TOGGLEUP</strong>.
So you get two <strong>S1 pressed.</strong> messages in sequence.</p>
</details>