---
title: FtSwarmPixel
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Der ftSwarm hat zwei eingebaute RGB LEDs, die den Betriebszustand des Controllers anzeigen. Diese LEDs können auch durch das Steuerprogramm verwendet werden.<br>
        Zusätzlich können am LED-Port des ftSwarm bis zu 16 externe ftPixel angeschlossen werden. 
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: elektrofuzzis" src="/assets/img/LampLedDisplay/ftPixel-api.png">ftPixel</div>
</div>

Externe ftPixel werden über den LED Extention Port angeschlossen. Am ftSwarm kann dazu der ftPixelAdpater verwendet werden, am ftSwarmRS wird der erste ftPixel direkt an der Schraubklemme angeschlossen. Die ftPixel benötigen zusätzlich eine 9V-Spannungsversorgung. Bitte beachten Sie bei der Auswahl der Stromversorgung für die ftPixel den Stromverbrauch der LEDs!

![Anschluss](/assets/img/LampLedDisplay/ftPixelChain.png)

Am Gehäuse der ftPixel sind auf der linken Seite "+" und "-" Markierungen. Verbinden Sie den mittleren Pin auf der markierten Seite des ftPixel mit dem LED Extention Port des ftSwarms. "+" und "-" werden an die Stromversorgung angeschlossen.

Weitere ftPixel werden einfach kaskadiert. Dabei werden jeweils die drei Pins der nicht markierten/rechten Seite des vorherigen ftPixel mit den Pins der markierten/linken Seite des nächsten ftPixel verbunden. 

ftPixel können nur am ftSwarm und ftSwarmRS verwendet werden.
{: .notice--info}

#### FtSwarmLED(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmLED Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Port number. Verwenden Sie **FTSWARM_LED1**, **FTSWARM_LED2** für die internen RGB-LEDs, **FTSWARM_LED3**..., **FTSWARM_LED18** für die externen LEDs.

#### FtSwarmLED( const char *name )

Constructor um ein FtSwarmLED Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO Ports.

#### void setColor(uint32_t color)

Setzt die Farbe der LED. Es können die Farbkonstante aus der FastLED-Bibliothek verwendet werden.

#### uint32_t getColor()

Bestimmt die Farbe der LED.

#### void setBrightness(uint8_t brightness)

Setzt die Helligkeit aller LEDs. Dabei ist 255 die maximale Helligkeit, 0 schaltet die LED aus.

#### uint8_t getBrightness()

Bestimmt die Helligkeit der LED.