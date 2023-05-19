---
title: Konfiguration
layout: category
toc: true
toc_sticky: true
lang: de
classes: wide
sidebar:
    nav: advanced-de
---
Die Controller benötigen einige Einstellungen, wie z.B. ein WLAN-Profil oder auch die Konfiguration des Schwarms. Einige Einstellungen wurden bereits im Tutorial verwendet. Dieses Kapitel beschreibt alle Konfigurationsmöglichkeiten im Detail.

Zur Konfiguration des Controllers kann in der Standardfirmware jederzeit das "Configuration Menu" gestartet werden. Dazu muss nur ein Terminal Programm verbunden werden nur eine Taste gedrückt werden. Das Configuration Menu kann mit

```cpp
ftSwarm.setup();
```

jederzeit aufgerufen werden.

<hr>
### Main Menu

Das **Main Menu** ist die oberste Ebene der Konfigurationsmenüs.  **(0) exit** beendet den Konfigurationsmodus, in der Standardfirmware bootet danach der Controller neu. Wurde das **Main Menu** mit **ftSwarm.setup();** aus einem Programm heraus aufgerufen, so werden nun die nächsten Kommandos im Programm ausgeführt.

<hr>
### Wifi Settings

Um einen Schwarm zu bilden, müssen die einzelnen Controller miteinander über WLAN oder RS485 kommunizieren. In der Regel kommunizieren sie über WLAN - nur die ftSwarmRS können zusätzlich kabelgebunden über RS485 kommunizieren.

Außerdem kann über WLAN die Statusseite der Controller aufgerufen werden.

Mit der Option **(1) wifi** wird der WLAN-Typ festgelegt: 
- CLIENT-MODE nutzt ein vorhandenes WLAN. Im Regelfall ist dies das WLAN Ihres Internetrouters.
- AP-MODE benötigt keinen Router, der ftSwarm-Controller erzeugt ein eigenes WLAN Netzwerk.
- OFF schaltet WLAN aus (nur ftSwarmRS) und reduziert deutlich den Stromverbrauch.

Ist ein WLAN bereits vorhanden, so sollten Sie dieses unbedingt verwenden. Wählen Sie dazu CLIENT-MODE und setzen Sie mit **SSID** und **password** die Zugangsdaten zu Ihrem WLAN. Mit **exit** speichern Sie die Daten im NVS des Controllers und er verbindet sich mit Ihrem WLAN indem er neu startet. 

Der **AP-MODE** ist ein wenig tricky:
- Wählen Sie einen Controller im geplanten Swarm aus und schalten Sie dort auf AP-MODE. Mit **SSID** geben Sie dem WLAN einen Namen. Aufgrund von Einschränkungen von espressif, sind WLANs im AP-MODE immer offene WLANs und es wird kein Passwort benötigt.
- Zusätzlich müssen Sie mit Option **wifi channel** den Kanal auswählen, auf dem Ihre SSID arbeitet.
- Alle anderen Controller werden auf CLIENT-MODE gestellt. Die SSID ist die des obigen Controllers; setzen Sie ein leeres Password.
- Setzen Sie nie die Kelda in den AP-MODE. Wie im Tutorial beschrieben, führt dies zu einigen Problemen beim flushen Ihres Programms.
- Der AP-MODE hat einen hohen Stromverbrauch, das Netzwerk ist für jeden offen (es hat kein Password) und der Sendebereich ist sehr beschränkt.

Die Option **off** kann nur am ftSwarmRS gesetzt werden. In diesem Fall muss Ihr Schwarm komplett über RS485 kommunizieren.

