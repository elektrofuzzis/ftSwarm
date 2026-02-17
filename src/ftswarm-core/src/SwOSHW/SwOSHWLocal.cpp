/*
 * SwOSHWLocal.cpp
 *
 * Local Hardware
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSLog.h"

/***************************************************
 *
 *   HC165
 *
 ***************************************************/

#if FTSWARM_HAL_HAS_HC165 > 0

// local HC165
HC165 *hc165 = NULL;

HC165::HC165( FtSwarmVersion_t CPU ) {

  // initialize local HW

  // initialize ports
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pin_bit_mask = (1ULL<<HC165_CS) | (1ULL<<HC165_LD) | (1ULL<<HC165_CLK) ;
  gpio_config(&io_conf);

  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = 1ULL<<HC165_MISO ;
  gpio_config(&io_conf);

  // set levels
  gpio_set_level( HC165_CS, 1 );
  gpio_set_level( HC165_LD, 1 );
  gpio_set_level( HC165_CLK, 1 );

}

void HC165::operate( ) {

  // invalid configuration?
  if (HC165_LD == GPIO_NUM_NC ) {
    return;
  }

  // parallel load
  gpio_set_level( HC165_LD, 0 );
  gpio_set_level( HC165_LD, 1 );

  // enable
  gpio_set_level( HC165_CS, 0 );

  // load
  lastValue = 0;
  for ( uint8_t i=0; i<8; i++ ) {

    // get value
    lastValue = ( lastValue << 1 ) | (!gpio_get_level( HC165_MISO ));

    // one tick
    gpio_set_level( HC165_CLK, 0 );
    gpio_set_level( HC165_CLK, 1 );

  }

}

#endif

/***************************************************
 *
 *   OLED
 *
 ***************************************************/

#if FTSWARM_HAL_HAS_OLED > 0

static void displayTask( void *parameter ) {

  while (1) {

    if (oled) oled->flush();

    delay(100);

  }

}

OLED::OLED( void ) {
  
  // startup hardware
  // display = new Adafruit_SSD1306 (128, 64, &Wire, -1);

  if ( !display.begin(SSD1306_SWITCHCAPVCC, 0x3C ) ) {
    SWARM_LOG_ERROR( "Couldn't initialize OLED display." );
    return;
  }

  // transfer task
  xTaskCreatePinnedToCore( displayTask, "displayTask", 10000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );

}

void OLED::flush( void ) { 
  
  if ( displayDirty ) { 
    
    display.display(); 
    displayDirty = false; 
  
  }

}

void OLED::invertDisplay(bool i) {
 
  display.invertDisplay( i );
  displayDirty = true;
   
}
 
void OLED::fillScreen(bool white) {
   
  drawRect( 0, 0, getWidth(), getHeight(), true, white );
   
} 
 
void OLED::dim(bool dim) {
 
  // origin adafruit code fails with some displays
  // if (display) display.dim( dim );
  setContrast( dim ? 1 : 0x8F );
   
}
 
void OLED::setContrast(uint8_t contrast) {
 
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

void OLED::clearDisplay( bool fullscreen ) {

  if ( fullscreen ) display.clearDisplay();
  else drawRect( 0, 0, getWidth()-1, getHeight()-1, true, SSD1306_BLACK );
  
}

void OLED::cp437( bool x ) { 
  
  display.cp437( x ); 

}
 
void OLED::drawPixel(int16_t x, int16_t y, bool white ) {
   
  display.drawPixel( x, y + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0) );
  displayDirty = true;

}
 
void OLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white) {
   
  if      (x0==x1) display.drawFastVLine( x0, y0 + YELLOWPIXELS, y1 - y0,               (white)?(SSD1306_WHITE):(0) ); 
  else if (y0==y1) display.drawFastHLine( x0, y0 + YELLOWPIXELS, x1 - x0,               (white)?(SSD1306_WHITE):(0) ); 
  else             display.drawLine(      x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0) );
  displayDirty = true;

} 
 
void OLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white) {
 
  if (fill) display.fillRect( x, y + YELLOWPIXELS, w, h, (white)?(SSD1306_WHITE):(0) );
  else      display.drawRect( x, y + YELLOWPIXELS, w, h, (white)?(SSD1306_WHITE):(0) );
  displayDirty = true;
   
}
 
void OLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white) {
   
  if (fill) display.fillRoundRect( x0, y0 + YELLOWPIXELS, w, h, radius, (white)?(SSD1306_WHITE):(0)); 
  else      display.drawRoundRect( x0, y0 + YELLOWPIXELS, w, h, radius, (white)?(SSD1306_WHITE):(0)); 
  displayDirty = true;
   
} 
 
 
void OLED::drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white) {
   
  if (fill) display.fillCircle( x0, y0 + YELLOWPIXELS, r, (white)?(SSD1306_WHITE):(0)); 
  else      display.drawCircle( x0, y0 + YELLOWPIXELS, r, (white)?(SSD1306_WHITE):(0)); 
  displayDirty = true;
   
} 
 
void OLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white) {
   
  if (fill) display.fillTriangle( x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, x2, y2 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0)); 
  else      display.drawTriangle( x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, x2, y2 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0)); 
  displayDirty = true;
 
} 
 
void OLED::setCursor(int16_t x, int16_t y) {
   
  display.setCursor( x, y  + YELLOWPIXELS);
  displayDirty = true;
   
}
 
void OLED::getCursor(int16_t *x, int16_t *y) {
   
  *x = display.getCursorX( );
  *y = display.getCursorY( );
   
}
 
void OLED::setTextColor(bool c, bool bg) {

  color      = c;
  background = bg;
   
  display.setTextColor( (c)?(SSD1306_WHITE):(0), (bg)?(SSD1306_WHITE):(0) );
  displayDirty = true;

}

void OLED::setTextWrap( bool w ) {
   
  display.setTextWrap( w );
   
}
 
void OLED::setRotation(uint8_t r) {
   
  display.setRotation( r );
  displayDirty = true;
   
}
 
uint8_t OLED::getRotation(void)  {
   
  return display.getRotation( ); 
   
}
 
void OLED::setTextSize(uint8_t sx, uint8_t sy) {
   
  textSizeX = sx;
  textSizeY = sy;
   
  display.setTextSize( sx, sy );
   
} 
 
void OLED::getTextSize( uint8_t *sx, uint8_t *sy ) {

  *sx = textSizeX;
  *sy = textSizeY;

}
 
void OLED::drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y) {
   
  display.drawChar( x, y + YELLOWPIXELS, c, (color)?(SSD1306_WHITE):(0), (bg)?(SSD1306_WHITE):(0), size_x, size_y );
  displayDirty = true;
 
} 
 
void OLED::write( const char *str ) {
   
  display.write(str);
  displayDirty = true;
   
}
 
void OLED::write( const char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill, bool invert ) { 
 
  int16_t x1, y1, x2, y2;
  uint16_t w, h;
  getTextBounds( str, 0, 0, &x1, &y1, &w, &h );
 
  switch (align) {

    case FTSWARM_ALIGNLEFT:   x2 = x;       y2 = y; break;
    case FTSWARM_ALIGNCENTER: x2 = x - w/2; y2 = y; break;
    case FTSWARM_ALIGNRIGHT:  x2 = x - w;   y2 = y; break;
    default:                  x2 = x;       y2 = y; break;
  }

  bool c = color;
  bool bg = background;
 
  if (invert) setTextColor( !c, !bg );

  if (fill) display.fillRect( x2-1, y2-1 + YELLOWPIXELS, w+2, h+1, invert );
  setCursor( x2+1, y2 );
  write( str );

  if (invert) setTextColor( c, bg );

}
 
void OLED::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
 
  display.getTextBounds( string, x, y + YELLOWPIXELS, x1, y1, w, h );
  *y1 -=  + YELLOWPIXELS;
   
} 
 
