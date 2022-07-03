---
title: Serial Connection
parent: Setup
nav_order: 1
---

## Serial Connection

Connect the ftSwarm to the PC using the USB cable. During the configuration process, the ftSwarm is powered by the USB cable, so you don't need to connect a power suppy.
If the device driver does not install automatically, download and install it. Below is the list of USB to serial converter chips used in ftSwarm devices:

- ftSwarm [wch-ic.com CH340C](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)
- ftSwarmControl [silicon labs CP210x USB to UART Bridge VCP Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).

### Windows

Check the list of identified COM ports in the Windows Device Manager. Disconnect ftSwarm and connect it back, 
to verify which port disappears from the list and then shows back again.

### Linux

To check the device name for the serial port of your ftSwarm, run this command two times, first with the ftSwarm unplugged, then with plugged in.
The port which appears the second time is the one you need:

```
ls /dev/tty*
```

The currently logged user should have read and write access the serial port over USB. 
On most Linux distributions, this is done by adding the user to `dialout` group with the following command:

```
sudo usermod -a -G dialout $USER
```

On Arch Linux this is done by adding the user to `uucp` group with the following command:

```
sudo usermod -a -G uucp $USER
```


### macOS

To check the device name for the serial port of your ftSwarm, run this command two times, first with the ftSwarm unplugged, then with plugged in.
The port which appears the second time is the one you need:

```
ls /dev/cu.*
```

[Choose and test a terminal program](../terminal){: .btn .float-right }
<br>