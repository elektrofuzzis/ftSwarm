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
    SwOSPixel(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags );
  
    // administrative stuff
    virtual void serialize( Serialize *serialize );
    virtual void onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t parameter );
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
    SwOSOLED(const char *name, SwOSCtrl *ctrl, uint8_t flags );

    // administrative stuff
    virtual bool isOLED( void ) { return true; };
    virtual void operate() {};

    // dim - reduce display brightness
    void dim(bool dim);

    // clear display
    void cls( void );

    // clear screen
    void cls( uint8_t screen );

    // get display width
    int16_t getScreenWidth(void);

    // get screen height
    int16_t getScreenHeight( uint8_t screen = FTSWARM_OLED_MAINSCREEN );

    // get text width
    int16_t getTextWidth( const char *text );

    // set get height
    int16_t getTextHeight( void );

    // draw button
    void drawButton( uint8_t screen, int16_t x, int16_t y, uint8_t width, const char *text, uint8_t flags = 0, uint8_t paddingH = 0, uint8_t paddingV = 0 );

    // set draw color ( 0=black, 1=white, 2=XOR )
    void setDrawColor( uint8_t color );

    // draw a circle
    void drawCircle( uint8_t screen, int16_t x, int16_t y, int16_t r, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );

    // draw an ellipse
    void drawEllipse( uint8_t screen, int16_t x, int16_t y, int16_t rx, int16_t ry, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );
    
    // draw line
    void drawLine( uint8_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1 );
    
    // draw pixel
    void drawPixel( uint8_t screen, int16_t x, int16_t y );

    // draw a rectangular
    void drawRect( uint8_t screen, int16_t x, int16_t y, int16_t w, int16_t h, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );

    // draw a round rectangular
    void drawRoundRect( uint8_t screen, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );

    // draw a string
    void drawStr( uint8_t screen, int16_t x, int16_t y, const char *text, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT );

    // draw a string in an rectangular
    void drawStrRect( uint8_t screen, int16_t x, int16_t y, int16_t w, const char *text, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT, uint8_t paddingH = 0, uint8_t paddingV = 0 );

    // draw a triangle
    void drawTriangle( uint8_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );
    
};