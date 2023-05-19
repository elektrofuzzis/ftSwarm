---
title: USB-Anschluß
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---

Um ftSwarm-Applikationen auf den Controller zu laden, muss er über USB an einen PC angeschlossen werden.

Für die ersten Schritte wird keine extene Stromversorgung benötigt. Da später die Ein- und Ausgänge ohne 9V-Versorgung nicht funktionieren werden, bietet es sich an gleich die 9V-Versorgung mit anzuschließen.

1. Verbinden Sie Ihr 9V Netzteil mit dem 9V-Pwr-Eingang des Controllers. Sobald der Controller gestartet ist, werden am ftSwarm beide LEDs grün leuchten. Beim fTSwarmControl zeigt das Display einen Startbildschirm an. Sollten die LEDs nicht an sein oder das Display nicht funktionieren so muss die Stormversorgung überprüft werden.

2. Verbinden Sie den fTSwarm mit einem USB Kabel mit Ihrem PC. Windows installiert in der Regel die notwendigen Treiber selbständig. Installieren Sie den Treiber ggf. manuell. Im Abschnitt **Treiber** finden Sie links zu den Treibern unter Windows und Linux.

3. Ist der Treiber installiert, so meldet sich der ftSwarm als COM-Schnittstelle im Windows Device Manager. Die COM-Schnittstelle Ihres ftSwarm können Sie identifizieren, indem Sie das USB-Kabel mehrfach abziehen und wieder anschließen.

4. Überprüfen Sie die USB-Verbindung mit einem Terminal-Programm wie [PuTTY SSH Client](https://www.putty.org/). Starten Sie den ftSwarm-Controller über den Reset-Taster, damit sich die Firmware im Putty-Terminal meldet. Schließen Sie zum Schluß das Terminalprogramm, da es die Schnittstelle blockiert.

<img alt="putty" src="/assets/img/putty.png" width="350">


### Windows Treiber

ftSwarm: [wch-ic.com CH340C](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)

ftSwarmControl: [silicon labs CP210x USB to UART Bridge VCP Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)


### Linux Treiber

Um den Gerätenamen für den seriellen Anschluss Ihres ftSwarm zu ermitteln, führen Sie den folgenden Befehl zweimal aus, zuerst mit ausgestecktem, dann mit eingestecktem ftSwarm.

Die Schnittstelle, die beim zweiten Mal zusätzlich erscheint, ist die des ftSwarm:
```
ls /dev/tty*
```

Ihr im Linux angemeldeter User benötigt Schreib- und Leserechte auf dem seriellen Port.
In den meisten Linux Distributionen muss dazu dem obigen Port die Gruppe **dialout** zugewiesen werden:

```
sudo usermod -a -G dialout $USER
```

Bei Arch Linux muss **uucp** verwendet werden:

```
sudo usermod -a -G uucp $USER
```


### macOS

Um den Gerätenamen für den seriellen Anschluss Ihres ftSwarm zu ermitteln, führen Sie den folgenden Befehl zweimal aus, zuerst mit ausgestecktem, dann mit eingestecktem ftSwarm.

Die Schnittstelle, die beim zweiten Mal zusätzlich erscheint, ist die des ftSwarm:

```
ls /dev/cu.*
```