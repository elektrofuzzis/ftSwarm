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

// #define FTSWARM_HAL_OLEDS 1

#if FTSWARM_HAL_OLEDS > 0
#include <U8g2lib.h>
#endif

/***************************************************
 *
 * wifiHandler
 *
 ***************************************************/

class WifiHandler {

  protected:

  public:

    bool scanActive = false;
    uint16_t aps = 0;
    wifi_ap_record_t *ap = nullptr;
    uint8_t connectedDevices = 0;

    // constructor
    WifiHandler();

    // destructor
    ~WifiHandler();

    // eventHandler - don't call
    void eventHandler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data );
    
    // start a network scan
    void startScan( void );

    // stop a network scan
    void stopScan( void );

    // deduplicate ap list
    void uniqueScanResult( void );

};

extern WifiHandler *wifiHandler;

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

#if FTSWARM_HAL_OLEDS > 0

/**
 * @class OLED
 * @brief Custom OLED display class derived from U8G2 to support virtual screens, 
 * scrolling, and advanced text rendering like truncation and alignment.
 */
class OLED : protected U8G2_SSD1306_128X64_NONAME_F_HW_I2C {

  protected:
    bool initialized  = false; ///< True if begin() was called successfully
    bool displayDirty = false; ///< True if the buffer has changed and needs a refresh
    FtSwarmOledScreen_t clipScreen = FTSWARM_OLED_MAXSCREEN;    ///< screen

    uint8_t drawColor = 1;     ///< Current active drawing color (0=black, 1=white, 2=XOR)
    
    uint8_t textSizeX = 0;     ///< Horizontal text scaling factor
    uint8_t textSizeY = 0;     ///< Vertical text scaling factor

    bool textWrap     = true;  ///< Enable/disable text wrapping (not implemented in all methods)
    bool color        = true;  ///< Foreground color state
    bool background   = false; ///< Background color state

    /** @brief Array containing the physical height of each virtual screen */
    int16_t screenHeight[3] = { 16, 48,  0 };

    /** @brief Array containing the vertical start position (Y) of each virtual screen */
    int16_t screenOffset[3] = {  0, 16, 64 };
    
    /** @brief Current scroll offset for each screen */
    int16_t screenScroll[3] = {  0,  0,  0 };

    /**
     * @brief Truncates a string to fit within a pixel width using U8g2's font metrics.
     * @param text The input string to be measured and potentially cut.
     * @param maxWidth Maximum allowed width in pixels.
     * @return A pointer to a new heap-allocated string with "..." appended if truncated. 
     * @note Must be freed by the caller using free()!
     */
    char* truncateText(const char* text, int16_t maxWidth);

    /**
     * @brief Translates a local screen coordinate to a global OLED Y-coordinate.
     * @param screen Index of the virtual screen.
     * @param y Local Y-coordinate within that screen.
     * @return Calculated global Y-coordinate on the 128x64 display.
     */
    int16_t translateY( FtSwarmOledScreen_t screen, int16_t y );

    /**
     * @brief Prepares drawing color and background based on fill type.
     * @param fill Fill mode (None, Black, White).
     * @return Returns true if setup was successful.
     */
    bool setFill( FtSwarmOledFill_t fill );

    /** @brief Resets the U8G2 draw color to the class's internal drawColor state. */
    void resetDrawColor( void ) { setDrawColor( drawColor ); };

    /** @brief set clipping area for my screen */
    void setClipping( FtSwarmOledScreen_t screen );


  public:

