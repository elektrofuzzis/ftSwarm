---
title: FAQ
nav_order: 92
---
## Frequently asked questions

### I used my controller as Kelda. Now I want to deploy the firmware again.

Just run a [firmware update](../update).

### Device is not working after firmware reinstall or upgrade

Connect a serial monitor using putty. Restart your device.
The device runs in an initial setup and asks for the hardware configuration.

Enter the needed information using the information on the device's name plate.
Afterwards the device will reboot.

The basic information about your device is stored in the nvs partition. If you cleaned that partition or the firmware version isn't able to load the nvs data,
you will be asked to enter this data once.

### Watchdog is crashing my app

Accessing a remote IO device, which isn't online yet and refreshing the Web UI at the same time will cause this error.
