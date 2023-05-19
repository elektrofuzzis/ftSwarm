---
title: WLAN und Statusseite
Lang:  de
layout: category
classes: wide
sidebar:
    nav: gettingstarted-de
---

Damit die einzelnen ftSwarm-Controller als Schwarm agieren können, müssen sie untereinander kommunizieren. Dies findet in der Regel über WLAN statt, der ftSwarmRS kann auch über RS485 kommunizieren. Über das WLAN kann zusätzlich die Statusseite des Controllers aufgerufen werden. Hier wird der Pegel der Eingänge angezeigt; Motoren, Servos und LEDs können manuell gesteuert werden.

Für die WLAN-Konfiguration muss der ftSwarm-Controller über die USB-Schnittstelle eingestellt werden. Starten Sie wie zuvor beschrieben ein Terminalprogramm und booten Sie den Controller über den Resettaster.

Der Controller meldet sich nun mit seiner Bootmeldung. Drücken Sie eine beliebige Taste, so dass Sie in das Konfigurationsmenü der ftSwarm-Firmware kommen:

```
ftSwarmOS 0.5.0

(C) Christian Bergschneider & Stefan Fuss

Main Menu

(1) wifi settings
(2) webserver settings
(3) swarm settings
(4) alias names
(5) factory settings

(0) exit
main>
```

Wählen Sie **(1) wifi Settings**. Im wifi-Menü stellen Sie nun auf dem Controller auf *CLIENT-MODE* und geben Sie die Zugangsdaten zu Ihrem WLAN ein:

```
wifi settings

(1) wifi:     CLIENT-MODE
(2) SSID:     <WLAN-Name/SSID>
(3) Password: <WLAN-Passwort>

(0) exit
wifi>
```

Beenden Sie mit **(0)** das Menü und speichern Sie die Einstellungen. Der Controller startet neu und verbindet sich mit Ihrem WLAN.

Sie können jetzt auf die Statusseite des Controllers über **`http:\\ftSwarm<SerialNumber>`** im Browser zugreifen. Dabei muss <SerialNumber> mit der Seriennummer des verwendeten ftSwarm-Controllers ersetzt werden.

![Monitoring ftSwarm](../../assets/img/ftSwarm_Monitor.png)

*Damit Sie vom PC aus auf die Statusseite zugreifen können, muss*
- *im Browser java script aktiv sein,*
- *im WLAN die Kommunikation zwischen zwei Geräten erlaubt sein. Dies ist in der Regel nur im Gäste-WLAN ausgeschaltet.*

Um die Aktoren wie Motoren zu bedienen, müssen Sie sich mit dem LOGIN-Button am Controller anmelden. Dabei wird der Swarm-Pin abgefragt, werksseitig ist dies die 4-stellige Seriennummer des Controllers (im Beispiel 0100).