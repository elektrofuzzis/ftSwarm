---
title: Motors
layout: category
lang: en
classes: wide
sidebar:
    nav: cppapi-en
gallery:
 - url: "/en/cpp-api/FtSwarmXMotor/"
   image_path: "/assets/img/motor/motor-xs.png"
   title: "XS Motor (137096)"
 - url: "/en/cpp-api/FtSwarmXMotor/"
   image_path: "/assets/img/motor/motor-mini.png"
   title: "Mini Motor (32293)"
 - url: "/en/cpp-api/FtSwarmXMMotor/"
   image_path: "/assets/img/motor/motor-xm.png"
   title: "XM Motor (135485)"
 - url: "/en/cpp-api/FtSwarmTractorMotor/"
   image_path: "/assets/img/motor/motor-tractor.png"
   title: "Tractor Motor (151178)"
 - url: "/en/cpp-api/FtSwarmEncoderMotor/"
   image_path: "/assets/img/motor/motor-encoder.png"
   title: "Encoder Motor (153422)"
 - url: "/en/cpp-api/FtSwarmEncoderMotor/"
   image_path: "/assets/img/motor/motor-encoder-competition.png"
   title: "Encoder Motor (186175)"
---
Both controllers have two independend motor drivers (M1 & M2). They could drive all type of *fischertechnik* 9V black DC motors. 
Old grey fischertechnik DC motors or other industrial 9V DC motors could be used, to.

### Speed

It's easy to move a motor. **setSpeed** will start or stop your motor:
- Setting a positive speed, the motor will turn.
- Setting a negative speed, the motor will turn in the other direction.
- Setting 0, the motor will stop.

Use **getSpeed** to read your speed settings.

### Brake & Coast

There are two general fischertechnik motor categories: spindle or axis. 

Using a "spindle" motor and some gear wheels in your robotics, the mechanics is self-locking. 
Cutting your power supply, the model will stop and the mechanics will keep in position. 

In case of an axis based motor, the mechanics isn't self-locking: the weight of your model might turn your motor.
Using an axis based motor, you have two additional motor commands:

- **coast** stops your motor and releases the motor power. The model could move the motor manually.
- **brake** stops your motor but keeps the motor power on. The motor is blocked and you couldn't move the motor manually.

In difference to **setSpeed(0)** both commands they will not change your speed setting. Using **setMotionType( FTSWARM_ON )** the motor will move again.

{% include gallery caption="Image source: fischertechnik & elektrofuzzis" %}