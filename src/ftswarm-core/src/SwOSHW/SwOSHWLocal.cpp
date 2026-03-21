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

#if FTSWARM_HAL_HC165 > 0

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

#define FTSWARM_HAL_OLEDS 1

#if FTSWARM_HAL_OLEDS > 0

static void displayTaskWrapper( void *parameter ) {

  oled.displayTask();

}

bool OLED::setFill( FtSwarmOledFill_t fill ) {

  switch ( fill ) {
    case FTSWARM_OLED_FILLBLACK: U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor( 0 ); return true;
    case FTSWARM_OLED_FILLWHITE: U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor( 1 ); return true;
  }

  return false;

}

void OLED::begin( void ) {

  if (initialized) return;
  
  // startup hardware
  if (!U8G2_SSD1306_128X64_NONAME_F_HW_I2C::begin() ) SWARM_LOG_ERROR( "OLED::begin failed.");

  // Font
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setFont(u8g2_font_6x10_tr);
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setFontPosTop();

  // colors
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor( drawColor );

  // transfer task
  xTaskCreatePinnedToCore( displayTaskWrapper, "displayTask", 10000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );

  initialized = true;

}

void OLED::displayTask( void ) {

  while (1) {

    if ( displayDirty ) { displayDirty = false; U8G2_SSD1306_128X64_NONAME_F_HW_I2C::sendBuffer(); }

    delay(100);

  }

} 
 
void OLED::dim(bool dim) {

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setContrast( dim ? 1 : 0x8F );
 
  /* legacy adafruit code
  // send set contrast
  Wire.beginTransmission( 0x3C );
  Wire.write( (uint8_t) 0 );
  Wire.write( 0x81 );
  Wire.endTransmission();
 
  // send contast value
  Wire.beginTransmission( 0x3C );
  Wire.write( (uint8_t) 0 );
  Wire.write( dim ? 1 : 0x8F );
  Wire.endTransmission();
  */
   
}

void OLED::buttonScreen( bool activate ) {

  if ( ( activate ) && ( screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] == 0 ) ) {
    screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] = getTextHeight() + 2;
    screenHeight[ FTSWARM_OLED_MAINSCREEN   ] = getDisplayHeight() - screenHeight[ FTSWARM_OLED_UPPERSCREEN ] - screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] ;
    screenOffset[ FTSWARM_OLED_BUTTONSCREEN ] = screenOffset[ FTSWARM_OLED_MAINSCREEN ] +screenHeight[ FTSWARM_OLED_MAINSCREEN ];
  } 
  
  if ( ( !activate ) && ( screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] != 0 ) ) {
    screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] = 0;
    screenHeight[ FTSWARM_OLED_MAINSCREEN   ] = getDisplayHeight() - screenHeight[ FTSWARM_OLED_UPPERSCREEN ];
    screenOffset[ FTSWARM_OLED_BUTTONSCREEN ] = getDisplayHeight();
  }

}

void OLED::cls( uint8_t screen ) {

  if ( screen >= MAXOLEDSCREENS ) return;

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor(0);
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawBox( 0, screenOffset[screen], getWidth(), screenHeight[screen] );
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor(drawColor);

}

int16_t OLED::getScreenWidth(void)  {
   
  return U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getDisplayWidth( ); 
   
}
 
int16_t OLED::getScreenHeight( uint8_t screen ) {
 
  if ( screen < MAXOLEDSCREENS ) return screenHeight[screen];
  else return 0;

}

void OLED::setDrawColor( uint8_t color ) {

  if ( color > 2 ) return;
  drawColor = color;
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor( drawColor );

}

void OLED::drawButton( uint8_t screen, int16_t x, int16_t y, uint8_t width, const char *text, uint8_t flags, uint8_t paddingH, uint8_t paddingV ) {

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawButtonUTF8( x, translateY( screen, y ), flags, width, paddingH, paddingV, text );
  displayDirty = true;

}

void OLED::drawRect( uint8_t screen, int16_t x, int16_t y, int16_t w, int16_t h, FtSwarmOledFill_t fill ) {

  if ( setFill( fill ) ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawBox(   x, translateY( screen, y ), w, h ); resetDrawColor(); }
  else                     U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawFrame( x, translateY( screen, y ), w, h );
  displayDirty = true;

}

