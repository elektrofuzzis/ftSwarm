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

// classic RGB LED
#define GPIO_RED         GPIO_NUM_4
#define GPIO_GREEN       GPIO_NUM_5
#define GPIO_BLUE        GPIO_NUM_10
#define LED_BASE_CHANNEL LEDC_CHANNEL_4

class RGBLed {

  protected:

    uint8_t  brightness  = 16;
    uint32_t color = 0;
    void setPWM( uint8_t c, uint32_t duty );

  public:

    RGBLed();
    void setColor( uint32_t color );
    void setBrightness( uint8_t brightness );

};

RGBLed::RGBLed() {

  // initialize local HW
  ledc_channel_config_t ledc;
  ledc.speed_mode     = LEDC_LOW_SPEED_MODE;
  ledc.intr_type      = LEDC_INTR_DISABLE;
  ledc.timer_sel      = LEDC_TIMER_0;
  ledc.duty           = 0; 
  ledc.hpoint         = 0;
  ledc.flags.output_invert = 1;

  ledc.gpio_num       = GPIO_RED;
  ledc.channel        = (ledc_channel_t) LED_BASE_CHANNEL;
  ESP_ERROR_CHECK( ledc_channel_config( &ledc ) );

  ledc.gpio_num       = GPIO_GREEN;
  ledc.channel        = (ledc_channel_t) (LED_BASE_CHANNEL+1);
  ESP_ERROR_CHECK( ledc_channel_config( &ledc ) );

  ledc.gpio_num       = GPIO_BLUE;
  ledc.channel        = (ledc_channel_t) (LED_BASE_CHANNEL+2);
  ESP_ERROR_CHECK( ledc_channel_config( &ledc ) );

}

void RGBLed::setPWM( uint8_t c, uint32_t duty ) {

  ledc_channel_t channel = (ledc_channel_t) (LED_BASE_CHANNEL+c);

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

SwOSPixel::SwOSPixel(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl, SWOSIO_PIXEL ) {

  if (ctrl->isLocal()) 
    setupLocal();

}

RGBLed *rgbLed = NULL;

void SwOSPixel::setupLocal() {

  // leds[] need to be initialized only once.
  if (!ledsInitialized) {

    // assign port to GPIO.
    switch ( ctrl->getCPU() ) {
      #if CONFIG_IDF_TARGET_ESP32S3
        case FTSWARMPWRDRIVE_1V141:
        case FTSWARMDUINO_1V141:
        case FTSWARMXL_1V00:
        case FTSWARMCAM_3V12:
        case FTSWARMRS_2V1:
        case FTSWARMRS_2V0:         FastLED.addLeds<WS2812, xGPIO_NUM_48, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); 
                                    break;

        case FTSWARMRC_1V141:       FastLED.addLeds<WS2812, xGPIO_NUM_48, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); 
                                    rgbLed = new RGBLed();
                                    break;
      #endif
      case FTSWARMJST_1V15:         FastLED.addLeds<WS2812, GPIO_NUM_26, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); 
                                    break;

      default:                      FastLED.addLeds<WS2812, GPIO_NUM_33, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); 
                                    break;
    }

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

  if ( (rgbLed) && ( port == 0 ) ) {
      rgbLed->setColor( color );
      return;
  }

  led[(rgbLed)?port-1:port] = color;
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

  if ( (rgbLed) && ( port == 0 ) ) {
      rgbLed->setBrightness( brightness );
      return;
  }

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

SwOSOLED::SwOSOLED(const char *name, SwOSCtrl *ctrl) : SwOSIO( name, ctrl, SWOSIO_OLED ) {
 
  if ( ctrl->isLocal() ) { 
    setupLocal(); 
  }
 
}
 
void SwOSOLED::setupLocal() {
 
  // startup hardware
  if (!oled) oled = new OLED( );
  if (!oledMenu) oledMenu = new OLEDMenu( ctrl );

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
