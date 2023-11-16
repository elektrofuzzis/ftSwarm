/*
 * SwOSExtLibs.h
 *
 * Abstraction Layer to switch between needed 3rd party HW libraries  
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#define COMPILE_FTSWARM
// #define COMPILE_FTSWARMRS 1
// #define COMPILE_FTSWARMCONTROL
// #define COMPILE_FTSWARMCAM
// #define COMPILE_FTSWARMPWRDRIVE
// #define COMPILE_FTSWARMDUINO


#if !(defined( COMPILE_FTSWARM ) || defined( COMPILE_FTSWARMRS ) || defined( COMPILE_FTSWARMCONTROL ) || defined( COMPILE_FTSWARMDUINO ) || defined( COMPILE_FTSWARMCAM ) || defined( COMPILE_FTSWARMPWRDRIVE ) )
  #error "no platform defined"
#endif

#include <FastLED.h>

/*
// FastLED
#if defined(COMPILE_FTSWARM) || defined(COMPILE_FTSWARMRS)|| defined(COMPILE_FTSWARMPWRDRIVE)|| defined(COMPILE_FTSWARMDUINO)

  #include <FastLED.h>

  #define FASTLED_SETBRIGHTNESS( b ) ( FastLED.setBrightness( b ) )
  #define FASTLED_SHOW               ( FastLED.show )
  #define FASTLED_CRGB               CRGB
  #define FASTLED_ADDLEDS( gpio, l, m ) ( FastLED.addLeds<WS2812, gpio, GRB>(l, m).setCorrection( TypicalLEDStrip ) );

#else

  #define FASTLED_SETBRIGHTNESS( b )    0
  #define FastLED_SHOW()                0
  #define FASTLED_ADDLEDS( gpio, l, m ) 0
  #define FASTLED_CRGB                  uint32_t
    
#endif
*/

#include <LSM6DSRSensor.h>
#include <ftPwrDrive/ftPwrDrive.h>
#include <esp_camera.h>

// OLED

#ifdef COMPILE_FTSWARMCONTROL

  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

#else

  #define SSD1306_WHITE 1
  #define SSD1306_SWITCHCAPVCC 0x02
  
  class Adafruit_SSD1306 {
    public:
      int16_t height( void ) { return 0; };
      int16_t width( void )  { return 0; };
      void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {};
      void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {};
      size_t write(char *) { return 0; };
      void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) {};
      void setTextSize(uint8_t s) {};
      void setTextSize(uint8_t sx, uint8_t sy) {};
      uint8_t getRotation(void) { return 0; };
      void setTextWrap(bool w) {};
      void setTextColor(uint16_t c) {};
      void setTextColor(uint16_t c, uint16_t bg) {};
      void stopscroll(void) {};
      int16_t getCursorX(void) { return 0; };
      int16_t getCursorY(void) { return 0; };
      void setCursor(int16_t x, int16_t y) {};
      void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {};
      void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {};
      void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {};
      void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {};
      void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) {};
      void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) {};
      void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {};
      void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {};
      void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {};
      void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {};
      void invertDisplay(bool i) {};
      void drawPixel(int16_t x, int16_t y, uint16_t color) {};
      void cp437(bool x = true) {};
      void display(void) {};
      void clearDisplay(void) {};
      bool begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = 0, bool reset = true, bool periphBegin = true) { return true; };
      Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire *twi = &Wire, int8_t rst_pin = -1, uint32_t clkDuring = 400000UL, uint32_t clkAfter = 100000UL) {};
  };

#endif
