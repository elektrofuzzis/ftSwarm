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

#include "esp_event.h"
#include "esp_wifi.h"

/***************************************************
 *
 * wifiHandler
 *
 ***************************************************/

WifiHandler* wifiHandler = nullptr;

static void wifiEventHandler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data ) {

  if ( wifiHandler ) wifiHandler->eventHandler( arg, event_base, event_id, event_data );

}

WifiHandler::WifiHandler() {

  // ESP_ERROR_CHECK( esp_event_handler_instance_register( WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &wifiEventHandler, nullptr, nullptr ) );

  ESP_ERROR_CHECK( esp_event_handler_instance_register( WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, nullptr, nullptr ) );

}

WifiHandler::~WifiHandler() {

  if ( ap ) free( ap );

}

void WifiHandler::eventHandler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data ) {

  if ( event_base == WIFI_EVENT ) {
    
    switch ( event_id ) {
      
      case WIFI_EVENT_SCAN_DONE:          // Get the number of APs found
                                          esp_wifi_scan_get_ap_num( &aps );
                                          if ( ap ) free( ap );
                                          ap = (wifi_ap_record_t*) malloc( sizeof( wifi_ap_record_t ) * aps );

                                          // Fetch the actual records
                                          ESP_ERROR_CHECK( esp_wifi_scan_get_ap_records( &aps, ap ) );

                                          scanActive = false;
                                          break;

      case WIFI_EVENT_AP_STACONNECTED:    connectedDevices++;
                                          if ( connectedDevices >= MAX_AP_CONNECTIONS ) SWARM_LOG_WARN( TRANSLATE( "Maximum number of clients (%d) in AP mode reached.", "Maximale Anzahl an Clients (%d) im AP-Modus erreicht." ), MAX_AP_CONNECTIONS );
                                          break;

      case WIFI_EVENT_AP_STADISCONNECTED: if (connectedDevices) connectedDevices--;
                                          break;

    }

  }

}

void WifiHandler::startScan( void ) {

  // stopScan();
  if ( ap ) { free( ap ); ap = nullptr; }
  aps = 0;

  wifi_scan_config_t scan_config = { .show_hidden = false };
  scanActive = true;
  ESP_ERROR_CHECK( esp_wifi_scan_start( &scan_config, false ) );

}

void WifiHandler::stopScan( void ) {

  if (!scanActive) return;

  ESP_ERROR_CHECK( esp_wifi_scan_stop( ) );
  scanActive = false;

}

void WifiHandler::uniqueScanResult( void ) {

  wifi_ap_record_t temp;
  uint16_t i=1;

  while ( i < aps ) {

    int8_t cmp = strcmp( (char*) ap[i-1].ssid, (char*) ap[i].ssid );

    if ( ap[i].rssi <= -80 ) {
      // poor signal: kill item
      aps--;
      if ( i < aps ) memmove( &ap[i], &ap[i+1], ( aps - i ) * sizeof( wifi_ap_record_t ) );

    } else if ( cmp == 0 ) {
      // deduplicate, take the strongest
      if ( ap[i-1].rssi < ap[i].rssi ) memcpy( &ap[i-1], &ap[i],   sizeof( wifi_ap_record_t ) );
      aps--;
      if ( i < aps ) memmove( &ap[i], &ap[i+1], ( aps - i ) * sizeof( wifi_ap_record_t ) );      

    } else if ( cmp > 0 ) {
      // wrong order, change i-1 and i
      memcpy( &temp,    &ap[i-1], sizeof( wifi_ap_record_t ) );
      memcpy( &ap[i-1], &ap[i],   sizeof( wifi_ap_record_t ) );
      memcpy( &ap[i],   &temp,    sizeof( wifi_ap_record_t ) );

      // need to bubble up?
      if ( i>1 ) i--;

    } else {
      // nothing to deduplicate, order is fine, continue
      i++;
    }

  }

}

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
  if (!U8G2_SSD1306_128X64_NONAME_F_HW_I2C::begin() ) SWARM_LOG_ERROR( TRANSLATE( "OLED::begin failed.", "OLED::begin fehlgeschlagen." ) );

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
    screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] = getTextHeight() + 1;
    screenHeight[ FTSWARM_OLED_MAINSCREEN   ] = getDisplayHeight() - screenHeight[ FTSWARM_OLED_UPPERSCREEN ] - screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] ;
    screenOffset[ FTSWARM_OLED_BUTTONSCREEN ] = screenOffset[ FTSWARM_OLED_MAINSCREEN ] +screenHeight[ FTSWARM_OLED_MAINSCREEN ] + 2;
  } 
  
  if ( ( !activate ) && ( screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] != 0 ) ) {
    screenHeight[ FTSWARM_OLED_BUTTONSCREEN ] = 0;
    screenHeight[ FTSWARM_OLED_MAINSCREEN   ] = getDisplayHeight() - screenHeight[ FTSWARM_OLED_UPPERSCREEN ];
    screenOffset[ FTSWARM_OLED_BUTTONSCREEN ] = getDisplayHeight();
  }

}

