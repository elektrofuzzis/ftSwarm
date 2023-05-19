---
title: Configuration
layout: category
lang: en
classes: wide
sidebar:
    nav: advanced-en
---
Your controller needs some settings, e.g. a wifi profile or swarm parameters. Some settings have already been used in the tutorial. This chapter describes all configuration options in detail.

To configure the controller, the "Configuration Menu" can be started in the standard firmware at any time. For this purpose, a terminal program must be connected and any key pressed. The configuration menu can be started by

```cpp
ftSwarm.setup();
```

at any time in your own code.

<hr>
### Main Menu

**Main Menu** is the top level of the configuration menus. **(0) exit** terminates the configuration mode, using the standard firmware the controller will reboot afterwards. If the **Main Menu** was called with **ftSwarm.setup();** from a program, program will just continue.

<hr>
### Wifi Settings

To form a swarm, the individual controllers need to communicate with each other. Usually they use wifi - ftSwarmRS controllers could communicate additionally via RS485.

In addition, the status page of the controllers is called up via wifi.

Use **wifi** to select the type of wifi: 
- CLIENT-MODE uses an existing WLAwifiN. As a rule, this is the wifi of your internet router.
- AP-MODE does not need a router, a ftSwarm controller provides its own wifi network.
- OFF turns wifi off (ftSwarmRS only). This option reduces power consumption significantly.

If a wifi is already available, you should definitely use it. Select CLIENT-MODE and enter access data using **SSID** and **password**. Save the data by **exit** in the NVS. The controller will boot automattically and connect to your wifi afterwards.

Using AP-MODE is a little bit tricky:
- Select one controller and switch to AP-MODE there. With **SSID** you give the wifi a name. Due to restrictions of espressif, wifis in AP-MODE are always open. There is no password is required.
- In addition, you need to use option **wifi channel** to select the wifi channel. 
- All other controllers are set to CLIENT-MODE. Use the SSID above; set an empty password.
- Never put the Kelda in AP-MODE. As described in the tutorial, this will cause some problems during flushing your program.
- The AP-MODE has a high power consumption, the network is open for everyone (it has no password) and the transmission range is very limited.

The option **off** can only be set on the ftSwarmRS. In this case your swarm must communicate completely via RS485.

You share the wifi channels with the surrounding wifis. Therefore it is important to select the "right" channel. The ESP32 processors use the 2.4 GHz band. This band is divided into 11 channels, with adjacent channels overlapping. If two adjacent channels are used by a WLAN, they will interfere with each other. For this reason, it is best in the 2.4 GHz band if all WLANs only use channels 1, 6 and 11. Optimally, there is one unused channel. If this is not available, use an already used channel whose neighboring channels are not in use. You can see which channels are already in use on most Internet routers. With the app [wifiman](https://play.google.com/store/apps/details?id=com.ubnt.usurvey&hl=de&gl=US&pli=1) you can also analyze this via your smartphone.
{: .notice--info}

<hr>
### Web Server Settings

This menu just has two options:
- **WebUI: on/off** enables or disables the controllers status page.
- **Show X ftPixels in UI** defines the number of ftPixels which are displayed on the status page. It's used in the menu "Alias Names" as well. This value has no side effekt on the programming of ftPixels via the port number, all ports **FTSWARM_LED1** to **FTSWARM_LED18** can always be addressed.

<hr>
### Swarm Settings

Within this menu, your swarm is formed. Before you can form a swarm, all controllers need be able to communicate with each other. For this purpose, all controllers need either be logged into the same wifi profile or connected via RS485.

The menu always displays the name of the swarm, the pin and the number of registered controllers in the swarm:

```
This device is connected to swarm "MeinLieberSchwan" with 10 member(s) online. Swarm PIN is 999.
```

**swarm communication** sets the medium which the swarm uses to communicate.
- **wifi**: all controllers use wifi.
- **RS485**: all controllers user RS485. Zhis option is available on ftSwarmRS only. A mixed mode wifi/RS485 isn't possible. 

**create a new swarm**: Creates a new swarm. Swarm's name and PIN are requested. Subsequently, this controller is the first and only controller within the new swarm.

**join another swarm**: Adds the controller to an existing swarm. Swarm's name and PIN are requested. Since the controller will try to connect to the swarm, at least one controller of the swarm must be online.

**list swarm members**: Lists members of the swarm who are online.

Each controller stores the swarm's name and PIN in its NVS. The names of other controllers in the swarm are not stored, anyone who knows the name of the swarm and the PIN can join the swarm at any time. Therefore no option for leaving a swarm is necessary.
{: .notice--info}

<hr>
### Alias Names Settings

With this menu the ports alias names could be set. Named IO ports could be addressed via their name instead of the port number. Read [Alias Names](../gettingstarted/MotorSwitchAlias/) in the turorial for a detailed explanation.

To assign an alias name to an IO port, select the number of the desired IO. The name can be up to 30 characters long and must not contain spaces.

Within your swarm, all alias names must be unique. This is not checked during configuration since not all controllers in the swarm may be online. If a name is assigned to multiple ports, it is a coincidence which IO port is addressed when using the alias name.
{: .notice--info}

<hr>
### Factory Settings

This menu item resets the controller to factory settings. Since confidential information like wifi parameters are stored in the NVS, you should reset a controller to factory settings before passing it on to third parties.

<hr>
### I2C Settings

This menu defines the behavior of the I²C bus.

**Mode** determines whether the controller is to operate in slave or master mode. If I²C sensors are connected to the bus, MASTER is always the correct one. 

If a TXT controller (see cable car project) shall exchange data with the ftSwarm only, SLAVE mode has to be selected. Since the optional gyro is also connected to the I²C bus of the **ftSwarm** and **ftSwarmControl**, the controller must operate in MASTER mode if the gyro is to be used. **ftSwarmRS** addresses the internal gyro using a second I²C bus, so that it can use gyro and SLAVE mode at the same time.  
{: .notice--info}

**Gyro** enables/disables the gyro. Enable it only, if you actually want to use it in the application. Unused gyros have an unnecessary power consumption and a high communication need in the swarm.

**I2C Address** sets the I²C address of the controller in SLAVE mode. This option is not available in MASTER mode.

<hr>
### ftSwarmControl

The ftSwarmControl has two device specific settings.

**Display Type**: Depending on the used OLED display the firmware needs some differnt parameters. The first ftSwarmControls need display type 0, the newer ones type 1. If the display of the ftSwarmControl does not work, choose the other value.  

**Calibrate Joysticks**: calibrates zero position of the joysticks.

<hr>
### Remote Control Settings

With this menu a swarm can be programmed without a line of code. Therefore the used IO ports get alias names, afterwards they are addressed by the ftSwarmControl. For this purpose events are defined on buttons and joysticks to control the IO ports.

A detailed description is available at [Remote control](../gettingstarted/RemoteControl/).
