---
title: FtSwarmLamp
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Mit dieser Klasse können fischertechnik Lampen und LEDs an den Ausgängen M1 und M2 betrieben werden. Verwenden Sie allerdings die Klasse <a href="FtSwarmPixel">FtSwarmPixel</a> wenn Sie RGB-LEDs bzw. ftPixel anzuschließen wollen.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/LampLedDisplay/kombi.png">verschiedene LEDs und Lampen</div>
</div>

Wird eine LED an einen Motorausgang mit beiden Kabeln angeschlossen, so muss die Polung getestet werden. Leuchtet die LED nicht werden, so vertauschen Sie die Anschlüsse an der LED. 

Bei den *fischertechnik*-TX und TXT-Controllern können die Ausgänge auf acht unabhängige Ports (O1..O8) erweitert werden. Dies ist bei den ftSwarm-Controllern nur bedingt möglich. Es können an M1 oder M2 zwei LEDs auf GND geschaltet werden. Diese können abwechselnd aber nicht gleichzeitig eingeschaltet werden. Verwenden Sie n diesem Fall eine Instanz der Klasse FtSwarmXMotor für beide LEDs. **setSpeed(255)** schaltet eine der beiden LEDs ein, **setSpeed(-255)** die andere LED. **setSpeed(0)** schaltet beide LEDs aus.
{: .notice--info}

#### FtSwarmLamp(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmLamp Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: **FTSWARM_M1** oder **FTSWARM_M2**

#### FtSwarmLamp( const char *name )

Constructor um ein FtSwarmLamp Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

#### void on( uint8_t power = 255 )

Schaltet die Lampe bzw. die LED ein. Die Helligkeit kann über den Parameter power optional eingestellt werden.

#### void off( void )

Schaltet die Lampe ode LED aus. Identisch zu **on(0)**.