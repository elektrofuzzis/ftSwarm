---
title: Debugging
layout: category
lang: de
classes: wide
sidebar:
    nav: advanced-de
---
Es gibt mehrere Möglichkeiten Kelda-Applikation zu debuggen. Die einfachste Variante ist, sich die Werte auf der [status page](/de/gettingstarted/WebUI) des Controllers anzuzeigen. Sie zeigt den Status aller Ein- und Ausgänge an; die Ausgänge können über die Statuspage auch verändert werden. 

Zusätzlich können Debugausgaben auf der seriellen Konsole / Terminal Programm ausgegeben werden. Dazu muss zwar das Programm angepasst werden, die Debugausgaben sind aber viel aussagekräftiger.


### Arduino Serial

In der Arduinoumgebung gibt es zwei einfache Funktionen um Daten auf der seriellen Konsole auszugeben. **Serial.print** gibt das Argument an, **Serial.println** zusätzlich mit CR/LF und wechselt so in die nächste Ausgabezeile. Verwenden Sie printf, wenn die Ausgabe sehr gut formattiert werden muss. 


### stdout & printf

Printf gibt das Ergebenis ebenfalls auf der seriellen Konsole aus. Damit können Daten mit **printf** wie gewohnt formattiert ausgegeben werden. Allerdings ist die serielle Konsole ein "buffered device", so dass **printf("Hello world");** nicht sofort ausgegeben wird. Um die direkte Ausgabe zu erzwingen muss entweder **fflush( stdout );** verwendet werden oder mit **/n** in eine neue Zeile gewechselt werden.


### ESP_LOGx

ESP stellt ein Logging-Macro [ESP_LOGx](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html) zur Verfügung. Zur COmpile-Zeit lässt sich dann für ein Programm festlegen, welche Loggingtiefe ausgegeben werden soll. In der Arduino IDE kann das Logginglevel über **Tools\Core Debug Level** eingestellt werden. In PlatformIO wird das Logginglevelin der Konfiguratonsdatei des Projektes angegeben: **build_flags = -DCORE_DEBUG_LEVEL=5**


### Beispiel

```cpp
#include <ftSwarm.h>
#include <esp_log.h>

void setup(  ) {

	Serial.begin( 115200 );
	ftSwarm.begin();
	
	int p1 = 17;
	int p2 = 42;
	
	// Serial
	Serial.print("p1=");   Serial.print(p1);
	Serial.print(", p2="); Serial.println(p2);
	
	// printf
	printf( "p1=%d p2=%s\n", p1, p2 );

	// debug message
	ESP_LOGD( "MYTAG", "p1=%d p2=%s", p1, p2 );
	
	// error message
	ESP_LOGE( "MYTAG", "unexpected behaviour" );

}

void loop() {}
```