Die WLAN-Kanäle teilen Sie mit anderen WLANs. Deshalb ist es wichtig, den "richtigen" Kanal auszuwählen. Die ESP32-Prozessoren verwenden das 2.4 GHz-Band. Dieser Bereich ist in 11 Kanäle aufgeteilt, dabei überlappen sich benachbarte Kanäle. Werden zwei benachbarte Kanäle durch ein WLAN verwendet, so stören diese sich gegenseitig. Deshalb ist es im 2.4GHz-Band am Besten, wenn alle WLANs nur die Kanäle 1, 6 und 11 verwenden. Optimalerweise gibt es einen nicht verwendeten Kanal. Ist dieser nicht verfügbar, so nutzen Sie einen bereits verwendeten Kanal dessen Nachbarkanäle nicht in Verwendung sind. Welche Kanäle bereits verwendet sind, können Sie sich auf dem meisten Internetroutern anzeigen lassen. Mit der App [wifiman](https://play.google.com/store/apps/details?id=com.ubnt.usurvey&hl=de&gl=US&pli=1) können Sie das auch über Ihr Smartphone analysieren.
{: .notice--info}

<hr>
### Web Server Settings

Dieses Menü hat nur zwei Optionen:
- **WebUI: on/off** schaltet den WEB-Server für die Statuspage ein bzw aus.
- **Show X ftPixels in UI** legt die Anzahl der ftPixel fest, die auf der Statuspage angezeigt werden, bzw. die im Menu "Alias Names" einen Namen zugewiesen bekommen können. Für die Programmierung von ftPixel über die Port-Nummer hat dieser Wert keinen Einfluss, es können immer alle Ports **FTSWARM_LED1** bis **FTSWARM_LED18** angesprochen werden.

<hr>
### Swarm Settings

In diesem Menü wird der Schwarm gebildet. Bevor Sie einen Schwarm bilden können, müssen alle Controller untereinander kommunizieren können. Dazu müssen entweder alle Controller im gleichen WLAN angemeldet sein, oder über RS485 verbunden sein.

Im Menü wird die der Name des Schwarms, die Schwarm-Pin und die Anzahl der angemeldeten Controller im Schwarm angezeigt:

```
This device is connected to swarm "MeinLieberSchwan" with 10 member(s) online. Swarm PIN is 999.
```

**swarm communication**: Stellt ein, über welches Medium die ftSwarm miteinander kommunizieren.
- **wifi**: die Controller nutzen WLAN. 
- **RS485**: die Controller nutzen die RS485-Schnittstelle. Diese Option steht nur am fTSwarmRS zur Verfügung. Ein Mixed-Mode mit WLAN und RS485 ist nicht möglich. 

**create a new swarm**: Erzeugt einen neuen Schwarm. Es werden der Name für den Schwarm und die PIN abgefragt. Anschließend ist dieser Controller der erste und einzige Controller im Schwarm.

**join another swarm**: Fügt den Controller zu einem Schwarm hinzu. Es werden der Name des Schwarms und die PIN abgefragt. Der Controller versucht sich dann mit dem Schwarm zu verbinden, dazu muss mind. ein weiterer Controller des Schwarms online sein.

**list swarm members**: Listet die Mitglieder des Schwarms auf, die online sind.

Jeder Controller hat den Namen des Schwarms und die Schwarm-PIN in seinem NVS gespeichert. Die Namen der anderen Controller im Schwarm werden nicht gespeichert, jeder der den Namen des Schwarms und die PIN kennt kann jederzeit dem Schwarm beitreten. Deshalb ist keine Option für das Verlassen eines Schwarms notwendig.
{: .notice--info}

<hr>
### Alias Names Settings

Mit diesem Menü können Aliasnamen für die Ports eingestellt werden. Anschließend können die IO-Ports über den logischen Namen anstatt der Portnummer angesprochen werden. Im Tutorial ist dies im Kapitel [Alias Names](../gettingstarted/MotorSwitchAlias/) näher erklärt.

Um einen Aliasnamen für einen IO Port zu vergeben, wählen Sie die Nummer des gewüschten IOs aus. Der Name darf bis zu 30 Zeichen lang sein und darf keine Leerzeichen enthalten.

In einem Schwarm muss ein Aliasname eindeutig sein. Dies wird bei der Konfiguration nicht geprüft - es müssen nicht alle Controller im Schwarm online sein. Wird ein Name mehrfach vergeben, so ist es Zufall welcher IO-Port beim Benutzen des Aliasnamens angesprochen wird.
{: .notice--info}

<hr>
### Factory Settings

Dieser Menüpunkt stellt den Controller auf Werkseinstellungen zurück. Da im NVS evtl. vertrauliche Informationen wie die wifi-Parameter gespeichert sind, sollten Sie einen Controller auf Werkseinstellungen zurücksetzten, bevor sie ihn an Dritte weitergeben.

<hr>
### I2C Settings

In diesem Menü wird das Verhalten des I²C-Busses festgelegt. 

**Mode** legt fest, ob der Controller im Slave- oder Mastermodus arbeiten soll. Werden I²C-Sensoren am Bus angeschlossen, so ist MASTER der richtige Modus. 

Nur wenn ein TXT-Controller (s. Seilbahnprojekt) mit dem ftSwarm Daten austauschen soll, ist SLAVE-Modus zu wählen. Da beim **ftSwarm** und **ftSwarmControl** am I²C-Bus auch der optionale Gyro angeschlossen ist, muss der Controller im MASTER-Modus arbeiten wenn der Gyro genutzt werden soll. Der **ftSwarmRS** spricht den internen Gyro über einem zweiten I²C-Bus an, so dass dieser gleichzeitig den Gyro und den SLAVE-Modus verwenden kann.
{: .notice--info}

**Gyro** schaltet die Verwendung des Gyros ein bzw. aus. Schalten Sie den Gyro nur ein, wenn Sie ihn in der Anwendung auch nutzen wollen. Ungenutzte Gyros haben einen unnötigen Stromverbrauch und im Schwarm einen hohen Kommunikationsbearf.

**I2C Address** legt die I²C-Adresse des Controllers im SLAVE-Modus fest. Diese Option steht im MASTER-Modus nicht zur Verfügung.

<hr>
### ftSwarmControl

Der ftSwarmControl hat zwei spezifische Einstellungen.

**Display Type**: Je nach verwendetem OLED-Display unterscheidet sich die Ansteuerung der OLEDs auf Firmwareebene ein wenig. Die ersten ftSwarmControl benötigen den Display Typ 0, die neueren Typ 1. Sollte das Display des ftSwarmControl nicht funktionieren, stellen Sie hier einen anderen Wert ein.  

**Calibrate Joysticks**: Die Joysticks liefern ein analoges Signal. Mit Calibrate Joysticks kann die mittlere Stellung des Joysticks kalibriert werden.

<hr>
### Remote Control Settings

Mit diesem Menü kann ein Schwarm ohne eine Zeile Code programmiert werden. Dazu bekommen die einzelnen IO-Ports Aliasnamen, die dann über den ftSwarmControl angesprochen werden. Dazu werden auf den Buttons und Joysticks des ftSwarmControl EVents definiert, die die IO Ports steuern.

Eine detaillierte Beschreibung ist im Tutorial unter [Fernbedieung](../gettingstarted/RemoteControl/) zu finden.
