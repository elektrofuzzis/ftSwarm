---
title: RGB LEDs & FtSwarmPixel
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---
The ftSwarm has two built-in RGB LEDs that indicate the operating status of the controller. However, the LEDs can also be switched by the control program. Up to 16 additional ftPixels can be connected to the LED port externally.

The hardware setup is easy: you just need a ftSwarm controller connected with a USB cable to your PC and a ftSwarm power supply.

ftPixel can only be used on the ftSwarm and ftSwarmRS.
{: .notice--info}

```cpp
#include <ftSwarm.h>
#include <FastLED.h>

FtSwarmLED *led1;
FtSwarmLED *led2;

void setup( ) {

  Serial.begin(115200);

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
  
  // get led instances
  led1 = new FtSwarmLED( local, FTSWARM_LED1 );
  led2 = new FtSwarmLED( local, FTSWARM_LED2 );
  
}

void loop( ) {

  Serial.println("colors...");
  led1->setColor( CRGB::Blue );
  led2->setColor( CRGB::Cyan);
  delay(500);
  led1->setColor( CRGB::Red);
  led2->setColor( CRGB::Orange);
  delay(500);
  led1->setColor( CRGB::Green);
  led2->setColor( CRGB::Yellow);
  delay(500);

  Serial.println("brightness");
  for (uint8_t i=64; i!=0; i-=16) {
    led1->setBrightness(i);
    delay(500);
  }
  
  led1->setBrightness(64);

}
```

The firmware uses internally the FastLED library. The import **#include "FastLED.h"** is needed to access the FastLED color definitions like **CRGB::Blue**.
To set the LEDs, we use the commands **setColor** and **setBrightness**. setColor expects a uint32_t as color [0..0xFFFFFF] and setBrightness needs a uint8_t. 
255 is maximum Power, using 0 sets the LED off. 

Known Issue: Due to a bug in the FastLED library, your couldn't set the brightness of an led individually. A setBrightness command sets the brightness of all leds
{: .notice--info}

Need To Know: Keep in mind, RGB LEDs need a lot of power. Setting a led to CRGB::White and maximum brightness, this LED has a power consumption of 60mA. Two LEDs with maximum power will need about the same power as the ESP32 Chip with wifi. Reducing the brigthness to a value between 16 and 64, the power consumption will be reduced to 6%..25%.
{: .notice--info}
