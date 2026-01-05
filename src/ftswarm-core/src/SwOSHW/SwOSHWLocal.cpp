/*
 * SwOSHWLocal.cpp
 *
 * Local Hardware
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SwOSHWHAL.h"
#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSLog.h"

// local HC165
HC165 *hc165 = NULL;

/***************************************************
 *
 *   HC165
 *
 ***************************************************/

HC165::HC165( FtSwarmVersion_t CPU ) {

  // initialize local HW

  switch ( CPU ) {
    case FTSWARMCONTROL_1V3UC: CS   = GPIO_NUM_10;
                               LD   = xGPIO_NUM_47;
                               CLK  = xGPIO_NUM_48;
                               MISO = GPIO_NUM_15;
                               break;

    case FTSWARMCONTROL_1V3:   CS   = GPIO_NUM_14;
                               LD   = GPIO_NUM_15;
                               CLK  = GPIO_NUM_12;
                               MISO = GPIO_NUM_35;
                               break;

    default:                   CS = LD = CLK = MISO = GPIO_NUM_NC;
                               return;
  }

  // initialize ports
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pin_bit_mask = (1ULL<<CS) | (1ULL<<LD) | (1ULL<<CLK) ;
  gpio_config(&io_conf);

  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = 1ULL<<MISO ;
  gpio_config(&io_conf);

  // set levels
  gpio_set_level( CS, 1 );
  gpio_set_level( LD, 1 );
  gpio_set_level( CLK, 1 );

}

void HC165::operate( ) {

  // invalid configuration?
  if (LD == GPIO_NUM_NC ) {
    return;
  }

  // parallel load
  gpio_set_level( LD, 0 );
  gpio_set_level( LD, 1 );

  // enable
  gpio_set_level( CS, 0 );

  // load
  lastValue = 0;
  for ( uint8_t i=0; i<8; i++ ) {

    // get value
    lastValue = ( lastValue << 1 ) | (!gpio_get_level( MISO ));

    // one tick
    gpio_set_level( CLK, 0 );
    gpio_set_level( CLK, 1 );

  }

}

/***************************************************
 *
 *   OLED
 *
 ***************************************************/

static void displayTask( void *parameter ) {

  while (1) {

    if (oled) oled->flush();

    delay(100);

  }

}

/*
  display->setTextSize(3,3);            // Logo
  write( (char *) "ftSwarm", getWidth()/2, 0, FTSWARM_ALIGNCENTER, true );
 
  display->setTextSize(1,1);            // hostname & version
  char line[100];
  sprintf( line, "%s %s", ctrl->getHostname(), SWOSVERSION );
  write( line, getWidth()/2, 32, FTSWARM_ALIGNCENTER, true );
 
  // additional default values
  display->setTextSize(1, 1);           // Normal 1:1 pixel scale
  display->setCursor(0, 0);             // Start at top-left corner
 
  dim(true);
  displayDirty = true;

*/

OLED::OLED( void ) {
  
  // startup hardware
  display = new Adafruit_SSD1306 (128, 64, &Wire, -1);

  if ( !display->begin(SSD1306_SWITCHCAPVCC, 0x3C ) ) {
    delete display;
    display = NULL;
    SWARM_LOG_ERROR( "Couldn't initialize OLED display." );
    return;
  }

  // transfer task
  xTaskCreatePinnedToCore( displayTask, "displayTask", 10000, NULL, 1, NULL, SWOSCORE );

}

void OLED::flush( void ) { 
  
  if ( (display) && (displayDirty) ) { 
    
    display->display(); 
    displayDirty = false; 
  
  }

}

void OLED::invertDisplay(bool i) {
 
  if (display) {
    display->invertDisplay( i );
    displayDirty = true;
  }
   
}
 
void OLED::fillScreen(bool white) {
   
  drawRect( 0, 0, getWidth(), getHeight(), true, white );
   
} 
 
void OLED::dim(bool dim) {
 
  // origin adafruit code fails with some displays
  // if (display) display->dim( dim );
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
 
void OLED::drawPixel(int16_t x, int16_t y, bool white ) {
   
  if (display) {
    display->drawPixel( x, y + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0) );
    displayDirty = true;
  }

}
 
void OLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white) {
   
  if (display) {
    if      (x0==x1) display->drawFastVLine( x0, y0 + YELLOWPIXELS, y1 - y0,               (white)?(SSD1306_WHITE):(0) ); 
    else if (y0==y1) display->drawFastHLine( x0, y0 + YELLOWPIXELS, x1 - x0,               (white)?(SSD1306_WHITE):(0) ); 
    else             display->drawLine(      x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0) );
    displayDirty = true;
  }

} 
 
void OLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white) {
 
  if (display) {
    if (fill) display->fillRect( x, y + YELLOWPIXELS, w, h, (white)?(SSD1306_WHITE):(0) );
    else      display->drawRect( x, y + YELLOWPIXELS, w, h, (white)?(SSD1306_WHITE):(0) );
    displayDirty = true;
  }
   
}
 
void OLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white) {
   
  if (display) {
    if (fill) display->fillRoundRect( x0, y0 + YELLOWPIXELS, w, h, radius, (white)?(SSD1306_WHITE):(0)); 
    else      display->drawRoundRect( x0, y0 + YELLOWPIXELS, w, h, radius, (white)?(SSD1306_WHITE):(0)); 
    displayDirty = true;
  }
   
} 
 
 
void OLED::drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white) {
   
  if (display) {
    if (fill) display->fillCircle( x0, y0 + YELLOWPIXELS, r, (white)?(SSD1306_WHITE):(0)); 
    else      display->drawCircle( x0, y0 + YELLOWPIXELS, r, (white)?(SSD1306_WHITE):(0)); 
    displayDirty = true;
  }
   
} 
 
void OLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white) {
   
  if (display) {
    if (fill) display->fillTriangle( x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, x2, y2 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0)); 
    else      display->drawTriangle( x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, x2, y2 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0)); 
    displayDirty = true;
  }
 
} 
 
void OLED::setCursor(int16_t x, int16_t y) {
   
  if (display) {
    display->setCursor( x, y  + YELLOWPIXELS);
    displayDirty = true;
  }
   
}
 
void OLED::getCursor(int16_t *x, int16_t *y) {
   
  if (display) {
    *x = display->getCursorX( );
    *y = display->getCursorY( );
  }
   
}
 
void OLED::setTextColor(bool c, bool bg) {

  color      = c;
  background = bg;
   
  if (display) {
    display->setTextColor( (c)?(SSD1306_WHITE):(0), (bg)?(SSD1306_WHITE):(0) );
    displayDirty = true;
  }

}

void OLED::setTextWrap( bool w ) {
   
  if (display) display->setTextWrap( w );
   
}
 
void OLED::setRotation(uint8_t r) {
   
  if (display) {
    display->setRotation( r );
    displayDirty = true;
  }
   
}
 
uint8_t OLED::getRotation(void)  {
   
  if (display) return display->getRotation( ); 
  else         return 0;
   
}
 
void OLED::setTextSize(uint8_t sx, uint8_t sy) {
   
  textSizeX = sx;
  textSizeY = sy;
   
  if (display) display->setTextSize( sx, sy );
   
} 
 
void OLED::getTextSize( uint8_t *sx, uint8_t *sy ) {

  *sx = textSizeX;
  *sy = textSizeY;

}
 
void OLED::drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y) {
   
  if (display) {
    display->drawChar( x, y + YELLOWPIXELS, c, (color)?(SSD1306_WHITE):(0), (bg)?(SSD1306_WHITE):(0), size_x, size_y );
    displayDirty = true;
  }
 
} 
 
void OLED::write( const char *str ) {
   
  if (display) {

    display->write(str);
    displayDirty = true;

  }
   
}
 
void OLED::write( const char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill, bool invert ) { 
 
  // no display...
  if (!display) return;
 
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

  if (fill) display->fillRect( x2-1, y2-1 + YELLOWPIXELS, w+2, h+1, invert );
  setCursor( x2+1, y2 );
  write( str );

  if (invert) setTextColor( c, bg );

}
 
void OLED::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
 
  if (display) {
    display->getTextBounds( string, x, y + YELLOWPIXELS, x1, y1, w, h );
    *y1 -=  + YELLOWPIXELS;
  }
   
} 
 
int16_t OLED::getWidth(void)  {
   
  if (display) return display->width( ); else return 0;
   
}
 
int16_t OLED::getHeight(void) {
 
  if (display) return display->height( ) - YELLOWPIXELS; else return 0;
   
}

OLED *oled  = NULL;
