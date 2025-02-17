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
#include "SwOSHW/SwOSHWHAL.h"

/***************************************************
 *
 *   SwOSPixel
 *
 ***************************************************/

  // LED Representation in FastLED library
  CRGB led[MAXLEDS];
  uint8_t usedPixels = 0;
  bool ledsInitialized = false;

SwOSPixel::SwOSPixel(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ) {

  if (ctrl->isLocal()) _setupLocal();

}

void SwOSPixel::_setupLocal() {

  // leds[] need to be initialized only once.
  if (!ledsInitialized) {

    // assign port to GPIO.
    switch ( _ctrl->getCPU() ) {
      #if CONFIG_IDF_TARGET_ESP32S3
        case FTSWARMPWRDRIVE_1V141:
        case FTSWARMDUINO_1V141:
        case FTSWARMXL_1V00:
        case FTSWARMCAM_3V12:
        case FTSWARMRS_2V1:
        case FTSWARMRS_2V0:  FastLED.addLeds<WS2812, xGPIO_NUM_48, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); break;
      #endif
      case FTSWARMJST_1V15:  FastLED.addLeds<WS2812, GPIO_NUM_26, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); break;
      default:               FastLED.addLeds<WS2812, GPIO_NUM_33, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); break;
    }

    setBrightness( BRIGHTNESSDEFAULT );
    ledsInitialized = true;
  }

  // initialize pixel
  if ( _port < MAXLEDS ) {
    // leds[_port] = CRGB::Black;
    setColor( FtSwarmColor::Black );
  }

}

void SwOSPixel::setColor(uint32_t color) {

  // store new color
  _color = color;

  // apply local or remote
  if (_ctrl->isLocal()) _setColorLocal();
  else                  _setRemote();
}

void SwOSPixel::_setRemote() {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETLED );
  cmd.data.ledCmd.index = _port;
  cmd.data.ledCmd.color = _color;
  cmd.data.ledCmd.brightness = _brightness;
  cmd.send( );
}

void SwOSPixel::_setColorLocal() {

  // set color
  if (_port < MAXLEDS ) {
    led[_port] = _color;
    FastLED.show();
  }

}

void SwOSPixel::setBrightness(uint8_t brightness) {

  // store new brightness
  _brightness = brightness;

  // apply local or remote
  if (_ctrl->isLocal()) _setBrightnessLocal();
  else                  _setRemote();

}

void SwOSPixel::_setBrightnessLocal() {

  // set brightness
  if (_port < MAXLEDS ) {
    // TODO: brightness per pixel
    // leds[_port].fadeLightBy( brightness );
    FastLED.setBrightness( _brightness );
    FastLED.show();
  }

}

void SwOSPixel::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI8  ("brightness", _brightness);
  json->variableUI32X("color",     _color);
  json->endObject();
}

