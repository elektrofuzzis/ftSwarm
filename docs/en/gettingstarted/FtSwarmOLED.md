---
title: OLED Display
layout: category
lang: en
classes: wide
sidebar:
    nav: gettingstarted-en
---
Now it's time to use the ftSwarmControls OLED to show "Hello world!" on the display. To run this example,you just need to connect your ftSwarmControl by USB to your PC.

```cpp
#include <ftSwarm.h>

void setup() {

  Serial.begin(115200);
  FtSwarmSerialNumber local = ftSwarm.begin();
  
  FtSwarmOLED oled = FtSwarmOLED( local );
  
  // upper left corner
  oled.write("Hello world!");
  
  // center
  oled.write("Hello world!", oled.width()/2, oled.height()/2, FTSWARM_ALIGNCENTER);
  
  // lower right corner
  oled.write("Hello world!", oled.width(), oled.height() - 8 , FTSWARM_ALIGNRIGHT);
  
}

void loop() {}
```

The example is very easy. It writes "Hello world!" at the upper left corner, at the center and at the lower right corner of your screen. Since the yellow area is used by the firmware, pixel (0/0) is the upper left pixel in the white area.

Comming Soon: The OLED display commands are actually limited to the local controller. You can't send OLED display commands to a remote controller yet.
{: .notice--info}
