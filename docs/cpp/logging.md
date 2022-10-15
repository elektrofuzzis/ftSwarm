---
title: Logging
parent: Writing C++ Applications
nav_order: 50
---

## Logging

You could use the serial console to show debug & status messages at your PC.

### Arduino Serial

The Arduino environment implements two simple functions to send some output using the serial console. 
`Serial.print` prints the argument, `Serial.println`prints the argument followed by a CR/LF.
If you like to format your output, printf is much more feasible.

### stdout & printf

The used espressif framework sends stdout to the serial console. So you could just use `printf` commands to get some formatted output.
Please keep in mind, the serial console is a buffered device. `printf("Hello world");` doesn't show the string immediatly.
The data is transferred using `fflush( stdout );` or by writing a new line `/n`.

### ESP_LOGx

You could use espressif's logging macro [ESP_LOGx](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html).
Using the Arduino IDE, you could set the logging level during compile time `Tools\Core Debug Level`. 
At PlatformIO you set the debug level in your projects ini-file: `build_flags = -DCORE_DEBUG_LEVEL=5`

### Example

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
