---
title: RGB LEDs & ftPixel
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---
Der ftSwarm hat zwei eingebaute RGB-LEDs, um den Betriebszustand anzeigen. Die LEDs können aber auch über das Steuerprogramm geschaltet werden. Bis zu 1 ftPixels können  zusätzliche extern an den LED-Port angeschlossen werden.

Die Einrichtung der Hardware ist einfach: Sie benötigen lediglich einen ftSwarm-Controller, der mit einem USB-Kabel an Ihren PC angeschlossen ist, und ein ftSwarm-Netzteil.

ftPixel kann nur mit ftSwarm und ftSwarmRS verwendet werden.
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

Die Firmware verwendet intern die FastLED-Bibliothek. Der Import **#include "FastLED.h "** wird benötigt, um auf die FastLED Farbdefinitionen wie **CRGB::Blue** zuzugreifen. Die LEDs werden mit den Funktionne  **setColor** und **setBrightness** gesteuert. setColor erwartet eine uint32_t als Farbe [0..0xFFFFFF] und setBrightness setz die Hellingkeit von 0 bis 255.

Bekanntes Problem: Aufgrund eines Fehlers in der FastLED-Bibliothek kann die Helligkeit einer LED nicht einzeln eingestellt werden. Ein setBrightness Befehl setzt die Helligkeit aller Leds.
{: .notice--info}

Gut zu wissen: RGB-LEDs benötigen viel Strom. Wenn Sie eine LED auf CRGB::White und maximale Helligkeit einstellen, so hat diese LED einen Stromverbrauch von 60 mA. Zwei LEDs mit maximaler Helligkeit benötigen genauso viel Strom wie der ESP32 Chip mit aktivem WLAN. Reduziert man die Helligkeit auf einen Wert zwischen 16 und 64, so sinkt der Stromverbrauch auf 6%..25%.
{: .notice--info}