/*
 * SwOSHWDisplay.cpp
 *
 * Display hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <SwOSColor.h>
 
#include "SwOSHW/SwOSHWDisplay.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSOLEDMenu.h"
#include "SwOSLog.h"

#include <FastLED.h>

/***************************************************
 *
 *   SwOSPixel
 *
 ***************************************************/

CRGB castUI32ToColor(uint32_t color) {
    uint8_t r = (uint8_t)((color >> 16) & 0xFF);
    uint8_t g = (uint8_t)((color >> 8) & 0xFF);
    uint8_t b = (uint8_t)(color & 0xFF);
    return CRGB(r, g, b);
}

uint32_t castColorToUI32(CRGB color) {
    return ((uint32_t)color.r << 16) | ((uint32_t)color.g << 8) | (uint32_t)color.b;
}

#ifdef RGB_BUILTIN
CRGB leds[MAXLEDS];
#endif

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
    FastLED.addLeds<WS2812B, RGB_BUILTIN, GRB>(leds, MAXLEDS);
    #endif

    ledsInitialized = true;

  }

  // initialize pixel
  if ( port < MAXLEDS ) {
    setColor(CRGB::Black );
  }

}

void SwOSPixel::setColor( uint32_t color ) {

  setColor( castUI32ToColor( color ) );

}

void SwOSPixel::setColor( CRGB color ) {

  // store new color
  this->color = color;

  // apply local or remote
  if (ctrl->isLocal()) setLocal();
  else                 setRemote();
}

void SwOSPixel::setRemote() {
  
  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETPIXEL );
  cmd.data.pixelCmd.index = ctrl->getIndex(this);
  cmd.data.pixelCmd.R = color.r;
  cmd.data.pixelCmd.G = color.g;
  cmd.data.pixelCmd.B = color.b;
  cmd.data.pixelCmd.brightness = brightness;
  cmd.send( );
}

void SwOSPixel::setLocal() {

  // set color

  if ( port>=MAXLEDS ) return;

  #if FTSWARM_HAL_DISCRETE_RGBS > 0
  if ( port == 0 ) {
      rgbLed->setColor( color );
      rgbLed->setBrightness( brightness );
      return;
  }
  #endif

  #ifdef RGB_BUILTIN
  
    uint8_t index = (FTSWARM_HAL_DISCRETE_RGBS)?port-1:port;
    leds[index] = CRGB(color.r, color.g, color.b);
    leds[index].nscale8_video( brightness ); 
    FastLED.show();

  #endif

}

void SwOSPixel::setBrightness(uint8_t brightness) {

  // store new brightness
  this->brightness = brightness;

  // apply local or remote
  if (ctrl->isLocal()) setLocal();
  else                 setRemote();

}

void SwOSPixel::serialize( Serialize *serialize ) {
  serialize->startObject( );
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_BRIGHTNESS, brightness);
  serialize->item( SERIALIZE_LITERAL_COLOR,      color);
  serialize->endObject();
}

void SwOSPixel::onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t parameter ) {
  

  setColor( castColorToUI32( evalTriggerMath( triggerMath, sensor, castColorToUI32( getColor() ), parameter, 0, 0xFFFFFF ) ) );

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
 
  oled.begin();

}

void SwOSOLED::dim(bool dim) { 
  
  oled.dim( dim ); 

}

void SwOSOLED::cls( void ) { 
  
  oled.cls();

};

void SwOSOLED::cls( FtSwarmOledScreen_t screen ) {

  oled.cls( screen );

}

int16_t SwOSOLED::getScreenWidth(void) {

  return oled.getScreenWidth();

}

int16_t SwOSOLED:: getScreenHeight( FtSwarmOledScreen_t screen ) {

  return oled.getScreenHeight();

}

int16_t SwOSOLED::getTextWidth( const char *text ) { 
  
  return oled.getTextWidth( text );

}

int16_t SwOSOLED::getTextHeight( void ) { 

  return oled.getTextHeight();

}

void SwOSOLED::drawButton( FtSwarmOledScreen_t screen, int16_t x, int16_t y, uint8_t width, const char *text, uint8_t flags, uint8_t paddingH, uint8_t paddingV ) {

  oled.drawButton( screen, x, y, width, text, flags, paddingH, paddingV );

}

void SwOSOLED::setDrawColor( uint8_t color ) {

  oled.setDrawColor( color );

}

void SwOSOLED::drawCircle( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t r, FtSwarmOledFill_t fill ) {

  oled.drawCircle( screen, x, y, r, fill );

}

void SwOSOLED::drawEllipse( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t rx, int16_t ry, FtSwarmOledFill_t fill ) {

  oled.drawEllipse( screen, x, y, rx, ry, fill );

}

void SwOSOLED::drawLine( FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1 ) {

  oled.drawLine( screen, x0, y0, x1, y1 );

}
    
void SwOSOLED::drawPixel( FtSwarmOledScreen_t screen, int16_t x, int16_t y ) {

  oled.drawPixel( screen, x, y );

}

void SwOSOLED::drawRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, int16_t h, FtSwarmOledFill_t fill ) {

  oled.drawRect( screen, x, y, w, h, fill );

}

void SwOSOLED::drawRoundRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, FtSwarmOledFill_t fill ) {

  oled.drawRoundRect( screen, x, y, w, h, r, fill );

}

void SwOSOLED::drawStr( FtSwarmOledScreen_t screen, int16_t x, int16_t y, const char *text, FtSwarmAlign_t align ) {

  oled.drawStr( screen, x, y, text, align );

}

void SwOSOLED::drawStrRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, const char *text, FtSwarmAlign_t align, uint8_t paddingH, uint8_t paddingV ) {

  oled.drawStrRect( screen, x, y, w, text, align, paddingH, paddingV );
}

void SwOSOLED::drawTriangle( FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, FtSwarmOledFill_t fill ) {

  oled.drawTriangle( screen, x0, y0, x1, y1, x2, y2, fill );

}