void OLED::drawRoundRect( uint8_t screen, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, FtSwarmOledFill_t fill ) {

  if ( setFill( fill ) ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawRBox(   x, translateY( screen, y ), w, h, r ); resetDrawColor(); }
  else                     U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawRFrame( x, translateY( screen, y ), w, h, r );
  displayDirty = true;

}

void OLED::drawCircle( uint8_t screen, int16_t x, int16_t y, int16_t r, FtSwarmOledFill_t fill ) {

  if ( setFill( fill ) ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawDisc(   x, translateY( screen, y ), r ); resetDrawColor(); }
  else                     U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawCircle( x, translateY( screen, y ), r );
  displayDirty = true;

}

void OLED::drawEllipse( uint8_t screen, int16_t x, int16_t y, int16_t rx, int16_t ry, FtSwarmOledFill_t fill ) {

  if ( setFill( fill ) ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawFilledEllipse( x, translateY( screen, y ), rx, ry ); resetDrawColor(); }
  else                     U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawEllipse( x, translateY( screen, y ), rx, ry );
  displayDirty = true;
}

void OLED::drawLine( uint8_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1 ) {

  if      ( y0 == y1 ) U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawHLine( x0, translateY( screen, y0), x1-x0 );
  else if ( x0 == x1 ) U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawVLine( x0, translateY( screen, y0), y1-y0 );
  else                 U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawLine(  x0, translateY( screen, y0), x1, translateY( screen, y1) );
  displayDirty = true;

}
 
void OLED::drawPixel( uint8_t screen, int16_t x, int16_t y ) {
   
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawPixel( x, translateY( screen, y ) );
  displayDirty = true;

}

void OLED::drawStr( uint8_t screen, int16_t x, int16_t y, const char *text, FtSwarmAlign_t align ) {

  int16_t width = U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getStrWidth( text );

  int16_t x1;
  switch ( align ) {
    case FTSWARM_ALIGNLEFT:   x1 = x;                break;
    case FTSWARM_ALIGNCENTER: x1 = x - ( width / 2); break;
    case FTSWARM_ALIGNRIGHT:  x1 = x - width;        break;
    default:                  x1 = x;                break;
  }

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawStr( x1, translateY( screen, y ), text );
  displayDirty = true;

}

void OLED::drawStrRect( uint8_t screen, int16_t x, int16_t y, int16_t w, const char *text, FtSwarmAlign_t align, uint8_t paddingH, uint8_t paddingV, FtSwarmOledFill_t fill ) {

  int16_t width = strlen( text ) * U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getStrWidth( text );

  // calculate number of chars, which could be printed
  uint8_t maxChars = strlen( text );
  if ( w > width ) maxChars = ( (float) width / (float) (w+2) ) * maxChars;

  // copy str->temp in right size
  char *temp = (char*) calloc( maxChars + 1, sizeof( char) );
  strncpy( temp, text, maxChars );

  drawRect( screen, x, y, w+2*paddingH, getTextHeight() + 2*paddingV, fill );
  if ( fill == FTSWARM_OLED_FILLWHITE ) U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor(0);
  drawStr( screen, x + paddingH, y + paddingV, temp, align );
  resetDrawColor();

  free( temp );

}

void OLED::drawTriangle( uint8_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, FtSwarmOledFill_t fill ) {
   
  if ( setFill(fill) ) {
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawTriangle( x0, translateY( screen, y0 ), x1, translateY( screen, y1 ), x2, translateY( screen, y2 ) ); 
    resetDrawColor();
  
  } else {
    drawLine( screen, x0, y0, x1, y1 );
    drawLine( screen, x1, y1, x2, y2 );
    drawLine( screen, x2, y2, x0, y0 );
  }

  displayDirty = true;
 
} 

int16_t OLED::translateY( uint8_t screen, int16_t y ) {

  // out-of-bounds
  if (screen >=3 ) return y;

  return y + screenOffset[ screen ] + screenScroll[ screen ];

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

OLED oled;

/***************************************************
 *
 *   RGBLed
 *
 ***************************************************/

#if FTSWARM_HAL_DISCRETE_RGBS > 0

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