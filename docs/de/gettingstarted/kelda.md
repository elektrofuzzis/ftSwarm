---
title: Das Kelda Prinzip
layout: category
lang: de
classes: wide
sidebar:
    nav: gettingstarted-de
---
Klassische Roboter werden von einer zentralen Steuereinheit gesteuert. Das ftSwarm-Programm aus dem letzten Abschnitt nutzt nur einen Controller und kann die IOs des Controllers mit den **setup**- und **loop**-Funktionen steuern. 

Schaltet man mehrere Controller zu einem Schwarm zusammen, entsteht ein verteiltes System. Technisch gesehen müssen nun Programme auf allen Controllern geschreiben werden, die über WLAN- oder RS485 miteinander  kommunizieren. Dies ist recht kompliziert, die ftSwarm Firmware übernimmt aber diese Funktion.

Die "Kleinen freien Männer" in Terry Pratchetts gleichnamigen Roman sind ein chaotischer Haufen, da jeder für sich selbst handelt - wie die ftSwarm Controller. Über die Männer herrscht jedoch die Kelda. Sie ist das einzige weibliche Wesen des Clans und sorgt für die Grundlagen der Zusammenarbeit: Was sie sagt, ist Gesetz und muss befolgt werden. Es wird also nur eine Kelda benötigt, die den Schwarm kontrolliert.

Da alle Controller einen leistungsstarken ESP32-Prozessor haben, wird ein beliebiger Controller im Schwarm zur Kelda. Er/sie übernimmt die Steuerung und sendet Steuerbefehle an die anderen Controller über WiFi oder RS485. Alle anderen Controller melden ihre Sensorzustände zurück, so dass die Kelda den Status aller Subsysteme kennt.

Damit ist die Programmierung wieder einfach. Es wird nur ein einfaches Programm auf der Kelda benötigt. Auf allen anderen Controllern wird die Standard-Firmware eingesetzt. Die ftSwarm-Library sorgt dafür, dass das "Kelda"-Programm einfach auf alle verteilten Aktoren und Sensoren zugreifen kann.