    /** @brief Constructor: Initializes the U8G2 driver with no reset pin. */
    OLED( ) : U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE) { };

    /** @brief Initializes the hardware (I2C) and the display. Call this in setup(). */
    void begin( void );

    /** @brief Periodic task to handle display refreshes. Usually called in the main loop. */
    void displayTask( void );

    /** @brief Sets the display contrast/dimming. @param dim True to reduce brightness. */
    void dim(bool dim);

    /** @brief Resizes MAIN and BUTTON screens to toggle between UI modes. @param activate True for button mode. */
    void buttonScreen( bool activate );

    /** @brief Clears the entire physical display buffer. */
    void cls( void ) { U8G2_SSD1306_128X64_NONAME_F_HW_I2C::clearBuffer(); displayDirty = true; };

    /** @brief Clears only the area of a specific virtual screen. @param screen Screen index. */
    void cls( FtSwarmOledScreen_t screen );

    /** @return Total physical width of the OLED display in pixels. */
    int16_t getScreenWidth(void);

    /** @brief Gets the physical height of the OLED display in pixels. */
    int16_t getDisplayHeight(void) { return U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getDisplayHeight(); };

    /** @brief Access raw display buffer data for screenshot generation. */
    uint8_t *getDisplayBuffer(void) { return U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getBufferPtr(); };

    /** @brief Gets the height of a virtual screen. @param screen Screen index. @return Height in pixels. */
    int16_t getScreenHeight( FtSwarmOledScreen_t screen = FTSWARM_OLED_MAINSCREEN );

    /** @brief Measures the pixel width of a string. @param text String to measure. @return Width in pixels. */
    int16_t getTextWidth( const char *text ) { return U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getStrWidth( text ); };

    /** @return The maximum height of a character in the currently active font. */
    int16_t getTextHeight( void ) { return U8G2_SSD1306_128X64_NONAME_F_HW_I2C::getMaxCharHeight(); };

    /** @brief Draws a button-like element with text and optional padding. */
    void drawButton( FtSwarmOledScreen_t screen, int16_t x, int16_t y, uint8_t width, const char *text, uint8_t flags = 0, uint8_t paddingH = 0, uint8_t paddingV = 0 );

    /** @brief Sets the drawing color (0=black, 1=white, 2=XOR). @param color Color value. */
    void setDrawColor( uint8_t color );

    /** @brief Draws a circle. @param fill Set to NOFILL, FILLBLACK, or FILLWHITE. */
    void drawCircle( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t r, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );

    /** @brief Draws an ellipse. @param rx horizontal radius, @param ry vertical radius. */
    void drawEllipse( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t rx, int16_t ry, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );
    
    /** @brief Draws a line between two points. */
    void drawLine( FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1 );
    
    /** @brief Draws a single pixel at local coordinates. */
    void drawPixel( FtSwarmOledScreen_t screen, int16_t x, int16_t y );

    /** @brief Draws a rectangle. @param fill Fill mode (Outline or Solid). */
    void drawRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, int16_t h, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );

    /** @brief Draws a rounded rectangle. @param r Corner radius in pixels. */
    void drawRoundRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );

    /** @brief Draws aligned text at a specific position. @param align Left, Center, or Right. */
    void drawStr( FtSwarmOledScreen_t screen, int16_t x, int16_t y, const char *text, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT );

    /**
     * @brief Draws text inside a rectangle with automatic truncation ("...") if too long.
     * @param w Available width for the text area.
     * @param fill Background fill for the text area.
     */
    void drawStrRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, const char *text, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT, uint8_t paddingH = 0, uint8_t paddingV = 0, FtSwarmOledFill_t fill = FTSWARM_OLED_FILLBLACK );

    /** @brief Draws a triangle between three points. */
    void drawTriangle( FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL );

    /**
     * @brief Draws a vertical scroll indicator on the right edge of the screen.
     * @param maxHeight The total height of the virtual content to scroll through.
     */
    void drawVSlider( FtSwarmOledScreen_t screen, int16_t maxHeight );

    /**
      * @brief Adjusts the screen scroll offset so that the given Y coordinate is visible.
      * @param screen The index of the virtual screen.
      * @param y The local Y-coordinate in the virtual space that must be visible.
      * @return True if the scroll offset was changed, false otherwise.
      */
    bool scroll( FtSwarmOledScreen_t screen, int16_t yLow, int16_t yHigh );
  
};

#else

class OLED {

  public:

    /** @brief Constructor: Initializes the U8G2 driver with no reset pin. */
    OLED( ) {};

    /** @brief Initializes the hardware (I2C) and the display. Call this in setup(). */
    void begin( void ) {};

    /** @brief Periodic task to handle display refreshes. Usually called in the main loop. */
    void displayTask( void ) {};

