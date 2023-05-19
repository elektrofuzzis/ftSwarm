---
title: FtSwarmOLED
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
      Diese Klasse steuert das OLED Display des ftSwarmControl. Zur Zeit ist dies nur am lokalen Controller möglich, Remotecontroller werden in einem späteren Release unterstützt werden.<br><br>
      Schauen Sie sich auch das OLED-Beispiel im <a href="/de/gettingstarted/FtSwarmOLED">Tutorial</a> an.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: elektrofuzzis" src="/assets/img/LampLedDisplay/ftSwarmControl.png">OLED am ftSwarmControl</div>
</div>



#### FtSwarmOLED(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmOLED Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Serial number of the used ftSwarm controller.

#### FtSwarmOLED( const char *name )

Constructor um ein FtSwarmOLED Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### void clearDisplay(void)

Display löschen.

#### void invertDisplay(bool i)

Anzeige invertieren.

#### void dim(bool dim)

Die Displayhelligkeit einstellen. Da OLED Displays einbrennen können, sollten Sie wenn möglich das Dimmen aktivieren.

#### int16_t getWidth(void)

Gibt die Breite des Displays in Pixeln zurück.

#### int16_t getHeight(void)

Gibt die Höhe des Displays in Pixeln zurück (weisse Fläche).

#### void drawPixel(int16_t x, int16_t y, bool white=true)

Setzt einen Pixel am Punkt (x,y).

#### void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white=true)

Zieht eine Linie von (x0,y0) nach (x1,y1).

#### void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill=false, bool white=true)

Zeichnet ein Rechteck mit der linken oberen Ecke bei (x,y), Breite w und Höhe h.

#### void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill=false, bool white=true)

Zeichnet ein Rechteck mit runden Ecken. Linke oberen Ecke bei (x,y), Breite w, Höhe h und Eckenradius radius.

#### void drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill=false, bool white=true)

Zeichnet einen Kreis mit Mittelpunkt (x0, y0) und Radius r.

#### void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white=true)

Zeichnet ein Dreieck mit den Ecken (x0,y0), (x1,y1) und (x2,y2).

#### void drawChar(int16_t x, int16_t y, unsigned char c, bool color=true, bool bg=false, uint8_t size_x=1, uint8_t size_y=1)

Gibt das Zeichen c an Position (x,y) aus. 

#### void write( char *str )

Gibt den String str an der aktuellen Cursor Position aus.

#### void write( char *str, int16_t x, int16_t y, FtSwarmAlign_t align = FTSWARM_ALIGNLEFT, bool fill = true )

- **FTSWARM_ALIGNLEFT**: Der String str wird links von (x,y) ausgegeben.
- **FTSWARM_ALIGNRIGHT**: Der String str wird rechts von  (x,y) ausegeben.
- **FTSWARM_ALIGNCENTER**: Der String str wird zentriert um (x,y) herum ausgegeben.

Da die Position des Strings angegeben wird, verändert sich die aktuelle CUrsor Position nicht.

#### void setCursor(int16_t x, int16_t y)

Setzt den Cursor auf (x,y).

#### void getCursor(int16_t *x, int16_t *y)

Gibt die Cursorposition zurück.

#### void setTextColor(bool c, bool bg=false)

Setzt die Textfarbe. c=true und bg=false ist weißer Text auf schwarzem Grund.

#### void setTextWrap(bool w)

Wenn TextWrap gesetzt ist, bricht die Ausgabe eines Textes am rechten Bildschirmrand um.

#### void setRotation(uint8_t r)

Alle nachfolgenden Textausgaben werden im den angerebenen Winkel gedreht.

#### uint8_t getRotation(void)

Bestimmt den Winkel der Textrotation.

#### void setTextSize(uint8_t sx, uint8_t sy=1)

Setzt die Schriftgröße.

#### void getTextSize( uint8_t *sx, uint8_t *sy )

Bestimmt die Schriftgröße.
    
#### void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)

Bestimmt die Fläche, die für eine Textausgabe benötigt wird.
