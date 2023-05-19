---
title: Motors
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
gallery:
 - url: "/de/cpp-api/FtSwarmXMotor"
   image_path: "/assets/img/motor/motor-xs.png"
   title: "XS Motor (137096)"
 - url: "/de/cpp-api/FtSwarmXMotor"
   image_path: "/assets/img/motor/motor-mini.png"
   title: "Mini Motor (32293)"
 - url: "/de/cpp-api/FtSwarmXMMotor"
   image_path: "/assets/img/motor/motor-xm.png"
   title: "XM Motor (135485)"
 - url: "/de/cpp-api/FtSwarmTractorMotor"
   image_path: "/assets/img/motor/motor-tractor.png"
   title: "Tractor Motor (151178)"
 - url: "/de/cpp-api/FtSwarmEncoderMotor"
   image_path: "/assets/img/motor/motor-encoder.png"
   title: "Encoder Motor (153422)"
 - url: "/de/cpp-api/FtSwarmEncoderMotor"
   image_path: "/assets/img/motor/motor-encoder-competition.png"
   title: "Encoder Motor (186175)"
---
Beide Controller verfügen über zwei unabhängige Motor Endstufen (M1 & M2). Mit ihnen können alle *fischertechnik* 9V DC Motroren angesteuert werden. Die graue Serie von Fischertechnik und andere 9V DC Motroren können ebenfalls verwendet werden.

### Speed

Um einen Motor zu starten, wird nur die Funktion **setSpeed** benötigt:
- Ein positiver speed-Wert startet den Motor.
- Ein negativer speed-Wert ändert die Drehrichtung.
- 0 stoppt den Motor.

Mit **getSpeed** kann die aktuelle Geschwindigkeit ausgelesen werden.

### Brake & Coast

Es gibt zwei Motorbauweisen bei *fischertechnik*: Motoren mit einer Antriebsachse und Motoren mit einer Spindel.

Treiben Spindelmotoren Zahnräder an, so ist der Aufbau selbstblockierend. Schaltet man die Stromversorgung ab, so stoppt das Modell und die Mechanik bleibt an der letzten Position stehen.

Werden Motoren mit Antriebsachsen verwendet, so ist der Antrieb nicht selbstblockierend: Im ausgeschalteten Zustand kann das Gewicht des Modells den Motor bewegen. Deshalb gibt es für Motoren mit ANtriebsachsen zwei zusätzliche Methoden:
- **coast** stoppt den Motor und schaltet die Endstufe komplett aus. Es ist möglich den Motor manuell zu drehen.
- **brake** stoppt den Motor, schaltet die Endstufe allerdings nicht aus. Der Motor wird blockiert und kann nicht manuell gedreht werden.

Im Gegensatz zu **setSpeed(0)** verändern die beiden Methoden nicht die gesetzte Geschwindigkeit. Ein **setMotionType( FTSWARM_ON )** startet den Motor in der zuvor gesetzten Geschwindigkeit.

{% include gallery caption="Bildnachweis: fischertechnik & elektrofuzzis" %}