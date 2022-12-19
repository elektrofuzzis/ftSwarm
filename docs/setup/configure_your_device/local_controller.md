---
title: Local Controller
parent: Configure Your Device
grand_parent: Setup
nav_order: 1
---

## Local Controller

In this menu you could setup the wifi connection and some other options of your locally connected controller.

Use the first options to configute of your ftSwarm's wifi connection. After resetting a device to factory settings, it provides it's own SSID.
This SSID is ftSwarm&lt;SerialNumber&gt; and has no password. You could connect your PC to the device's SSID and connect to the device's status page [http://192.168.4.1/](http://192.168.4.1i/).

The other options are used to set the number of RGBLeds and to calibrate ftSwarmControl's joysticks.

```
Local controller menu

(1) AP-Mode:  AP-MODE
(2) SSID:     ftSwarm1
(X) Password: NOPASSWORD
(4) Channel:  0
(5) RGBLeds:  2
(X) Calibrate joysticks

(0) main
```

### AP-MODE

Your ftSwarm has two wifi modes: AP-MODE and CLIENT-MODE. Use AP-Mode, if you want to run your swarm without using other wifi infrastructure.
This mode is pretty nice to use at fairies and conventions. But keep in mind, everybody could connect to this SSID and check the device's status page.

The second mode - CLIENT-MODE - is used, if you want to run your swarm in an existing wifi structure. In that case, you need to set SSID and Password as well.

### SSID

Use this option to set your wifi SSID. If you use AP-Mode to run your swarm, you need to set the same SSID at all Swarm members. 
Using CLIENT-MODE this is the SSID of your wifi infrastructure.

### Password

If you're using CLIENT-MODE, you need to enter the wifi password.

### Channel

Working in **AP-Mode** you need to specify your wifi **channel** as well. Since ftSwarm is using a 2.4GHz wifi, you could use 11 different channels. 
All devices in your Swarm must use the same channel. Since adjacent channels influence each other, it's best practice to use only channels 1, 6 and 11.

In CLIENT-MODE the channel is automatically set by your wifi infrastructure.

ftSwarm
{: .label .label-blue .float-right}
### RGBLeds

ftSwarm has two builtin RGB-Leds. Additionally, you could add up to 14 external ftRGBLeds. To show the status of the RGB Leds at the controller's status 
page correctly, you need to set the number of RGB-Leds.

ftSwarmControl
{: .label .label-blue .float-right}
### Calibrate Joysticks

With a ftSwarmControl, this option is used calibrate the joystick's zero position and store it in the nvs storage.

[Setup your swarm](../swarm){: .btn .float-right }
<br>
