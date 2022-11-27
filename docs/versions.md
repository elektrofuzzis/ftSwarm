---
title: Version History
nav_order: 91
---
## Version History

### Version 0.4.1 11/2022

- remote motors couldn't handle negative speed values
- remote led's didn't work properly
- supress "ghost" toggles on ftSwarmControl buttons during startup
- ESP Board defintion V2.0.5 is stable now. Future development will be based on 2.0.5.
- Arduino IDE 2.0.2 is stable now.

### Version 0.4.0 07/2022

Please use ESP32 Board definition V2.0.4. New 2.0.5 isn't stable with ftSwarm.

- Servo's alias name is now saved in NVS.
- Support of 2 internal and 16 external RGB LEDs.
- Showing the IP address correctly.
- Negative speed switches rotation movement.
- wifi won't go to sleep mode
- Changed AP mode parameters

### Version 0.3.0 07/2022

- [Remote control:](../setup/configure_your_device/remote_control) Use your swarm without any line of code.
- [Event Controlled Programming.](../cpp/YourFirstApplication/EventControlled)
- Bugfix: Speedup sensor readings.

### Version 0.2.0 05/2022

- Redesign of ftSwarm Web-UI.
- Support of ftSwarmControls OLED display.
- Up to 14 ftRGBLeds could be connected externally.
- Detailed swarm & device status at ftSwarmControl's OLED display.
- New class FtSwarmLightBarrier.
- New class FtSwarmXMMotor.
- New class FtSwarmValve.
- New class FtSwarmCompressor.
- New class FtSwarmBuzzer.
- Extended swarm communication using flexible packet sizes.
- Bugfix: Sending alias names to Kelda.

### Version 0.11.0 04/2022

Some bugfixes.

### Version 0.10.0 03/2022

First official version.