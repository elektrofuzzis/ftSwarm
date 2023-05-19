---
title: Installation der IDE 
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---

Für die Programmierung der ftSwarm-Controller in c++ gibt es mehere Möglichkeiten. Für Anfänger ist die Arduino-IDE am besten: einfach in der Installation und Bedienung, sowie eine sehr große Internet Community.

1. [Arduino](https://www.arduino.cc/)
   ist die verbreiteste IDE für DIY Projekte. 2005 wurde sie in einem Projekt zur Entwicklung eines Open-Source-Mikrocontroller-Boards entwickelt, heute unterstützt die Plattform alle gängigen Mikrocontroller. 

2. [PlattformIO](https://platformio.org)
   ist ein Plugin für Standard IDEs wie [Visual Studio Code](../vscode) zur Entwicklung von Embedded Systems. Das Einrichten von PlattformIO ist etwas komplizierter, die IDEs sind jedoch deutlich besser und das Compilieren der Software deutlich schneller.

3. [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) wurde von *espressif* selbst für die ESP-Prozessoren entwickelt. Ein sehr mächtiges Framework für den fortgeschrittenen CPP-Entwickler.


### Installation der Arduino IDE

Die Arduino IDE gibt es in zwei Versionen. Unterstützt werden 1.8.19 oder die jeweils neueste 2.x Version. Version 1.8.19 ist auf älteren PCs schneller.

Für die Installation sind folgende Schritte notwendig:

<table>
   <tr><td>Version 1.8.19</td>
       <td>Version 2.x</td>
   </tr>
   <tr><td>Download und Installation der Arduino IDE Version 1.8.19 von <a href="https://www.arduino.cc/en/software">arduino.cc</a>.</td>
       <td>Download und Installation der Arduino IDE Version 2.x von <a href="https://www.arduino.cc/en/software">arduino.cc</a>.</td>
   </tr>
   <tr>
      <td>In der Arduino IDE den Voreinstellungen-Dialog <strong>Datei/Voreinstellungen</strong> öffnen.<br>In <strong>Zusätzliche Boardverwalter-URLs</strong> die folgende URL eintragen: <br> <strong>https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json</strong></td>
      <td>In der Arduino IDE den Einstellungen-Dialog <strong>Datei/Einstellungen öffnen</strong>.<br>In <strong>Zusätzliche Boardverwalter-URLs</strong> die folgende URL eintragen: <br> <strong>https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json</strong></td>
   </tr>
   <tr>
      <td> Zu <strong>Werkzeuge/Board.../Boardverwalter</strong> wechseln and die neueste Version von <strong>esp32 by espressif systems</strong> installieren. Sollten nur 1.x-Versionen angezeigt werden, die Einstellung der Boardverwalter URL aus dem vorherigen Schritt überprüfen.</td>
      <td> Zu <strong>Werkzeuge/Board/Boardverwaltung</strong> wechseln and die neueste Version von <strong>esp32 by espressif systems</strong> installieren. Sollten nur 1.x-Versionen angezeigt werden, die Einstellung der Boardverwalter URL aus dem vorherigen Schritt überprüfen.</td>
   </tr>
   <tr>
      <td colspan="2">Unsere ftSwarm Firmware benötigt einige Drittbibliotheken. Diese werden über <strong>Werkzeuge\Bibliotheken verwalten</strong> installiert:
         <ul>
            <li><strong>Adafruit GFX Library</strong> Version 1.10.12 oder neuer</li>
            <li><strong>Adafruit SSD1306</strong>, Version 2.5.3 oder neuer</li>
            <li><strong>FastLED</strong> by Daniel Garcia, Version 3.4.0 oder neuer</li>
         </ul> </td>
   </tr>
   <tr>
      <td colspan="2">Um die "tSwarm Bibliothek zu installieren die neueste Version von ftswarm.zip von <a href="https://github.com/elektrofuzzis/ftSwarm/releases">github</a> laden und über <strong>Sketch\Bibliothek einbinden\ZIP Bibliothek hinzufügen</strong> installieren.</td>
   </tr>
</table>

Nun muss noch das richtige Board unter <strong>Werkzeuge</strong> eingestellt werden:

<table>
   <tr><td>Option</td><td>ftSwarm</td><td>ftSwarmRS</td><td>ftSwarmControl</td></tr>
   <tr><td>Tools\Board\ESP32 Arduino</td><td>ESP32 Dev Module</td><td>ESP32S3 Dev Module</td><td>ESP32 Dev Module</td></tr>
   <tr><td>Tools\PSRAM</td><td>Enabled</td><td>Enabled</td><td>Enabled</td></tr>
   <tr><td>Tools\Core Debug Level:</td><td>none</td><td>none</td><td>none</td></tr>
   <tr><td>Tools\Arduino Runs On:</td><td>core 0</td><td>core 0</td><td>core 0</td></tr>
   <tr><td>Tools\Events Run On:</td><td>core 1</td><td>core 1</td><td>core 1</td></tr>
   <tr><td>Tools\Port:</td><td>virtuellen COM-Port auswählen</td><td>virtuellen COM-Port auswählen</td><td>virtuellen COM-Port auswählen</td></tr>
</table>

 
### Compile & Upload:

Arduino Programme werden Sketch genannt und als <strong>.ino</strong> Dateien gespeichert. Es kann in einem Verzeichnis immer nur ein Sketch gespeichert werden, da der Verzeichnisname identisch zum Sketchnamen sein muss.

<style>
img { vertical-align: middle;important! }
</style>

- ![build](/assets/img/arduino_compile.png) compiliert den Sketch.
- ![upload](/assets/img/arduino_upload.png) flashed den Sketch auf den ftSwarm.
- ![serial](/assets/img/arduino_serial.png) startet eine Serielle Konsole.

Die Beispiele aus dem Tutorial können über <strong>Datei/Beispiele/ftSwarm</strong> aufgerufen werden.
