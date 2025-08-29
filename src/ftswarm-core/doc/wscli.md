# ftSwarm CLI & WS documentation

## Top Level Commands

| command   | WS/CLI | description |
|:----------|:-------|:------------|
| help      | CLI    | list all commands
| setup     | CLI    | start setup mode
| halt      | both   | stop all motors
| whoami    | both   | list my own hostname
| uptime    | both   | list controllers uptime
| exit      | CLI    | exit command line interface.

---

## Swarm Commands

Swarm commands use the keyword swarm, e.g. swarm.getSwarm( 1 )

| command                  | description |
|:-------------------------|:------------|
| getSwarm(format)         | return complete swarm. format 0 - raw, 1 - json [response](#getswarm-response)
| **getEvents(format)**        | return a list of all events. . format 0 - raw, 1 - json [response](#getevent-response)
| login(pin)               | login
| useConfig(config)        | switch to event config 
| save(scope)              | save settings in nvs. scope 0 - all, 1 - controller settings only, 2 - alias names only, 3 - event configs only
| **addController(SN)**    | add controller with serial number SN to swarm
| **deleteController(SN)** | revoke controller with serial number SN from swarm
| **newSwarm(pin, name)**  | create a new swarm

---

## Controller Commands

Controller commands follow the syntax \<Hostname\>.\<Controller-Command\>\(\<parameter\>,...).

| command                       | description |
|:------------------------------|:------------|
| show                          | identify controller by setting it's LED to blue
| triggerUserEvent(P1,P2,..P10) | Trigger a user remote code.
| setMicroStepMode(mode)        | set Microstep Mode (ftSwarmPwrDrive only)
| getMicroStepMode()            | get Microstep Mode (ftSwarmPwrDrive only)
| setWifi( mode, SSID, PSK )    | set wifi parameters, to activate follow up witch save(1); reboot();
| reboot                        | reboot controller
| save(scope)                   | save settings in nvs. scope 0 - all, 1 - controller settings only, 2 - alias names only, 3 - event configs only
| setAlias( alias )             | set alias name, e.g. ftSwam123.setAlias("MainCtrl")

---

## IO Commands

IO commands start with the name or alias of an IO:
- \<Alias-Name\>
- \<IO-Name\> which reflects to the kelda's IOs
- \<Hostname\>.\<IO-Name\>

followed by the command itself. The commands depend on the io's io-type.

| command             | description   |
|:--------------------|:--------------|
| setAlias( alias )   | set alias name, e.g. M1.setAlias("engine")
| getIOType()         | return [\<io-type\>](#iotype)
| setIOType( [\<io-type\>](#iotype) ) | set [\<io-type\>](#iotype)

### Input Commands

| command                 | description   |
|:------------------------|:--------------|
| subscribe( hysteresis ) | subscribe, to be notified on changes 
| getValue()              | get sensor reading
| onTrigger( [\<triggerEvent\>](#trigger), actor, p1) |
| onTrigger( [\<triggerEvent\>](#trigger), actor)     |

#### Digital Inputs

| command                 | description   |
|:------------------------|:--------------|
| getToggle()             | 0 - no toggle, 1 - toggle up, 2 - toggle down

#### Voltmeter

| command                 | description   |
|:------------------------|:--------------|
| getVoltage()            | get voltage  [V]

#### Ohmmeter

| command                 | description   |
|:------------------------|:--------------|
| getResistance()         | get resistance [Ohm]

#### Thermometer

| command                 | description   |
|:------------------------|:--------------|
| getKelvin()             | get reading in degree Kelvin
| getCelcius()            | get reading in degree Celcius
| getFahrenheit()         | get reading in degree Fahrenheit

### Joystick commands (JOY1..JOY2):

A joystick is a superset of 3 IOs. LR, FR and Button.
These IOs could be addressed on joystick levl (see table below) or based on the singular io names:
- JOY[1\|2]LR
- JOY[1\|2]FB
- JOY[1\|2]Button

| command                 | description   |
|:------------------------|:--------------|
| subscribe               | subscribe all 3 sub-ios
| getValue()              | returns LR & FB reading in a line
| onTriggerLR( [\<io-type\>](#iotype), actor, p1) |
| onTriggerLR( [\<io-type\>](#iotype), actor)     |
| onTriggerFB( [\<io-type\>](#iotype), actor, p1) |
| onTriggerFB( [\<io-type\>](#iotype), actor)     |

### Output Commands

#### Motor Commands

| command                 | description   |
|:------------------------|:--------------|
| setSpeed( speed )            | sets the motor's speed
| getSpeed()                   | gets the motor's speed
| setMotionType( motionType )  | set motion type - 0 FTSWARM_COAST, 1 FTSWARM_BRAKE, 2 FTSWARM_ON
| getMotionType()              | get motion type
| setPosition( position )      | sets actual position to  
| getPosition()                | get position
| setDistance( distance, rel ) | sets the distance to go, relative to actual position (rel=1) or absolute
| getDistance()                | gets actual position
| setHomingOffset( offset )    |
| run()                        | start moving
| stop()                       | stop moving
| isHoming                     | 1 - if homing procedure is ongoing
| isRunning                    | 1 - if the motor is moving
| homing( maxSteps )           | invoke homing procedure. sign of maxSteps defines direction. maxSteps are the max. steps to run

#### Servo Commands

| command                 | description   |
|:------------------------|:--------------|
| setPosition( position )
| getPosition()
| setOffset( position )
| getOffset()

### Display Commands

#### ftPixel Commands

| command                     | description   |
|:----------------------------|:--------------|
| setColor( color )           | set Pixels color, color is a RGB value as decimal, c-hex (0xFF0000) or html (#FF0000) syntax
| getColor()                  | get pixels color, returns hex RGB-Value with leading #,m e.g. #FF0000
| setBrightness( brightness ) | set LED's brightness [0..255]
| getBrightness()             | get LED's brightness

### I2C Commands

| command                 | description   |
|:------------------------|:--------------|
| setRegister( register, value )
| getRegister( register )
| onTrigger( [\<io-type\>](#iotype), actor, p1)

## getEvents response

getEvents could be triggered by swarm.getEvents(format).

The response is:
{"activeConfig": \<integer\>, "events":[[\<Event\>](#event), ... ]}

### Event

| json            | ftSwarm | value         | example | description |
|:----------------|:--------|:--------------|:--------|:------------|
| "sensor":       | \\004   | \<string\>    | "on"    |sensor's alias name
| "actor":        | \\005   | \<string\>    | "led"   | actors's alias name
| "trigger":      | \\006   | [\<trigger\>](#trigger)| 1 | triggering event
| "value":        | \\012   | \<integer\>   | #FF0000 | constant value to use in case of tiggerUp or TriggerDown

### Trigger

| value | trigger |
|:------|:---------|
| 0     | TriggerDown: A digtal input's value changes from 1 to 0. The actor will be set by the event's constant value.
| 1     | TiggerUp: - a digtal input's value changes from 0 to 1. The actor will be set by the event's constant value.
| 2     | TriggerValue: - a sensor's value has changed. The actor will be set by the sensor's value.
| 3     | TriggerI2CREAD
| 4     | TriggerI2CWRITE

## getSwarm response

getSwarm could be triggered by swarm.getSwarm and is send periodically to all connected WebUIs to inform about the swarm's state. In case of the periodically push, kelda uses the ftSwarm RAW format, with swarm.getSwarm you can choose between RAW and JSON format.

The response sting is:

{"name": \<string\>, "kelda:"\<[0\|1]\>,"controllers":[[\<Ctrl\>](#ctrl), ... ]}

| json            | ftSwarm | value         | description |
|:----------------|:--------|:--------------|:------------|
| "name":         | \\015   | \<string\>    | swarm name
| "kelda":        | \\003   | [0\|1]        | 1, if the connected controller is Kelda


### Ctrl

| json            | ftSwarm | value         | description |
|:----------------|:--------|:--------------|:------------|
| "name":         | \\015   | \<string\>    | controller's hostname
| "serialNumber": | \\017   | \<integer\>   | controller's serial number
| "type":         | \\018   | [\<cpuversion\>](#cpuversion)| version/type of cpu
| "state":        | \\007   | \<integer\>   | if controller [state](#state)
| "io":           | \\002   | [\<ioarray\>](#iotype)   | array of controller's ios


### Cpuversion

| value | cpu hardware |
|:------|:---------|
| 0     | ftSwarm V1.0 - development only
| 1     | ftSwarmControl V1.3
| 2     | ftSwarm V1.15 - production version with JST connectors
| 3     | ftSwarmRS V2.0 - development only
| 4     | ftSwarmRS V2.0 - production version
| 5     | ftSwarmCAM
| 6     | ftSwarmDuino
| 7     | ftSwarmPwdDrive
| 8     | ftSwarmXL
| 9     | ftSwarmRC
| 10    | ftSwarmControl V2.x - reserved

### IOType

| value | IO type |
|:------|:---------|
| 0     | [digital input](#digital-inputs)
| 1     | [switch](#digital-inputs)
| 2     | [reedswitch](#digital-inputs)
| 3     | [light barrier](#digital-inputs)
| 4     | [button](#digital-inputs)
| 5     | [analog input](#analog-inputs)
| 6     | [voltmeter](#analog-inputs)
| 7     | [ohmmeter](#analog-inputs)
| 8     | [thermometer](#analog-inputs)
| 9     | [ldr](#analog-inputs)
| 10    | [joystick](#analog-inputs)
| 11    | [motor](#motor--output)
| 12    | [x-motor](#motor--output)
| 13    | [xm-motor](#motor--output)
| 14    | [tractor motor](#motor--output)
| 15    | [encoder motor](#motor--output)
| 16    | [lamp](#motor--output)
| 17    | [valve](#motor--output)
| 18    | [compressor](#motor--output)
| 19    | [buzzer](#motor--output)
| 20    | [stepper](#motor--output)
| 21    | [counter](#digital-inputs)
| 22    | [rotary encoder](#digital-inputs)
| 23    | [frequency meter](#digital-inputs)
| 24    | [lidar](#digital-inputs)
| 25    | [cam](#cam)
| 26    | [servo](#servo)
| 27    | [ftPixel](#pixel)
| 28    | OLED - not used in UI
| 29    | I2C - not used in UI
| 30    | [gyro](#gyro)
| 31    | HC165 - not used in UI
| 32    | [power sensor](#analog-inputs)
| 33    | color sensor - not implemented yet
| 34    | trail sensor - not implemented yet
| 35    | ultrasonic sensor - not implemented yet
| 36    | joystick poti - not used in UI

### state

| value | state |
|:------|:---------|
| 0     | Offline
| 1     | booting - internal state, for documentation only
| 2     | starting wifi - internal state, for documentation only
| 3     | running
| 4     | error
| 5     | waiting for hardware / other controllers
| 6     | identify
| 7     | fatal error - swarm has stopped operation

### IO

#### Common fields, all types of io:
| json            | ftSwarm | value                 | example | description |
|:----------------|:--------|:----------------------|:--------|:------------|
| "name":         | \\015   | \<string\>            | "A1"    | alias or name
| "type":         | \\018   | [\<iotype\>](#iotype) | 1       | io-type
| "icon":         | \\019   | \<string\>            | "0.svg" | icon resource name
| "active":       | \\020   | [0\|1]                | 1       | 1, show the io in "only active IOs"  view

#### Special fields based on io type

##### Motor / Output

Motor, x-motor, xm-motor, tractor, encoder, lamp, valve, compressor, buzzer

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "speed":          | \\008   | \<integer\>  | 64      | output speed, range dependend on highResolution<br>0 +/-255<BR>1 +/-4095
| "highResolution": | \\009   | [0\|1]       | 1       |

##### Stepper

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "speed":          | \\008   | \<integer\>  | 64      | output speed, range dependend on highResolution<br>0 +/-255<BR>1 +/-4095
| "highResolution": | \\009   | [0\|1]       | 1       | 0 - maxspeed [-255, 255], 1 maxspeed [-4095,4095]
| "homing":         | \\035   | [0\|1]       | 0       | 1 - homing procedure ongoing
| "running":        | \\036   | [0\|1]       | 0       | 1 - stepper in progress
| "position":       | \\011   | \<long\>     | 10000   | actual position in steps
| "distance":       | \\034   | \<long\>     | 666     | distance to go in steps


##### Servo

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "offset":         | \\010   | [-255..255]  | 64      | offset to set mid position
| "position":       | \\011   | [-255..255]  | 32      | position

##### Digital Inputs

Digital Input, Switch, Reed Switch, Light Barrier, Ultrasonic, Counter, Frequency Meter, Rotary Encoder, Lidar

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "value":          | \\012   | \<integer\>  | 2123    | sensor value

Button

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "value":          | \\012   | [0\|1]       | 1       | sensor value

##### Analog Inputs

Thermometer

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "value":          | \\012   | \<string\>   | 23.3 °C | sensor value

Ohmmeter

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "value":          | \\012   | \<string\>   | 333 Ohm | sensor value

Voltmeter

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "value":          | \\012   | \<string\>   | 2.123 V | sensor value

LDR, Power, Analog:

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "value":          | \\012   | \<integer\>  | 2123    | sensor value

##### Joystick

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "valueLR":        | \\013   | [-100..100]  | 0       | left/right sensor value
| "valueFB":        | \\014   | [-100..100]  | 0       | front/back sensor value
| "value":          | \\012   | [0\|1]       | 1       | button value


##### Cam

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "url":            | \\025   | \<string\>   | /stream | 
| "framesize":      | \\026   | \<integer\>  | 5       | 
| "quality":        | \\027   | \<integer\>  | 5       | 
| "brightness":     | \\021   | \<integer\>  | 5       | 
| "contrast":       | \\028   | \<integer\>  | 5       | 
| "saturation":     | \\029   | \<integer\>  | 5       | 
| "h-Mirror":       | \\030   | [0\|1]       | 5       | 
| "v-Flip":         | \\031   | [0\|1]       | 5       | 

##### Pixel

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "brightness":     | \\021   | [0..255]     | 48      | Brightness
| "color":          | \\022   | \<string\>   | #FF0000 | RGB hex value 

##### Gyro

MPU6050

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "quarternion":    | \\023   | (\<float\>,\<float\>,\<float\>,\<float\>) | (0.00,0.00,0.00,0.00)  | Quarterion (w,x,y,z)
| "acceleration":   | \\024   | (\<float\>,\<float\>,\<float\>) | (0.00,0.00,0.00) | Acceleration (ax,ay,az)
