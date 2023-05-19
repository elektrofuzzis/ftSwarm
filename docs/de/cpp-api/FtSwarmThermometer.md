---
title: FtSwarmThermometer
layout: category
lang: de
classes: wide
sidebar:
    nav: cppapi-de
---
<div class="apicontainer">
    <div class="apileft">
        Es gibt verschiedene NTC-Widerstände von fischertechnik. Diese Klasse unterstützt den aktuell verfügbaren NTC mit 1.5kOhm oder jeden anderen NTC mit gleichem Widerstandswert und R/T-Kurve 1023.
    </div>
    <div class="apiright apiimg"><img title="Bildnachweis: fischertechnik" src="/assets/img/analog/sensor-ntc-api.png">NTC Widerstand 1.5 kΩ (36437)</div>
</div>

**ftSwarm**: Am ftSwarm können Widerstände an allen Eingängen angeschlossen werden.<br>
**ftSwarmControl**: Hier können analoge Werte nur am Eingang **A1** ausgelesen werden.
{: .notice--info}

#### FtSwarmThermometer(FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port)

Constructor um ein FtSwarmButton Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- serialNumber: Seriennummer des ftSwarm-Controllers.
- port: Portnummer, **FTSWARM_A1**, **FTSWARM_A2**, ..., **FTSWARM_A6**. ftSwarmControl unterstützt nur **FTSWARM_A1**.

#### FtSwarmThermometer(const char *name)

Constructor um ein FtSwarmButton Objekt zu erzeugen. Ist der angesprochene Controller nicht online, so wartet die Firmware solange bis der entsprechende Controller gestartet wird.

- name: Aliasname des IO-Ports.

####  float Celcius()

Gemessene Temperatur in °Celcius.

####  float Fahrenheit()

Gemessene Temperatur in °Fahrenheit.

####  float Kelvin()

Gemessene Temperatur in °Kelvin.

#### int32_t getValue()

Eigentlicher Messwert des Analogwandlers (raw value).