void OLED::cls( FtSwarmOledScreen_t screen ) {

  if ( screen == FTSWARM_OLED_MAXSCREEN ) return;

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setMaxClipWindow();
  clipScreen = FTSWARM_OLED_MAXSCREEN;

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor(0);
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawBox( 0, screenOffset[screen], getWidth(), screenHeight[screen] );
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor(drawColor);

}

int16_t OLED::getScreenWidth(void)  {
   
  return U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getDisplayWidth( ); 
   
}
 
int16_t OLED::getScreenHeight( FtSwarmOledScreen_t screen ) {
 
  if ( screen != FTSWARM_OLED_MAXSCREEN  ) return screenHeight[screen];
  else return 0;

}

void OLED::setDrawColor( uint8_t color ) {

  if ( color > 2 ) return;
  drawColor = color;
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor( drawColor );

}

void OLED::drawButton( FtSwarmOledScreen_t screen, int16_t x, int16_t y, uint8_t width, const char *text, uint8_t flags, uint8_t paddingH, uint8_t paddingV ) {

  setClipping( screen );

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawButtonUTF8( x, translateY( screen, y ), flags, width, paddingH, paddingV, text );
  
  displayDirty = true;

}

void OLED::drawRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, int16_t h, FtSwarmOledFill_t fill ) {

  setClipping( screen );
  
  if ( setFill( fill ) ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawBox(   x, translateY( screen, y ), w, h ); resetDrawColor(); }
  else                     U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawFrame( x, translateY( screen, y ), w, h );
  
  displayDirty = true;

}

void OLED::drawRoundRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, FtSwarmOledFill_t fill ) {

  setClipping( screen );
  
  if ( setFill( fill ) ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawRBox(   x, translateY( screen, y ), w, h, r ); resetDrawColor(); }
  else                     U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawRFrame( x, translateY( screen, y ), w, h, r );
  
  displayDirty = true;

}

void OLED::drawCircle( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t r, FtSwarmOledFill_t fill ) {

  setClipping( screen );
  
  if ( setFill( fill ) ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawDisc(   x, translateY( screen, y ), r ); resetDrawColor(); }
  else                     U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawCircle( x, translateY( screen, y ), r );
  
  displayDirty = true;

}

void OLED::drawEllipse( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t rx, int16_t ry, FtSwarmOledFill_t fill ) {

  setClipping( screen );
  
  if ( setFill( fill ) ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawFilledEllipse( x, translateY( screen, y ), rx, ry ); resetDrawColor(); }
  else                     U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawEllipse( x, translateY( screen, y ), rx, ry );
  
  displayDirty = true;

}

void OLED::drawLine( FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1 ) {

  setClipping( screen );
  
  if      ( y0 == y1 ) U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawHLine( x0, translateY( screen, y0), x1-x0 );
  else if ( x0 == x1 ) U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawVLine( x0, translateY( screen, y0), y1-y0 );
  else    U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawLine(  x0, translateY( screen, y0), x1, translateY( screen, y1) );
  
  displayDirty = true;

}
 
void OLED::drawPixel( FtSwarmOledScreen_t screen, int16_t x, int16_t y ) {
   
  setClipping( screen );
  
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawPixel( x, translateY( screen, y ) );
  
  displayDirty = true;

}

void OLED::drawStr( FtSwarmOledScreen_t screen, int16_t x, int16_t y, const char *text, FtSwarmAlign_t align ) {

  setClipping( screen );
  
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

char* OLED::truncateText(const char* text, int16_t maxWidth) {
  
  if (!text) return nullptr;

  int16_t fullWidth = getStrWidth(text);

  // 1. Return a copy if text already fits
  if (fullWidth <= maxWidth) return strdup(text);

  // 2. Account for ellipsis width
  int16_t dotsWidth = getStrWidth("...");
  int16_t targetW = maxWidth - dotsWidth;

  // Return empty string if not even "..." fits
  if (targetW < 0) return strdup(""); 

  // 3. Prepare buffer (+3 for "...", +1 for null terminator)
  size_t originalLen = strlen(text);
  char* temp = (char*)malloc(originalLen + 4); 
  if (!temp) return nullptr;
  strcpy(temp, text);

  // 4. Fast Approximation (Ratio-based jump)
  // Reduce number of getStrWidth calls by jumping near the target length
  float ratio = (float)targetW / (float)fullWidth;
  size_t currentLen = (size_t)((float)originalLen * ratio);
  temp[currentLen] = '\0';

  // 5. Fine-tuning (Pixel-perfect adjustment)
  // If still too wide: trim characters one by one
  while (currentLen > 0 && getStrWidth(temp) > targetW) {
    temp[--currentLen] = '\0';
  }
    
  // If too short: add characters back until we hit the limit
  // Important for proportional fonts where 'i' is much thinner than 'W'
  while (currentLen < originalLen) {
    size_t nextLen = currentLen + 1;
    char nextChar = text[currentLen]; // Get next char from original
        
    temp[currentLen] = nextChar;
    temp[nextLen] = '\0';

    if (getStrWidth(temp) > targetW) {
      temp[currentLen] = '\0'; // Backtrack: too wide
      break;
    }
  
  currentLen = nextLen;
  
  }

  // 6. Append ellipsis and return
  strcat(temp, "...");
  return temp;

}

void OLED::drawStrRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, const char *text, FtSwarmAlign_t align, uint8_t paddingH, uint8_t paddingV, FtSwarmOledFill_t fill ) {

  char *temp = truncateText( text, w );

  int16_t xRect = 0;
  int16_t wTemp = getTextWidth( temp ) + 2*paddingH;
  switch (align) {
    case FTSWARM_ALIGNLEFT:   xRect = x;
                              break;
    case FTSWARM_ALIGNCENTER: xRect = x - wTemp/2;
                              break;
    case FTSWARM_ALIGNRIGHT:  xRect = x - wTemp;
                              break;
  }
  
  drawRect( screen, xRect, y, wTemp, getTextHeight() + 2*paddingV, fill );
  if ( fill == FTSWARM_OLED_FILLWHITE ) U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setDrawColor(0);
  drawStr( screen, xRect + paddingH, y + paddingV, temp, FTSWARM_ALIGNLEFT );
  resetDrawColor();

  free( temp );

}