void SwOSPixel::onTrigger( int32_t value ) {

  setColor( (uint32_t) value );
  
}/***************************************************
 *
 *   SwOSOLED
 *
 ***************************************************/

 SwOSOLED::SwOSOLED(const char *name, SwOSCtrl *ctrl, uint8_t displayType) : SwOSIO( name, ctrl ) {
 
   if ( _ctrl->isLocal() ) { 
     displayType = displayType; 
     _setupLocal(); 
   }
 
 }
 
 void SwOSOLED::_setupLocal() {
 
   _display = new Adafruit_SSD1306 (128, 64, &Wire, -1);
   if ( !_display->begin(SSD1306_SWITCHCAPVCC, 0x3C ) ) {
     delete _display;
     _display = NULL;
     ESP_LOGE( LOGFTSWARM, "Couldn't initialize OLED display." );
     return;
   }
 
   _display->clearDisplay();
   // _display->dim(true);
      
   // set useful default values
   _display->setTextColor(true, false);   // Draw white text
   _display->cp437(true);       // Use full 256 char 'Code Page 437' font
 
   _display->setTextSize(3,3);            // Logo
   write( (char *) "ftSwarm", getWidth()/2, 0, FTSWARM_ALIGNCENTER, true );
 
   _display->setTextSize(1,1);            // hostname & version
   char line[100];
   sprintf( line, "%s %s", _ctrl->getHostname(), SWOSVERSION );
   write( line, getWidth()/2, 32, FTSWARM_ALIGNCENTER, true );
 
   // additional default values
   _display->setTextSize(1, 1);           // Normal 1:1 pixel scale
   _display->setCursor(0, 0);             // Start at top-left corner
 
   dim(true);
 
   display();
 
 }
 
 void SwOSOLED::display(void) {
 
   if (_display) _display->display();
   
 }
 
 void SwOSOLED::invertDisplay(bool i) {
 
   if (_display) _display->invertDisplay( i );
   
 }
 
 void SwOSOLED::fillScreen(bool white) {
   
   drawRect( 0, YELLOWPIXELS, getWidth(), getHeight()-YELLOWPIXELS, true, white );
   
 } 
 
 void SwOSOLED::dim(bool dim) {
 
   // origin adafruit code fails with some displays
   // if (_display) _display->dim( dim );
   setContrast( dim ? 1 : 0x8F );
   
 }
 
 void SwOSOLED::setContrast(uint8_t contrast) {
 
   // avoid problemns with setContrast and Display Types != 1
   if (_displayType != 1 ) return;
 
   // send set contrast
   Wire.beginTransmission( 0x3C );
   Wire.write( (uint8_t) 0 );
   Wire.write( 0x81 );
   Wire.endTransmission();
 
   // send contast value
   Wire.beginTransmission( 0x3C );
   Wire.write( (uint8_t) 0 );
   Wire.write( contrast );
   Wire.endTransmission();
   
 }
 
 void SwOSOLED::drawPixel(int16_t x, int16_t y, bool white ) {
   
   if (_display) _display->drawPixel( x, y + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0) );
   
 }
 
 void SwOSOLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white) {
   
   if (_display) {
     if      (x0==x1) _display->drawFastVLine( x0, y0 + YELLOWPIXELS, y1 - y0,               (white)?(SSD1306_WHITE):(0) ); 
     else if (y0==y1) _display->drawFastHLine( x0, y0 + YELLOWPIXELS, x1 - x0,               (white)?(SSD1306_WHITE):(0) ); 
     else             _display->drawLine(      x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0) );
   }
   
 } 
 
 void SwOSOLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white) {
 
   if (_display) {
     if (fill) _display->fillRect( x, y + YELLOWPIXELS, w, h, (white)?(SSD1306_WHITE):(0) );
     else      _display->drawRect( x, y + YELLOWPIXELS, w, h, (white)?(SSD1306_WHITE):(0) );
   }
   
 }
 
 void SwOSOLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white) {
   
    if (_display) {
     if (fill) _display->fillRoundRect( x0, y0 + YELLOWPIXELS, w, h, radius, (white)?(SSD1306_WHITE):(0)); 
     else      _display->drawRoundRect( x0, y0 + YELLOWPIXELS, w, h, radius, (white)?(SSD1306_WHITE):(0)); 
   }
   
 } 
 
 
 void SwOSOLED::drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white) {
   
   if (_display) {
     if (fill) _display->fillCircle( x0, y0 + YELLOWPIXELS, r, (white)?(SSD1306_WHITE):(0)); 
     else      _display->drawCircle( x0, y0 + YELLOWPIXELS, r, (white)?(SSD1306_WHITE):(0)); 
   }
   
 } 
 
 void SwOSOLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white) {
   
   if (_display) {
     if (fill) _display->fillTriangle( x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, x2, y2 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0)); 
     else      _display->drawTriangle( x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, x2, y2 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0)); 
   }
 
 } 
 
 void SwOSOLED::setCursor(int16_t x, int16_t y) {
   
   if (_display) _display->setCursor( x, y  + YELLOWPIXELS);
   
 }
 
 void SwOSOLED::getCursor(int16_t *x, int16_t *y) {
   
   if (_display) {
     *x = _display->getCursorX( );
     *y = _display->getCursorY( );
   }
   
 }
 
 void SwOSOLED::setTextColor(bool c, bool bg) {
   
   if (_display) _display->setTextColor( (c)?(SSD1306_WHITE):(0), (bg)?(SSD1306_WHITE):(0) );
   
 }
 
 void SwOSOLED::setTextWrap(bool w) {
   
   if (_display) _display->setTextWrap( w );
   
 }
 
 void SwOSOLED::setRotation(uint8_t r) {
   
   if (_display) _display->stopscroll( );
   
 }
 
 uint8_t SwOSOLED::getRotation(void)  {
   
   if (_display) return _display->getRotation( ); else return 0;
   
 }
 
 void SwOSOLED::setTextSize(uint8_t sx, uint8_t sy) {
   
   _textSizeX = sx;
   _textSizeY = sy;
   
   if (_display) _display->setTextSize( sx, sy );
   
 } 
 
 void SwOSOLED::getTextSize( uint8_t *sx, uint8_t *sy ) {
   *sx = _textSizeX;
   *sy = _textSizeY;
 }
 
 void SwOSOLED::drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y) {
   
   if (_display) _display->drawChar( x, y + YELLOWPIXELS, c, (color)?(SSD1306_WHITE):(0), (bg)?(SSD1306_WHITE):(0), size_x, size_y );
 
 } 
 
 void SwOSOLED::write( char *str ) {
   
   if (_display) _display->write(str);
   
 }
 
 void SwOSOLED::write( char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill ) { 
 
   // no display...
   if (!_display) return;
 
   int16_t x1, y1, x2, y2;
   uint16_t w, h;
   getTextBounds( str, 0, 0, &x1, &y1, &w, &h );
 
   switch (align) {
     case FTSWARM_ALIGNLEFT:   x2 = x;       y2 = y; break;
     case FTSWARM_ALIGNCENTER: x2 = x - w/2; y2 = y; break;
     case FTSWARM_ALIGNRIGHT:  x2 = x - w;   y2 = y; break;
     default:                  x2 = x;       y2 = y; break;
   }
 
   if (fill) _display->fillRect( x2, y2 + YELLOWPIXELS, w, h, 0 );
   setCursor( x2, y2 );
   write( str );
   
 }
 
 void SwOSOLED::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
 
   if (_display) {
     _display->getTextBounds( string, x, y + YELLOWPIXELS, x1, y1, w, h );
     *y1 -=  + YELLOWPIXELS;
   }
   
 } 
 
 int16_t SwOSOLED::getWidth(void)  {
   
   if (_display) return _display->width( ); else return 0;
   
 }
 
 int16_t SwOSOLED::getHeight(void) {
 
   if (_display) return _display->height( ) - YELLOWPIXELS; else return 0;
   
 }
