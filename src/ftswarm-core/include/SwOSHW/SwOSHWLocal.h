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

#if FTSWARM_HAL_OLEDS > 0
#include <U8g2lib.h>
#endif

/**************************************************
 *
 *   HC165
 *
 **************************************************/

#if FTSWARM_HAL_HC165 > 0

class HC165 {
  protected:
    // gpio_num_t LD, CS, CLK, MISO;
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

#endif

/***************************************************
 *
 *   SwOSOLED
 *
 ***************************************************/

#define MAXOLEDSCREENS 3

class OLED : protected U8G2_SSD1306_128X64_NONAME_F_HW_I2C {

  protected:

    bool initialized  = false;

    bool displayDirty = false;

    uint8_t drawColor = 1;
    
    uint8_t textSizeX = 0;
    uint8_t textSizeY = 0;

    bool textWrap   = true;
    bool color      = true;
    bool background = false;

    int16_t screenHeight[MAXOLEDSCREENS] = { 16, 48,  0 };
    int16_t screenOffset[MAXOLEDSCREENS] = {  0, 16, 64 };
    int16_t screenScroll[MAXOLEDSCREENS] = {  0,  0,  0 };

    /**
     * Truncates a string to fit within a pixel width (maxWidth) using U8g2's font metrics.
     * Adds "..." if the text is too long.
     * @return A pointer to a new heap-allocated string. Must be freed by the caller!
    */
    char* truncateText(const char* text, int16_t maxWidth);

    int16_t translateY( uint8_t screen, int16_t y );
    bool setFill( FtSwarmOledFill_t fill );
    void resetDrawColor( void ) { setDrawColor( drawColor ); };

  public:

    // constructor
    OLED( ) : U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE) { };

    // begin - initializes HW. Must be called before anything else
    void begin( void );

    // to be called once to run the display task
    void displayTask( void );

    // dim - reduce display brightness
    void dim(bool dim);

    // activat/deactivate button screen - change sizes of MAIN and BUTTON screens
    void buttonScreen( bool activate );

    // clear display
    void cls( void ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::clearBuffer(); displayDirty = true; };

    // clear screen
    void cls( uint8_t screen );

    // get display width
    int16_t getScreenWidth(void);

    // get screen height
    int16_t getScreenHeight( uint8_t screen = FTSWARM_OLED_MAINSCREEN );

    // get text width
    int16_t getTextWidth( const char *text ) { return U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getStrWidth( text ); };

    // set get height
    int16_t getTextHeight( void ) { return U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getMaxCharHeight(); };

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
    void drawStrRect( uint8_t screen, int16_t x, int16_t y, int16_t w, const char *text, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT, uint8_t paddingH = 0, uint8_t paddingV = 0, FtSwarmOledFill_t fill = FTSWARM_OLED_FILLBLACK );

    // draw a triangle
    void drawTriangle( uint8_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );
    
};

extern OLED oled;

/***************************************************
 *
 *   RGBLed
 *
 ***************************************************/

#if FTSWARM_HAL_DISCRETE_RGBS > 0

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

extern RGBLed *rgbLed;

#endif