---
title: CLion
parent: Choose your IDE
grand_parent: Writing C++ Applications
nav_order: 3
has_children: yes
has_toc: no
---
Beta
{: .label .label-purple .float-right }
## CLion

1. Download and install the IDE [CLion](https://www.jetbrains.com/clion/)

2. Download and install the latest version of [PlatformIO](https://docs.platformio.org/en/latest//integration/ide/clion.html). 

3. Create a new PlatformIO project in your IDE using the Board `Espressif ESP 32 Dev Module/esp32dev`
- Add the line `lib_deps = bloeckchengrafik/ftSwarm` to your `platformio.ini`
- Change the `platform = espressif32` to `platform = https://github.com/platformio/platform-espressif32.git#eff0222cd1ce270a9c5f6d183e6e240f5e5cd458`

4. Run `pio run` to download and build all libraries.

<br>
[Write your first application: motor&switch](../../YourFirstApplication/MotorSwitch){: .btn .float-right }
<br>