---
title: Terminal Program
parent: Setup
nav_order: 2
---

## Terminal Program

For configuration you need a serial terminal program like **PuTTY** or **screen**. The serial console of the Arduino IDE is not suitable for this purpose.

### Windows and Linux

You could use [PuTTY SSH Client](https://www.putty.org/) on Windows and Linux. You can use other serial program and set communication parameters like below.
Run terminal, set identified serial port, baud rate = 115200, data bits = 8, stop bits = 1, and parity = N. Reset your ftSwarm to check some output.

### macOS

On macOS there is already a suitable serial terminal command named screen.

As discussed in Check port on Linux and macOS, run:

```
ls /dev/cu.*
```

You should see similar output:

```
/dev/cu.Bluetooth-Incoming-Port /dev/cu.SLAB_USBtoUART      /dev/cu.SLAB_USBtoUART7
```

The output will vary depending on the type and the number of boards connected to your PC. Then pick the device name of your board and run:

```
screen /dev/cu.device_name 115200
```

Replace device_name with the name found running ls /dev/cu.*.

Reset your ftSwarm to check some output. To exit the screen session type Ctrl-A + \ .

[Configure your device](../configure_your_device/configure_your_device){: .btn .float-right }
<br>