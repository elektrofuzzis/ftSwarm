import flask
import flask_cors

app = flask.Flask(__name__)
flask_cors.CORS(app)


@app.route('/api/getSwarm')
def index():
    has_auth = flask.request.headers.get('Authorization')

    return flask.Response("""
{
"auth": {
    "provided": """ + str(has_auth is not None).lower() + """,
    "status": """ + str(has_auth is not None).lower() + """
},
"swarms": [
  {
    "name": "fAlias1",
    "id": 0,
    "serialNumber": 1,
    "hostinfo": "ftSwarm S/N: 1 HAT1v0 CPU1v15 Kelda: 1",
    "type": "ftSwarm",
    "io": [
      {
        "name": "Aliasus2",
        "id": "0-A1",
        "type": "INPUT",
        "icon": "04_voltage.svg",
        "sensorType": 4,
        "subType": "VOLTMETER",
        "value": "4.971 V"
      },
      {
        "name": "Alisas3",
        "id": "0-A2",
        "type": "INPUT",
        "icon": "05_resistor.svg",
        "sensorType": 5,
        "subType": "OHMMETER",
        "value": "2279 Ohm"
      },
      {
        "name": "Alias4",
        "id": "0-A3",
        "type": "INPUT",
        "icon": "06_ntc.svg",
        "sensorType": 6,
        "subType": "THERMOMETER",
        "value": "15.5 °C"
      },
      {
        "name": "Alias5",
        "id": "0-A4",
        "type": "INPUT",
        "icon": "07_ldr.svg",
        "sensorType": 7,
        "subType": "LDR",
        "value": 2321
      },
      {
        "name": "Alias6",
        "id": "0-M1",
        "type": "ACTOR",
        "icon": "17_tractor.svg",
        "subType": "TRACTORMOTOR",
        "motiontype": 0,
        "speed": 0
      },
      {
        "name": "ALias7",
        "id": "0-M2",
        "type": "ACTOR",
        "icon": "16_xmotor.svg",
        "subType": "XMOTOR",
        "motiontype": 0,
        "speed": 0
      },
      {
        "name": "Alias8",
        "id": "0-LED1",
        "type": "LED",
        "icon": "15_rgbled.svg",
        "brightness": 48,
        "color": "008000"
      },
      {
        "name": "Alias9",
        "id": "0-LED2",
        "type": "LED",
        "icon": "15_rgbled.svg",
        "brightness": 48,
        "color": "008000"
      },
      {
        "name": "SERVO",
        "id": "0-SERVO",
        "type": "SERVO",
        "icon": "14_servo.svg",
        "offset": 0,
        "position": 0
      }
    ]
  },
  {
      "name": "flAlias1",
      "id": 1,
      "serialNumber": 1,
      "hostinfo": "ftSwarm S/N: 1 HAT1v0 CPU1v15 Kelda: 1",
      "type": "ftSwarmControl",
      "io": [
        {
          "name": "Alias2",
          "id": "0-A1",
          "type": "INPUT",
          "icon": "04_voltage.svg",
          "sensorType": 4,
          "subType": "VOLTMETER",
          "value": "4.971 V"
        },
        {
          "name": "Alisas3",
          "id": "0-A2",
          "type": "INPUT",
          "icon": "05_resistor.svg",
          "sensorType": 5,
          "subType": "OHMMETER",
          "value": "42069 Ohm"
        },
        {
          "name": "Alias4",
          "id": "0-A3",
          "type": "INPUT",
          "icon": "06_ntc.svg",
          "sensorType": 6,
          "subType": "THERMOMETER",
          "value": "15.5 °C"
        },
        {
          "name": "Alias5",
          "id": "0-A4",
          "type": "INPUT",
          "icon": "07_ldr.svg",
          "sensorType": 7,
          "subType": "LDR",
          "value": 2321
        },
        {
          "name": "Alias6",
          "id": "0-M1",
          "type": "ACTOR",
          "icon": "17_tractor.svg",
          "subType": "TRACTORMOTOR",
          "motiontype": 0,
          "speed": 255
        },
        {
          "name": "ALias7",
          "id": "0-M2",
          "type": "ACTOR",
          "icon": "16_xmotor.svg",
          "subType": "XMOTOR",
          "motiontype": 0,
          "speed": 0
        },
        {
          "name": "Alias8",
          "id": "0-LED1",
          "type": "LED",
          "icon": "15_rgbled.svg",
          "brightness": 48,
          "color": "008000"
        },
        {
          "name": "Alias9",
          "id": "0-LED2",
          "type": "LED",
          "icon": "15_rgbled.svg",
          "brightness": 48,
          "color": "008000"
        },
        {
          "name": "SERVO",
          "id": "0-SERVO",
          "type": "SERVO",
          "icon": "14_servo.svg",
          "offset": 0,
          "position": 0
        },
        {
          "name": "B1",
          "id": "0-BUTTON1",
          "type": "BUTTON",
          "icon": "12_button.svg",
          "state": 0
        },
        {
          "name": "B2",
          "id": "0-BUTTON2",
          "type": "BUTTON",
          "icon": "12_button.svg",
          "state": 1
        },
        {
          "name": "J1",
          "id": "0-JOYSTICK1",
          "type": "JOYSTICK",
          "icon": "11_joystick.svg",
          "valueLr": 50,
          "valueFb": 50,
          "button": 1
        },
        {
          "name": "J1",
          "id": "0-JOYSTICK1",
          "type": "JOYSTICK",
          "icon": "11_joystick.svg",
          "valueLr": 30,
          "valueFb": 70,
          "button": 1
        }
      ]
    }
]
}
    """, mimetype='application/json', headers={'Access-Control-Allow-Origin': '*'})


@app.route('/api/getToken')
def get_token():
    return flask.Response(response="""{"Token": "0000"}""", mimetype='application/json', headers={'Access-Control-Allow-Origin': '*'})

@app.route('/api/actor', methods=['POST'])
def actor():
    return flask.Response(response="""{"Token": "0000"}""", mimetype='application/json', headers={'Access-Control-Allow-Origin': '*'})

@app.route('/api/led', methods=['POST'])
def led():
    return flask.Response(response="""{"Token": "0000"}""", mimetype='application/json', headers={'Access-Control-Allow-Origin': '*'})

@app.route('/api/servo', methods=['POST'])
def servo():
    return flask.Response(response="""{"Token": "0000"}""", mimetype='application/json', headers={'Access-Control-Allow-Origin': '*'})


if __name__ == '__main__':
    app.run()
