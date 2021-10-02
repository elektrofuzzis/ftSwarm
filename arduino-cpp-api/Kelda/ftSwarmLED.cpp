//////////////////////////////////////////////////////////////////
//
// FtSwarmLED.cpp
//
// ftSwarm LED Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#include "FtSwarmLED.h"
#include <Adafruit_NeoPixel.h>

// FtSwarmLED is a facade on adafruit's neopixel implementation.
// Therefore a "global" instance of the neopixel-strip-class is needed to control all LEDs

Adafruit_NeoPixel neoStrip = Adafruit_NeoPixel(2, 32, NEO_GRB + NEO_KHZ800);
bool neoStripStarted = false;

// class implementation

FtSwarmLED::FtSwarmLED( char* UUID, uint8_t LED ) : FtSwarmObj( UUID, LED) {

  if (isLocal()) {

    // check, if local strip is started
    if (!neoStripStarted) {
      neoStrip.begin();
      neoStripStarted = true;
    }

    // check, if user added neopixels 
    if ( neoStrip.numPixels() >= LED ) {
      neoStrip.updateLength( LED+1 );
    }
    
  }
  
}

void FtSwarmLED::setPixelColor( uint8_t r, uint8_t g, uint8_t b ) {

  if (isLocal()) {
    neoStrip.setPixelColor( _port, r, g, b );
    neoStrip.show();
  } else {
    rpc( SETPIXELCOLOR, neoStrip.Color(r, g, b));
  }
  
}

void FtSwarmLED::setPixelColor( uint32_t c ) {
  
  if (isLocal()) {
    neoStrip.setPixelColor( _port, c );
    neoStrip.show();
  } else {
    rpc( SETPIXELCOLOR, c );
  }
  
}

void FtSwarmLED::setBrightness( uint8_t brightness ) {
  
  if (isLocal()) {
    neoStrip.setBrightness( brightness);
    neoStrip.show();
  } else {
    rpc( SETBRIGHTNESS, brightness );
  }
  
}

void FtSwarmLED::off( void ) {
  
  if (isLocal()) {
    setPixelColor( BLACK );
  } else {
    rpc( SETPIXELCOLOR, BLACK );
  }
  
}
