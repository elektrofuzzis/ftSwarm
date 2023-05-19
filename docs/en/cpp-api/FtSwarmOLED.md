---
title: FtSwarmOLED
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
---
<div class="apicontainer">
    <div class="apileft">
      This class controls the OLED display of the ftSwarmControl. Currently this is only possible on the local controller, remote controllers will be supported in a later release.<br><br>
      You should also take a look at the OLED example in the <a href="/en/gettingstarted/FtSwarmOLED">tutorial</a>.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: elektrofuzzis" src="/assets/img/LampLedDisplay/ftSwarmControl.png">ftSwarmControl's OLED display</div>
</div>



#### FtSwarmOLED(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmOLED object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.

#### FtSwarmOLED( const char *name )

Constructor to create a FtSwarmOLED object. If the referenced controller isn't connected to the swarm yet, the firmware will waits until the controller gets online.

- name: Alias name of the IO port.

#### void clearDisplay(void)

Clear display.

#### void invertDisplay(bool i)

Invert display.

#### void dim(bool dim)

Dim the display's brightness.

#### int16_t getWidth(void)

Return the display's with.

#### int16_t getHeight(void)

Return the display's height: It's the white text area's height.

#### void drawPixel(int16_t x, int16_t y, bool white=true)

Draw a pixel at position x,y.

#### void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white=true)

Draw a line from (x0,y0) to (x1,y1).

#### void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill=false, bool white=true)

Draw a rectangular with upper left edge (x,y) using width w and height h.

#### void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill=false, bool white=true)

Draw a rectangular with "round" edges. Start with upper left edge (x,y) using width w and height h.

#### void drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill=false, bool white=true)

Draw a circle with center (x0, y0) and radius r.

#### void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white=true)

Draw a Traingle using the edges (x0,y0), (x1,y1) and (x2,y2).

#### void drawChar(int16_t x, int16_t y, unsigned char c, bool color=true, bool bg=false, uint8_t size_x=1, uint8_t size_y=1)

Draw a single character c at (x,y). 

#### void write( char *str )

Write some text on the display at the cursor position.   

#### void write( char *str, int16_t x, int16_t y, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT, bool fill = true )

Write an aligned string. 
- **FTSWARM_ALIGNLEFT**: The string is printed from (x,y) to the right.
- **FTSWARM_ALIGNRIGHT**: The string is printed from (x,y) to the left.
- **FTSWARM_ALIGNCENTER**: The string is printed centered horiziontally around (x,y)

Since you specify (x,y), thes command has no effect on the cursor's position.

#### void setCursor(int16_t x, int16_t y)

Sets cursor to (x,y).

#### void getCursor(int16_t *x, int16_t *y)

Gets cursor position (x,y).

#### void setTextColor(bool c, bool bg=false)

Set text color. c=true,bg=false is white text on a black background.

#### void setTextWrap(bool w)

If set, text text wraps to the next line at the right edge of the display.

#### void setRotation(uint8_t r)

Set an angle to rotate text output.

#### uint8_t getRotation(void)

Get text rotation.

#### void setTextSize(uint8_t sx, uint8_t sy=1)

Set text size.

#### void getTextSize( uint8_t *sx, uint8_t *sy )

Get text size.
    
#### void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)

Calculate the needed size to write a text.
