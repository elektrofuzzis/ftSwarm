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

Swarm commands use the keyword swarm, e.g. swarm.get( 1 )

| command                  | description |
|:-------------------------|:------------|
| getSwarm(format)         | return complete swarm. format 0 - raw, 1 - json
| login(pin)               | login
| **addController(SN)**    | add controller with serial number SN to swarm
| **deleteController(SN)** | revoke controller with serial number SN from swarm
| **newSwarm(pin, name)**  | create a new swarm

---

## NVS Commands

Swarm commands use the keyword nvs, e.g. nvs.save( 1 )

| command           | description |
|:------------------|:------------|
| save(scope)       | save settings in nvs. scope 0 - all, 1 - controller settings only, 2 - alias names only, 3 - event configs only
| useConfig(config) | switch to event config 

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
| getIOType()         | return \<io-type\>
| setIOType( ioType ) | set \<io-type\>

### Input Commands

| command                 | description   |
|:------------------------|:--------------|
| subscribe( hysteresis ) | subscribe, to be notified on changes 
| getValue()              | get sensor reading
| onTrigger( triggerEvent, actor, p1) |
| onTrigger( triggerEvent, actor)     |

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
| onTriggerLR( triggerEvent, actor, p1) |
| onTriggerLR( triggerEvent, actor)     |
| onTriggerFB( triggerEvent, actor, p1) |
| onTriggerFB( triggerEvent, actor)     |

### Output Commands

#### Motor Commands

| command                 | description   |
|:------------------------|:--------------|
| setSpeed( speed )
| getSpeed()
| setMotionType( motionType )
| getMotionType()

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
| onTrigger( triggerEvent, actor, p1)