void OLED::drawTriangle( FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, FtSwarmOledFill_t fill ) {
   
  setClipping( screen );
  
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

int16_t OLED::translateY( FtSwarmOledScreen_t screen, int16_t y ) {

  return y + screenOffset[ screen ] - screenScroll[ screen ];

}

void OLED::drawVSlider( FtSwarmOledScreen_t screen, int16_t maxHeight ) {

  if ( screen == FTSWARM_OLED_MAXSCREEN ) return;

  // Nothing to scroll if virtual height fits on screen
  if ( maxHeight <= screenHeight[screen] ) return;

  uint16_t screenH = getScreenHeight(screen);
  uint16_t width = getScreenWidth();

  // 1. Calculate the scale factor
  float scale = (float)screenH / (float)maxHeight;

  // 2. Calculate the indicator's position and height
  int16_t indicatorY = (int16_t)(screenScroll[screen] * scale);
  int16_t indicatorHeight = (int16_t)(screenH * scale);

  // Safety: ensure the indicator is at least 2 pixels high so it's visible
  if ( indicatorHeight < 2 ) indicatorHeight = 2;

  // 3. Drawing
  // Clear the slider area (2 pixels wide on the right edge)
  drawRect( screen, width - 2, 0, 2, screenH, FTSWARM_OLED_FILLBLACK );

  // Draw the indicator line (1 pixel wide at the very edge)
  // Must be drawn by native function, to avoid to be shifted by my own screen
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::drawVLine( width - 1, indicatorY + screenOffset[screen], indicatorHeight);
    
}

bool OLED::scroll(FtSwarmOledScreen_t screen, int16_t yLow, int16_t yHigh ) {

  if ( screen == FTSWARM_OLED_MAXSCREEN ) return false;

  int16_t currentScroll = screenScroll[screen];
  int16_t viewHeight = screenHeight[screen];

  // 1. Check if y is ABOVE the current visible window
  if ( yLow < currentScroll ) { screenScroll[screen] = yLow; return true; }

  // 2. Check if y is BELOW the current visible window
  if ( yHigh >= (currentScroll + viewHeight) ) { screenScroll[screen] = yHigh - viewHeight + 1; return true; };

  return false;

}

void OLED::setClipping( FtSwarmOledScreen_t screen ) {

  if ( ( clipScreen == screen )|| ( screen == FTSWARM_OLED_MAXSCREEN ) ) return;
  
  clipScreen = screen;

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C::setClipWindow( 0, screenOffset[screen], getScreenWidth(), screenOffset[screen] + screenHeight[screen] );

}

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
  ledc_channel_config_t ledc = {
    .gpio_num       = DISCRETE_RGB_RED,
    .speed_mode     = LEDC_LOW_SPEED_MODE,
    .channel        = (ledc_channel_t) DISCRETE_RGB_BASE_CHANNEL,
    .intr_type      = LEDC_INTR_DISABLE,
    .timer_sel      = LEDC_TIMER_0,
    .duty           = 0,
    .hpoint         = 0,
    .flags = {
      .output_invert = 1
    }
  };

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

void RGBLed::setColor( CRGB color ) {

  this->color = color;

  set();

}

void RGBLed::setBrightness( uint8_t brightness ) {

  this->brightness = brightness;
  set();

}

void RGBLed::set( void ) {

  setPWM( 0, scale8_video( color.r, brightness ) );
  setPWM( 1, scale8_video( color.g, brightness ) );
  setPWM( 2, scale8_video( color.b, brightness ) );
  
}

RGBLed *rgbLed = NULL;

#endif