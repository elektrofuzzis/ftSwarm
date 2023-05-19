---
title: FtSwarmPixel
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
        The ftSwarm has two built-in RGB LEDs that indicate the operational status of the controller. These LEDs can also be used by the control program.<br>
        Additionally, up to 16 external ftPixels can be connected to the LED port of the ftSwarm. 
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: elektrofuzzis" src="/assets/img/LampLedDisplay/ftPixel-api.png">ftPixel</div>
</div>

External ftPixels are connected via the LED Extention Port. On the ftSwarm the ftPixel adapter can be used, on the ftSwarmRS the first ftPixel is connected directly to the screwterminal. The ftPixels additionally require a 9V power supply. Please consider the current consumption of the LEDs when selecting the power supply for the ftPixels!

![Anschluss](/assets/img/LampLedDisplay/ftPixelChain.png)

On the left side of the ftPixel's housing you will see "+" and "-" markings. Connect the middle pin on the marked side of the ftPixel to the LED Extention Port of the ftSwarm. "+" and "-" are connected to the power supply.

Further ftPixels could be simply cascaded. The three pins of the unmarked/right side of the previous ftPixel are connected to the pins of the marked/left side of the next ftPixel.

ftPixel can only be used on the ftSwarm and ftSwarmRS.
{: .notice--info}

#### FtSwarmLED(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmLED object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, use **FTSWARM_LED1** and **FTSWARM_LED2** for the built-LEDs, **FTSWARM_LED3** ... **FTSWARM_LED18** for external ftPixel. 

#### FtSwarmLED( const char *name )

Constructor to create a FtSwarmLED object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void setColor(uint32_t color)

Sets the LED's color. Color is a RGB value, you could use FastLEDs CRGB-constants.

#### uint32_t getColor()

Gets the LED's color.

#### void setBrightness(uint8_t brightness)

Set the LED's brightness. 255 is maximum power, 0 will turn off the LED.

#### uint8_t getBrightness()

Get the LED's brightness.