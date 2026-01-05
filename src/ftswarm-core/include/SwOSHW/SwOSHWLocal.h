/*
 * SwOSHWLocal.h
 *
 * Local Hardware
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include "SwOS.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
 

/**************************************************
 *
 *   HC165
 *
 **************************************************/

class HC165 {
  protected:
    gpio_num_t LD, CS, CLK, MISO;
    uint8_t    lastValue;
  
  public:
  
    // constructor
    HC165( FtSwarmVersion_t CPU  );
  
    // administrative stuff
    virtual void operate();
  
    // commands
    virtual void    setValue( uint8_t value ) { this->lastValue = value; };
    virtual uint8_t getValue( uint8_t bit )   { return lastValue && 1<<bit; };
    virtual uint8_t getValue()                { return lastValue; };
  
  };

extern HC165 *hc165;

/***************************************************
 *
 *   SwOSOLED
 *
 ***************************************************/

#define YELLOWPIXELS 16

class OLED {

  protected:

    Adafruit_SSD1306 display = Adafruit_SSD1306 (128, 64, &Wire, -1);
    bool displayDirty = false;
    
    uint8_t textSizeX = 0;
    uint8_t textSizeY = 0;

    bool color      = true;
    bool background = false;
    
  public:

    OLED( void );

    void flush( void );

    void invertDisplay(bool i);
    void fillScreen( bool white);    
    void dim(bool dim);
    void setContrast(uint8_t contrast = 0x8F );
    int16_t getWidth(void);
    int16_t getHeight(void);
    void clearDisplay(void);
    void cp437( bool x );
    
    void drawPixel(int16_t x, int16_t y, bool white);  
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white);
    void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white);
    void drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y);

    // write some text
    void write( const char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill, bool invert );
    void write( const char *str );
   
    void setCursor(int16_t x, int16_t y);
    void getCursor(int16_t *x, int16_t *y);

    void setTextColor( bool c,  bool bg);
    void setTextWrap(bool w);

    void setRotation(uint8_t r);
    uint8_t getRotation(void);

    void setTextSize(uint8_t sx, uint8_t sy);
    void getTextSize( uint8_t *sx, uint8_t *sy );
    
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    
};

extern OLED *oled;