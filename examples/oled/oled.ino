/*
 * oled.ino
 *
 * ftSwarm example using the onboard OLED display
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <Arduino.h>
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
