---
title: Choose your IDE 
parent: Writing C++ Applications
nav_order: 1
has_children: yes
has_toc: no
---

## Choosing your platform and IDE 

There are different platforms and IDEs to write your C++ code.

1. [Arduino](https://www.arduino.cc/)
   is the most common platform for DIY projects. It started 2005 as a project to design an open source microcontroller board, 
   today the platform supports all common types of microcontrollers. The Arduino environment is an IDE with integrated framework for embedded systems.

2. [PlattformIO](https://platformio.org)
   is a framework to develop embedded systems, too. This framework is not part of an IDE like Arduino and you could choose among different editors/IDEs.
   In the first time, PlatformIO is a bit more complicated than Arduino IDE, but it has many advantages. IDEs like [Visual Studio Code](https://code.visualstudio.com/) 
   or [CLion](https://www.jetbrains.com/clion/) are much more powerful, compile times are lot faster.

3. [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) is the core framework for ESP-microcontrollers, 
   powered by *espressif*. This framework is under the hood on Arduino and PlatformIO. Today, our firmware uses some 3rd party libraries from the 
   Arduino/PlatformIO community. So it's easier to work with Arduino or Platform. If you like, you could spend some time on ESP-IDF as well.

In the following we describe the setup of the three common environemnts:

1. **Arduino:** If you are undecided, use the [Arduino IDE](../arduino).

2. **PlatformIO & Visual Studio Code:** Using PlatformIO most developers use [Visual Studio Code](../vscode) because it's free.

3. **PlatformIO & CLion:** This is is good option, if you are already using [CLion](../clion)

<br>
[CLion](../clion){: .btn .float-right .mx-4 }
[Visual Studio Code](../vscode){: .btn .float-right .mx-4 }
[Arduino](../arduino){: .btn .float-right .mx-4 }
<br>
