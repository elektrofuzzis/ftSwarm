---
title: FtSwarmOLED
parent: Lamp, Led & Display
grand_parent: Writing C++ Applications
nav_order: 3
---
FtSwarmOLED
{: .label .label-blue .float-right}
## OLED Display

Your ftSwarmControl has an OLED display. It's splitted in two areas: the upper yellow area is used by the firmware to show the status of your swarm.
Feel free to use the lower white area with 128x48 pixels in your swarm application.

```cpp
#include <ftSwarm.h>

void setup() {

  Serial.begin(115200);
  FtSwarmSerialNumber local = ftSwarm.begin();
  
  FtSwarmOLED oled = FtSwarmOLED( local );
  
  // upper left corner
  oled.write("Hello world!");
  
  // center
  oled.write("Hello world!", oled.width()/2, oled.height()/2, FTSWARM_ALIGNCENTER);
  
  // lower right corner
  oled.write("Hello world!", oled.width(), oled.height() - 8 , FTSWARM_ALIGNRIGHT);
  
}

void loop() {}
```

The example is very easy. It writes "Hello world!" at the upper left corner, at the center and at the lower right corner of your screen.
Since the yellow area is used by the firmware, pixel (0/0) is the upper left pixel in the white area.

Comming Soon
{: .label .label-yellow .float-right}
*The OLED display commands are actually limited to the local controller. You can't send OLED display commands to a remote controller yet.*

### API Reference

#### FtSwarmOLED(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor to create a FtSwarmOLED object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- serialNumber: Serial number of the used ftSwarm controller.
- port: Port number, `FTSWARM_LED1` or `FTSWARM_LED2`

#### FtSwarmOLED( const char *name )

Constructor to create a FtSwarmOLED object. If the referenced controller isn't connected to the swarm yet, the firmware will wait until the controller gets online.

- name: Alias name of the IO port.

#### void display(void)

To fasten up the application speed, the driver uses an internal display buffer.
Call `display` to send the internal display buffer to your OLED display.

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
- `FTSWARM_ALIGNLEFT`: The string is printed from (x,y) to the right.
- `FTSWARM_ALIGNRIGHT`: The string is printed from (x,y) to the left.
- `FTSWARM_ALIGNCENTER`: The string is printed centered horiziontally around (x,y)

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
