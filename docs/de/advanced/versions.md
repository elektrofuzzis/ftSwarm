---
title: Versionen
layout: category
lang: de
classes: wide
sidebar:
    nav: advanced-de
---

### Version 0.4.2 11/2022

- neu: OLED type support

### Version 0.4.1 11/2022

- Remote Motoren konnten keine negaviven Speed-Werte verarbeiten
- Remote LED's funktionierten nicht korrekt
- "ghost" toggles bei ftSwarmControl-Buttons beim Start werden unterdrückt
- ESP Board Defintion V2.0.5 ist nun stabil. Die weitere Entwicklung der Firmware findet mit Version 2.0.5 statt.
- Arduino IDE 2.0.2 is stable now.

### Version 0.4.0 07/2022

Verwenden Sie bitte die ESP32 Board Definition V2.0.4. Die neue Version 2.0.5 funktioniert noch nicht stabil.

- Servo-Alias-Namen werden im NVS gespeichert.
- Support von 2 internen und 16 externen RGB LEDs.
- Die IP Addresse wird korrekt angezeigt.
- Negative Speed-Parameter ändern die Drehrichtung.
- wifi schaltet nicht mehr in den sleep-Modus
- Di AP Mode Parameter wurden geändert.

### Version 0.3.0 07/2022

- [Fernbedieung](/de/gettingstarted/RemoteControl/) ohne Code zu schreiben.
- [Event-Programming](/de/gettingstarted/EventControlled/)
- Bugfix: Höhere Frequenz beim Auslesen der Sensoren.

### Version 0.2.0 05/2022

- Redesign des Web-UIs.
- Unterstützung des ftSwarmControls OLED Display.
- Es werden bis zu 14 externe ftRGBLeds unterstützt.
- Schwarm & Device Status im ftSwarmControl OLED Display.
- Neue Klasse FtSwarmLightBarrier.
- Neue Klasse FtSwarmXMMotor.
- Neue Klasse FtSwarmValve.
- Neue Klasse FtSwarmCompressor.
- Neue Klasse FtSwarmBuzzer.
- Flexible Paketgröße
- Bugfix: Sende Alias Namem zu Kelda.

### Version 0.11.0 04/2022

Bugfixes.

### Version 0.10.0 03/2022

Erste Version.