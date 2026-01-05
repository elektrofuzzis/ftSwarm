/*
 * SwOSHWDisplay.h
 *
 * Display hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOSHWBaseIO.h"
#include "SwOSOLEDMenu.h"

// only to feed that silly compiler
class OLEDMenu;

/***************************************************
 *
 *   SwOSPixel
 *
 ***************************************************/

 class SwOSPixel : public SwOSIO {
  protected:
    uint32_t color = 0;
    uint8_t  brightness = BRIGHTNESSDEFAULT;
    bool     dynamic = false;
  
    // local HW procedures
    virtual void setupLocal(); 
    virtual void setColorLocal();
    virtual void setBrightnessLocal();
  
    // remote HW procedures
    virtual void setRemote();
    
  public:
    // constructor
    SwOSPixel(const char *name, uint8_t port, SwOSCtrl *ctrl);
  
    // administrative stuff
    virtual void serialize( Serialize *serialize );
    virtual void onTrigger( SwOSTriggerMath_t triggerMath, int32_t sensor, int32_t parameter );
    virtual bool isPixel( void ) { return true; };
    virtual bool isActor( void ) { return true; };
  
    // commands
    virtual uint32_t getColor()      { return color; };
    virtual uint8_t  getBrightness() { return this->brightness; };
    virtual void     setColor(uint32_t color);
    virtual void     setBrightness(uint8_t brightness);
    virtual void     setValue( uint8_t brightness, uint32_t color ) { this->brightness = brightness; this->color = color; };
  };

/***************************************************
 *
 *   SwOSOLED
 *
 ***************************************************/

class SwOSOLED : public SwOSIO {
  protected:

    OLEDMenu *oledMenu = NULL;
    
    // local HW procedures
    virtual void setupLocal(); // initializes local HW
    
  public:
    // constructor
    SwOSOLED(const char *name, SwOSCtrl *ctrl);

    // administrative stuff
    virtual bool isOLED( void ) { return true; };
    virtual void operate() {};

    void invertDisplay(bool i);
    void fillScreen( bool white);    
    void dim(bool dim);
    void setContrast(uint8_t contrast = 0x8F );
    int16_t getWidth(void);
    int16_t getHeight(void);
    
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

    void setTextColor(bool c, bool bg);
    void setTextWrap(bool w);

    void setRotation(uint8_t r);
    uint8_t getRotation(void);

    void setTextSize(uint8_t sx, uint8_t sy);
    void getTextSize( uint8_t *sx, uint8_t *sy );
    
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

    
};