    /** @brief Sets the display contrast/dimming. @param dim True to reduce brightness. */
    void dim(bool dim) {};

    /** @brief Resizes MAIN and BUTTON screens to toggle between UI modes. @param activate True for button mode. */
    void buttonScreen( bool activate ) {};

    /** @brief Clears the entire physical display buffer. */
    void cls( void ) { };

    /** @brief Clears only the area of a specific virtual screen. @param screen Screen index. */
    void cls( FtSwarmOledScreen_t screen ) {};

    /** @return Total physical width of the OLED display in pixels. */
    int16_t getScreenWidth(void) { return 0; };

    /** @brief Gets the height of a virtual screen. @param screen Screen index. @return Height in pixels. */
    int16_t getScreenHeight( FtSwarmOledScreen_t screen = FTSWARM_OLED_MAINSCREEN ) { return 0; };

    /** @brief Measures the pixel width of a string. @param text String to measure. @return Width in pixels. */
    int16_t getTextWidth( const char *text ) { return 0; };

    /** @return The maximum height of a character in the currently active font. */
    int16_t getTextHeight( void ) { return 0; };

    /** @brief Draws a button-like element with text and optional padding. */
    void drawButton( FtSwarmOledScreen_t screen, int16_t x, int16_t y, uint8_t width, const char *text, uint8_t flags = 0, uint8_t paddingH = 0, uint8_t paddingV = 0 ) {};

    /** @brief Sets the drawing color (0=black, 1=white, 2=XOR). @param color Color value. */
    void setDrawColor( uint8_t color ) {};

    /** @brief Draws a circle. @param fill Set to NOFILL, FILLBLACK, or FILLWHITE. */
    void drawCircle( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t r, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL ) {};

    /** @brief Draws an ellipse. @param rx horizontal radius, @param ry vertical radius. */
    void drawEllipse( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t rx, int16_t ry, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL ) {};
    
    /** @brief Draws a line between two points. */
    void drawLine( FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1 ) {};
    
    /** @brief Draws a single pixel at local coordinates. */
    void drawPixel( FtSwarmOledScreen_t screen, int16_t x, int16_t y ) {};

    /** @brief Draws a rectangle. @param fill Fill mode (Outline or Solid). */
    void drawRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, int16_t h, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL ) {};

    /** @brief Draws a rounded rectangle. @param r Corner radius in pixels. */
    void drawRoundRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL ) {};

    /** @brief Draws aligned text at a specific position. @param align Left, Center, or Right. */
    void drawStr( FtSwarmOledScreen_t screen, int16_t x, int16_t y, const char *text, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT ) {};

    /**
     * @brief Draws text inside a rectangle with automatic truncation ("...") if too long.
     * @param w Available width for the text area.
     * @param fill Background fill for the text area.
     */
    void drawStrRect( FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t w, const char *text, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT, uint8_t paddingH = 0, uint8_t paddingV = 0, FtSwarmOledFill_t fill = FTSWARM_OLED_FILLBLACK ) {};

    /** @brief Draws a triangle between three points. */
    void drawTriangle( FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, FtSwarmOledFill_t fill = FTSWARM_OLED_NOFILL ) {};

    /**
     * @brief Draws a vertical scroll indicator on the right edge of the screen.
     * @param maxHeight The total height of the virtual content to scroll through.
     */
    void drawVSlider( FtSwarmOledScreen_t screen, int16_t maxHeight ) {};

    /**
      * @brief Adjusts the screen scroll offset so that the given Y coordinate is visible.
      * @param screen The index of the virtual screen.
      * @param y The local Y-coordinate in the virtual space that must be visible.
      * @return True if the scroll offset was changed, false otherwise.
      */
    bool scroll( FtSwarmOledScreen_t screen, int16_t yLow, int16_t yHigh ) { return false; };
  
};

#endif

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
    CRGB color =CRGB::Black;
    void set( void );
    void setPWM( uint8_t c, uint32_t duty );

  public:

    RGBLed();
    void setColor( CRGB color );
    void setBrightness( uint8_t brightness );

};

extern RGBLed *rgbLed;

#endif