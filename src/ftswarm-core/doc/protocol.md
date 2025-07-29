# ftSwarm API documentation

## API Calls

### /api/getSwarm

Method: HTTP_GET

Request:

| Request       | Header<BR>Parameter | value               | description |
|:--------------|:--------------------|:--------------------|:---|
| Accept        | Header              | application/json    | send json response
| Accept        | Header              | application/ftSwarm | send proprietary ftSwarm compressed format 
| Authorization | Header              | \<token\>           | optional token

Response:

| Header                      | value               | description  |
|:----------------------------|:--------------------|:-------------|
| Access-Control-Allow-Origin | *                   | CORS allowed |
| Content-type                | application/json    | json, if requested
| Content-type                | application/json    | send proprietary ftSwarm compressed format, if requested


Status Codes:

| Status   | description |
|:---------|:------------|
| 200 ok   | ok          |

Body: {"auth":{[\<auth\>](#auth)},"ctrl":{[[\<ctrl\>](#ctrl)...}}

### /api/getLog

Method: HTTP_GET

Response:

| Header                      | value               | description  |
|:----------------------------|:--------------------|:-------------|
| Access-Control-Allow-Origin | *                   | CORS allowed |
| Content-type                | text/plain          | plain text   |

Status Codes:

| Status   | description |
|:---------|:------------|
| 200 ok   | ok          |

Body: \<log\>

### /api/getToken

Method: HTTP_GET

Response:

| Header                      | value               | description  |
|:----------------------------|:--------------------|:-------------|
| Access-Control-Allow-Origin | *                   | CORS allowed |
| Content-type                | application/json    | json

Status Codes:

| Status   | description |
|:---------|:------------|
| 200 ok   | ok          |

Body: {"token":"\<token\>"}

### /api/led

Method: HTTP_POST

| Request       | Header<BR>Parameter | value       | description |
|:--------------|:--------------------|:------------|:------------|
| Accept        | Header              | application/json | send json response
| Authorization | Header              | \<token\>   | token
| id            | Parameter           | \<string\>  | unique identfier, i.e. "0-A1"
| brightness    | Parameter           | [0.225]     | optional brightness
| color         | Parameter           | #RGB        | optional color

Response:

| Header                      | value               | description  |
|:----------------------------|:--------------------|:-------------|
| Access-Control-Allow-Origin | *                   | CORS allowed |
| Content-type                | application/json    | json


Status Codes:

| Status           | description |
|:-----------------|:------------|
| 200 ok           | ok          |
| 400 bad request  | bad request - any malformed set of headers/parameters
| 401 unauthorized | unauthorized 


### /api/servo

Method: HTTP_POST

| Request       | Header<BR>Parameter | value       | description |
|:--------------|:--------------------|:------------|:------------|
| Accept        | Header              | application/json | send json response
| Authorization | Header              | \<token\>   | token
| id            | Parameter           | \<string\>  | unique identfier, i.e. "0-A1"
| offset        | Parameter           | [-255..255] | optional offset
| position      | Parameter           | [-255..255] | optional position

Response:

| Header                      | value               | description  |
|:----------------------------|:--------------------|:-------------|
| Access-Control-Allow-Origin | *                   | CORS allowed |
| Content-type                | application/json    | json

Status Codes:

| Status           | description |
|:-----------------|:------------|
| 200 ok           | ok          |
| 400 bad request  | bad request - any malformed set of headers/parameters
| 401 unauthorized | unauthorized 

### /api/actor

Method: HTTP_POST

| Request       | Header<BR>Parameter | value       | description |
|:--------------|:--------------------|:------------|:------------|
| Accept        | Header              | application/json | send json response
| Authorization | Header              | \<token\>   | token
| id            | Parameter           | \<string\>  | unique identfier, i.e. "0-A1"
| speed         | Parameter           | \<integer\> | speed

Response:

| Header                      | value               | description  |
|:----------------------------|:--------------------|:-------------|
| Access-Control-Allow-Origin | *                   | CORS allowed |
| Content-type                | application/json    | json

Status Codes:

| Status           | description |
|:-----------------|:------------|
| 200 ok           | ok          |
| 400 bad request  | bad request - any malformed set of headers/parameters
| 401 unauthorized | unauthorized 

### /api/cam

Remark: Only stubs implemented yet

Method: HTTP_POST

| Request       | Header<BR>Parameter | value       | description |
|:--------------|:--------------------|:------------|:------------|
| Accept        | Header              | application/json | send json response
| Authorization | Header              | \<token\>   | token
| id            | Parameter           | \<string\>  | unique identfier, i.e. "0-A1"
| streaming     | Parameter           | [0\|1]      | 
| framesize     | Parameter           | \<integer\> | 
| quality       | Parameter           | \<integer\> | 
| brightness    | Parameter           | \<integer\> | 
| contrast      | Parameter           | \<integer\> | 
| saturation    | Parameter           | \<integer\> | 
| specialEffect | Parameter           | \<integer\> | 
| wbMode        | Parameter           | \<integer\> | 
| H-Mirror      | Parameter           | [0\|1]      | 
| V-Flip        | Parameter           | [0\|1]      | 

Response:

| Header                      | value               | description  |
|:----------------------------|:--------------------|:-------------|
| Access-Control-Allow-Origin | *                   | CORS allowed |

Status Codes:

| Status           | description |
|:-----------------|:------------|
| 200 ok           | ok          |
| 400 bad request  | bad request - any malformed set of headers/parameters
| 401 unauthorized | unauthorized 

###  /api/isAuthorized

Method: HTTP_POST

| Request       | Header<BR>Parameter | value       | description |
|:--------------|:--------------------|:------------|:------------|
| Accept        | Header              | application/json | send json response
| Authorization | Header              | \<token\>   | token

Response:

| Header                      | value               | description  |
|:----------------------------|:--------------------|:-------------|
| Access-Control-Allow-Origin | *                   | CORS allowed |

Status Codes:

| Status           | description |
|:-----------------|:------------|
| 200 ok           | ok          |
| 400 bad request  | bad request - any malformed set of headers/parameters
| 401 unauthorized | unauthorized 

---

## Field Descriptions

### Auth

| json        | ftSwarm | value        | description |
|:------------|:--------|:-------------|:------------|
|"provided":  | \\006   | [0\|1]       | got a token within the request
|"state":     | \\007   | [0\|1]       | 1, if token was accepted
|"kelda":     | \\003   | [0\|1]       | 1, if I'm a kelda

### Ctrl

| json            | ftSwarm | value         | description |
|:----------------|:--------|:--------------|:------------|
| "name":         | \\015   | \<string\>    | controller's hostname
| "id":           | \\016   | \<integer\>   | # of controller for fast identifycation
| "serialNumber": | \\017   | \<integer\>   | controller's serial number
| "type":         | \\018   | [\<cpuversion\>](#cpuversion)| version/type of cpu
| "state":        | \\007   | [0\|1]        | 1, if controller is online
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

### IO

#### Common fields, all types of io:
| json            | ftSwarm | value                 | example | description |
|:----------------|:--------|:----------------------|:--------|:------------|
| "name":         | \\015   | \<string\>            | "A1"    | alias or name
| "id":           | \\016   | \<string\>            | "0-A1"  | unique identifier for fast identification
| "type":         | \\018   | [\<iotype\>](#iotype) | 1       | io-type
| "icon":         | \\019   | \<string\>            | "0.svg" | icon resource name
| "active":       | \\020   | [0\|1]                | 1       | 1, show the io in "only active IOs"  view

#### Special fields based on io type

##### Motor / Output

Motor, x-motor, xm-motor, tractor, encoder, lamp, valve, compressor, stepper, buzzer

| json              | ftSwarm | value        | example | description |
|:------------------|:--------|:-------------|:--------|:------------|
| "speed":          | \\008   | \<integer\>  | 64      | output speed, range dependend on highResolution<br>0 +/-255<BR>1 +/-4095
| "highResolution": | \\009   | [0\|1]       | 1       |

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
