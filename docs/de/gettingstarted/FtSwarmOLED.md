---
title: OLED Display
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---
Nun ist es endlich soweit! In diesem Beispiel wird "Hello World!" auf dem OLED Display des ftSwarmControl ausgegeben. Für dieses Beispiel reicht es aus, den ftSwarmControl über USB an den PC anzuschließen.

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

Das Beispiel ist sehr einfach. Es gibt "Hello World!" in der linken oberen Bildschirmecke des weißen Bereiches aus.

Comming Soon: Die OLED-Kommandos funktionieren derzeit nur auf dem lokalen Controller.
{: .notice--info}