int16_t OLED::getWidth(void)  {
   
  return display.width( ); 
   
}
 
int16_t OLED::getHeight(void) {
 
  return display.height( ) - YELLOWPIXELS; 
   
}

#else

// no local hardware - just define stubs

OLED::OLED( void ) {};
void OLED::flush( void ) {};
void OLED::invertDisplay(bool i) {};
void OLED::fillScreen( bool white) {};
void OLED::dim(bool dim) {};
void OLED::setContrast(uint8_t contrast ) {};
int16_t OLED::getWidth(void) { return 0; };
int16_t OLED::getHeight(void) { return 0; };
void OLED::clearDisplay( bool fullscreen) {};
void OLED::cp437( bool x ) {};
void OLED::drawPixel(int16_t x, int16_t y, bool white) {}; 
void OLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white) {};
void OLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white) {};
void OLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white) {};
void OLED::drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white) {};
void OLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white) {};
void OLED::drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y) {};
void OLED::write( const char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill, bool invert ) {};
void OLED::write( const char *str ) {};
void OLED::setCursor(int16_t x, int16_t y) {};
void OLED::getCursor(int16_t *x, int16_t *y) {};
void OLED::setTextColor( bool c,  bool bg) {};
void OLED::setTextWrap(bool w) {};
void OLED::setRotation(uint8_t r) {};
uint8_t OLED::getRotation(void) { return 0; };
void OLED::setTextSize(uint8_t sx, uint8_t sy) {};
void OLED::getTextSize( uint8_t *sx, uint8_t *sy ) {};
void OLED::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {};

#endif

OLED *oled  = NULL;

/***************************************************
 *
 *   RGBLed
 *
 ***************************************************/

#if FTSWARM_HAL_HAS_DISCRETE_RGB > 0

RGBLed::RGBLed() {
  
  // initialize local HW
  ledc_channel_config_t ledc;
  ledc.speed_mode     = LEDC_LOW_SPEED_MODE;
  ledc.intr_type      = LEDC_INTR_DISABLE;
  ledc.timer_sel      = LEDC_TIMER_0;
  ledc.duty           = 0; 
  ledc.hpoint         = 0;
  ledc.flags.output_invert = 1;

  ledc.gpio_num       = DISCRETE_RGB_RED;
  ledc.channel        = (ledc_channel_t) DISCRETE_RGB_BASE_CHANNEL;
  ESP_ERROR_CHECK( ledc_channel_config( &ledc ) );

  ledc.gpio_num       = DISCRETE_RGB_GREEN;
  ledc.channel        = (ledc_channel_t) (DISCRETE_RGB_BASE_CHANNEL+1);
  ESP_ERROR_CHECK( ledc_channel_config( &ledc ) );

  ledc.gpio_num       = DISCRETE_RGB_BLUE;
  ledc.channel        = (ledc_channel_t) (DISCRETE_RGB_BASE_CHANNEL+2);
  ESP_ERROR_CHECK( ledc_channel_config( &ledc ) );

}

void RGBLed::setPWM( uint8_t c, uint32_t duty ) {

  ledc_channel_t channel = (ledc_channel_t) (DISCRETE_RGB_BASE_CHANNEL+c);

  ESP_ERROR_CHECK( ledc_set_duty( LEDC_LOW_SPEED_MODE, channel, duty ) );
  ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, channel ) );

}

void RGBLed::setColor( uint32_t color ) {

  this->color = color;

  setPWM( 0, ( ( color >> 16 ) & 0xFF ) * brightness);
  setPWM( 1, ( ( color >> 8  ) & 0xFF ) * brightness );
  setPWM( 2, (   color         & 0xFF ) * brightness );

}

void RGBLed::setBrightness( uint8_t brightness ) {

  this->brightness = brightness / 16;
  setColor( color );

}

RGBLed *rgbLed = NULL;

#endif