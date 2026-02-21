/*
 * SwOSHWDisplay.cpp
 *
 * Display hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <FastLED.h>
 
#include "SwOSHW/SwOSHWDisplay.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSOLEDMenu.h"
#include "SwOSLog.h"


/***************************************************
 *
 *   SwOSPixel
 *
 ***************************************************/

// LED Representation in FastLED library
CRGB led[MAXLEDS];
uint8_t usedPixels = 0;
bool ledsInitialized = false;

SwOSPixel::SwOSPixel(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags) : SwOSIO( name, port, ctrl, SWOSIO_PIXEL, flags ) {

  if (ctrl->isLocal()) 
    setupLocal();

}

void SwOSPixel::setupLocal() {

  // leds[] need to be initialized only once.
  if (!ledsInitialized) {
    
    #if FTSWARM_HAL_DISCRETE_RGBS > 0
    rgbLed = new RGBLed();
    #endif

    #ifdef RGB_BUILTIN
    FastLED.addLeds<WS2812, RGB_BUILTIN, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); 
    #endif

    setBrightness( BRIGHTNESSDEFAULT );
    ledsInitialized = true;

  }

  // initialize pixel
  if ( port < MAXLEDS ) {
    setColor( FtSwarmColor::Black );
  }

}

void SwOSPixel::setColor(uint32_t color) {

  // store new color
  this->color = color;

  // apply local or remote
  if (ctrl->isLocal()) setColorLocal();
  else                 setRemote();
}

void SwOSPixel::setRemote() {
  
  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETPIXEL );
  cmd.data.pixelCmd.index = ctrl->getIndex(this);
  cmd.data.pixelCmd.color = color;
  cmd.data.pixelCmd.brightness = brightness;
  cmd.send( );
}

void SwOSPixel::setColorLocal() {

  // set color

  if ( port>=MAXLEDS ) return;

  #if FTSWARM_HAL_DISCRETE_RGBS > 0
  if ( port == 0 ) {
      rgbLed->setColor( color );
      return;
  }
  #endif

  led[(FTSWARM_HAL_DISCRETE_RGBS)?port-1:port] = color;
  FastLED.show();

}

void SwOSPixel::setBrightness(uint8_t brightness) {

  // store new brightness
  this->brightness = brightness;

  // apply local or remote
  if (ctrl->isLocal()) setBrightnessLocal();
  else                 setRemote();

}

void SwOSPixel::setBrightnessLocal() {

  // set brightness

  if ( port>=MAXLEDS ) return;

  #if FTSWARM_HAL_DISCRETE_RGBS > 0
  if ( port == 0 ) {
      rgbLed->setBrightness( brightness );
      return;
  }
  #endif

  FastLED.setBrightness( brightness );
  FastLED.show();

}

void SwOSPixel::serialize( Serialize *serialize ) {
  serialize->startObject( );
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_BRIGHTNESS, brightness);
  serialize->itemX( SERIALIZE_LITERAL_COLOR,     color);
  serialize->endObject();
}

void SwOSPixel::onTrigger( SwOSTriggerMath_t triggerMath, int32_t sensor, int32_t parameter ) {
  
  setColor( evalTriggerMath( triggerMath, sensor, getColor(), parameter, 0, 0xFFFFFF ) );

}

/***************************************************
 *
 *   SwOSOLED
 *
 ***************************************************/

SwOSOLED::SwOSOLED(const char *name, SwOSCtrl *ctrl, uint8_t flags ) : SwOSIO( name, ctrl, SWOSIO_OLED, flags ) {
 
  if ( ctrl->isLocal() ) { 
    setupLocal(); 
  }
 
}
 
void SwOSOLED::setupLocal() {
 
  // startup hardware
  if (!oled) oled = new OLED( );
  // if (!oledMenu) oledMenu = new OLEDMenu( ctrl );

}


void SwOSOLED::invertDisplay(bool i) {
 
  if (oled) oled->invertDisplay( i );

}
 
void SwOSOLED::fillScreen(bool white) {

  if (oled) oled->fillScreen( white );

} 
 
void SwOSOLED::dim(bool dim) {

  if (oled) oled->dim( dim );
   
}
 
void SwOSOLED::setContrast(uint8_t contrast) {
 
  if (oled) oled->setContrast( contrast );
   
}
 
void SwOSOLED::drawPixel(int16_t x, int16_t y, bool white ) {

  if (oled) oled->drawPixel( x, y, white );

}
 
void SwOSOLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white) {
   
  if (oled) oled->drawLine( x0, y0, x1, y1, white );

} 
 
void SwOSOLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white) {
 
  if (oled) oled->drawRect( x, y, w, h, fill, white );
   
}
 
void SwOSOLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white) {
   
  if (oled) oled->drawRoundRect( x0, y0, w, h, radius, fill, white );
   
} 
 
 
void SwOSOLED::drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white) {

  if (oled) oled->drawCircle( x0, y0, r, fill, white );
   
} 
 
void SwOSOLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white) {
   
  if (oled) oled->drawTriangle( x0, y0, x1, y1, x2, y2, fill, white );
 
} 
 
void SwOSOLED::setCursor(int16_t x, int16_t y) {

  if (oled) oled->setCursor( x, y );
   
}
 
void SwOSOLED::getCursor(int16_t *x, int16_t *y) {
   
  if (oled) oled->getCursor( x, y );
   
}
 
void SwOSOLED::setTextColor(bool c, bool bg) {
   
  if (oled) oled->setTextColor( c, bg );

}
 
void SwOSOLED::setTextWrap(bool w) {
   
  if (oled) oled->setTextWrap(w);

}
 
void SwOSOLED::setRotation(uint8_t r) {
   
  if (oled) oled->setRotation(r);
   
}
 
uint8_t SwOSOLED::getRotation(void)  {

  if (oled) return oled->getRotation();
  return 0;
   
}
 
void SwOSOLED::setTextSize(uint8_t sx, uint8_t sy) {
   
  if (oled) oled->setTextSize( sx, sy );
   
} 
 
void SwOSOLED::getTextSize( uint8_t *sx, uint8_t *sy ) {

  if (oled) oled->getTextSize( sx, sy );

}
 
void SwOSOLED::drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y) {
   
  if (oled) oled->drawChar( x, y,c, color, bg, size_x, size_y );
 
} 
 
void SwOSOLED::write( const char *str ) {

  if (oled) oled->write( str );
   
}
 
void SwOSOLED::write( const char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill, bool invert ) { 

  if (oled) oled->write( str, x, y, align, fill, invert );
   
}
 
void SwOSOLED::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
 
  if (oled) oled->getTextBounds( string, x, y, x1, y1, w, h );
   
} 
 
int16_t SwOSOLED::getWidth(void)  {

  if (oled) return oled->getWidth();
  return 0;
   
}
 
int16_t SwOSOLED::getHeight(void) {
 
  if (oled) return oled->getHeight();
  return 0;
   
}
