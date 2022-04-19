/*
 * ftSwarm.h
 *
 * framework to build a ftSwarm application
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include <stdint.h>
#include <cstddef>

#include <Adafruit_GFX.h>

#include "SwOS.h"

typedef uint16_t FtSwarmSerialNumber_t;
typedef uint8_t  FtSwarmPort_t;

// **** enumerations ****

// IO types
typedef enum { FTSWARM_UNDEF = -1, FTSWARM_INPUT, FTSWARM_ACTOR, FTSWARM_BUTTON, FTSWARM_JOYSTICK, FTSWARM_LED, FTSWARM_SERVO,  FTSWARM_OLED, FTSWARM_GYRO, FTSWARM_HC165, FTSWARM_MAXIOTYPE } FtSwarmIOType_t;

// controler types
typedef enum { FTSWARM_NOCTRL = -1, FTSWARM = 0, FTSWARMCONTROL = 1 } FtSwarmControler_t;

// sensor types
typedef enum { FTSWARM_DIGITAL, FTSWARM_ANALOG, FTSWARM_SWITCH, FTSWARM_REEDSWITCH, FTSWARM_VOLTMETER, FTSWARM_OHMMETER, FTSWARM_THERMOMETER, FTSWARM_LDR, FTSWARM_TRAILSENSOR, FTSWARM_COLORSENSOR, FTSWARM_ULTRASONIC, FTSWARM_MAXSENSOR } FtSwarmSensor_t;

// actor types
typedef enum { FTSWARM_XMOTOR, FTSWARM_TRACTOR,  FTSWARM_ENCODER, FTSWARM_LAMP, FTSWARM_MAXACTOR } FtSwarmActor_t;

// HW versions
typedef enum { FTSWARM_NOVERSION = -1, FTSWARM_1V0, FTSWARM_1V3, FTSWARM_1V15 } FtSwarmVersion_t;

// how to move
typedef enum { FTSWARM_COAST, FTSWARM_BRAKE, FTSWARM_ON } FtSwarmMotion_t;

// toggles
typedef enum { FTSWARM_NOTOGGLE, FTSWARM_TOGGLEUP, FTSWARM_TOGGLEDOWN } FtSwarmToggle_t;

// **** port definitions ****

// switches
#define FTSWARM_S1 0
#define FTSWARM_S2 1
#define FTSWARM_S3 2
#define FTSWARM_S4 3
#define FTSWARM_F1 4
#define FTSWARM_F2 5
#define FTSWARM_J1 6
#define FTSWARM_J2 7

// inputs
#define FTSWARM_A1 0
#define FTSWARM_A2 1
#define FTSWARM_A3 2
#define FTSWARM_A4 3

// outputs
#define FTSWARM_M1 0
#define FTSWARM_M2 1 

// joysticks
#define FTSWARM_JOY1 0
#define FTSWARM_JOY2 1

// leds
#define FTSWARM_LED1 0
#define FTSWARM_LED2 1

// **** some internal types & classes, don't use them at all ****

// handle/pointer to a swarm IO, used by FtSwarmIO base class
typedef void* SwOSIOHandle_t;      

class FtSwarmIO {
  // base class for all ftSwarm interfaces, don't use this class at all
  
  protected:
    SwOSIOHandle_t me = NULL; // pointer to my swarm HW
    
    // constructors
    FtSwarmIO() {};
    FtSwarmIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType);
    FtSwarmIO( const char *name );

  public:
    // check, if I'm online
    bool isOnline();
};

class FtSwarmInput : public FtSwarmIO {
  // an input base class, don't use this class at all
  
  protected:
    FtSwarmInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType, bool normallyOpen = true);
    FtSwarmInput( const char *name, FtSwarmSensor_t sensorType, bool normallyOpen = true );
};

// **** input / actor classes to use in your sketch ****

class FtSwarmDigitalInput : public FtSwarmInput {
  // digital inputs. ports A1..A4, all controler types

  protected:
    FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType, bool normallyOpen = true);
    FtSwarmDigitalInput( const char *name, FtSwarmSensor_t sensorType, bool normallyOpen );
  
  public:
    FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmDigitalInput( const char *name  );

    bool isPressed();                     // getState()
    bool isReleased();                    // !getState()
    bool hasToggledUp();                  // was the last toggle event from down to up, since a hasToggled* method called last time?
    bool hasToggledDown();                // was the last toggle event from up to down, since a hasToggled* method called last time?
    virtual bool getState();              // true if input is high, false otherwise
    virtual FtSwarmToggle_t getToggle();  // last toggle event since a hasToggled* method called last time
};

class FtSwarmSwitch : public FtSwarmDigitalInput {
  // all kind of mechanical switches, A1..A4 all controlers
  // fischertechnik switches: 1-3 is normally open, 1-2 is normally closed
  public:
    FtSwarmSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmSwitch( const char *name, bool normallyOpen = true );
};

class FtSwarmReedSwitch : public FtSwarmDigitalInput {
  // reed switches, A1..A4 all controlers
  public:
    FtSwarmReedSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmReedSwitch( const char *name, bool normallyOpen = true);
};

class FtSwarmButton : public FtSwarmIO {
  // onboard buttons, FtSwarmControl only
  public:
    FtSwarmButton( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmButton( const char *name );
    
    bool isPressed();                     // button is pressed
    bool isReleased();                    // button is released
    bool hasToggledUp();                  // was the last toggle event from down to up, since a hasToggled* method called last time?
    bool hasToggledDown();                // was the last toggle event from up to down, since a hasToggled* method called last time?
    virtual bool getState();              // true, if the button is pressed
    virtual FtSwarmToggle_t getToggle();  // last toggle event since a hasToggled* method called last time
};

class FtSwarmAnalogInput : public FtSwarmInput { 
  // general analog input, A1..A4 ftSwarm only
  protected:
    FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType);
    FtSwarmAnalogInput( const char *name, FtSwarmSensor_t sensorType );

  public:
    FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmAnalogInput( const char *name );

    int32_t getValue();  // get 12 bit raw reading
};

class FtSwarmVoltmeter : public FtSwarmAnalogInput {
  // Voltmeter, ftSwarm.A2 only
  public:
    FtSwarmVoltmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmVoltmeter( const char *name );
    
    float getVoltage();  // get calibrated value 150 mV ~ 2450 mV
};

class FtSwarmOhmmeter : public FtSwarmAnalogInput {
  // Ohmmeter, A11.A4 ftSwarm only
  
  public:
    FtSwarmOhmmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmOhmmeter( const char * name );
    
    float getResistance(); 
};

class FtSwarmThermometer : public FtSwarmAnalogInput {
  // Thermometer based on 1.5kOhm NTC, A1..A4 ftSwarm only
  
  public:
    FtSwarmThermometer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmThermometer( const char *name );
    
    float getCelcius();
    float getKelvin();
    float getFahrenheit();
};

class FtSwarmLDR : public FtSwarmAnalogInput {
  // LDR, A1..A4 ftSwarm only
  // At FtSwarmControl you could use a LDR with a FtSwarmDigitalInput as well
  public:
    FtSwarmLDR( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmLDR( const char * name );
};

class FtSwarmActor : public FtSwarmIO {
  // an actor base class, don't use this class at all
  protected:
    FtSwarmActor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType );
    FtSwarmActor( const char *name, FtSwarmActor_t actorType );
};


class FtSwarmMotor : public FtSwarmActor {
  // general motor class, use this class for (old) gray motors, mini motors, XS motors
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType = FTSWARM_XMOTOR );
    FtSwarmMotor( const char *name, FtSwarmActor_t actorType = FTSWARM_XMOTOR );
    
    void     setSpeed( int16_t speed );    // speed +/-256, speed 0 motor stopss
    uint16_t getSpeed();                   // actual speed
};

class FtSwarmTractorMotor : public FtSwarmMotor {
  // tractor motor
  // M1..M2 all contollers - keep power budget in mind!
  protected:
    FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType );
    FtSwarmTractorMotor( const char * name, FtSwarmActor_t actorType );
      
  public:
    FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmTractorMotor( const char * name );

    // FtSwarmTractor has different options for stopping motor:
    virtual void setMotionType( FtSwarmMotion_t motionType );
    FtSwarmMotion_t getMotionType();
    virtual void coast( void ) { setMotionType( FTSWARM_COAST ); };
    virtual void brake( void ) { setMotionType( FTSWARM_BRAKE ); };
};

class FtSwarmEncoderMotor : public FtSwarmTractorMotor {
  // encoder motor
  // M1..M2 all contollers - keep power budget in mind!
  // TODO: implement encoder input
  public:
    FtSwarmEncoderMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmEncoderMotor( const char * name );
};

class FtSwarmLamp : public FtSwarmActor {
  // classic lamps and LEDs
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmLamp( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmLamp( const char *name );
    
    void on( uint8_t power = 255 );
    void off( void );
};

class FtSwarmJoystick : public FtSwarmIO {
  protected:
    SwOSIOHandle_t button = NULL;
  public:
    FtSwarmJoystick( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmJoystick( const char *name );
    
    int16_t getFB();         // front/back position
    int16_t getLR();         // left/right position
    bool getButtonState();   // button pressed/released
    void getValue( int16_t *FB, int16_t *LR, bool *buttonState );
};

class FtSwarmLED : public FtSwarmIO {
  // RGB LEDs, ftSwarm only
  // one LED takes up to 60mA, keep power budget in mind!
  public:
    FtSwarmLED( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmLED( const char *name );

    // brightness 0..255
    uint8_t getBrightness();
    void setBrightness(uint8_t brightness);

    // color
    uint32_t getColor();
    void setColor(uint32_t color);
};

class FtSwarmServo : public FtSwarmIO {
  // Servo, ftSwarm only
  // A servo has 150mA typically, higher values with load. Keep power budget in mind!
  public:
    FtSwarmServo( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmServo( const char *name );

    // position
    int16_t getPosition();
    void setPosition( int16_t position );

    // offset
    int16_t getOffset();
    void setOffset( int16_t position );
};

class FtSwarmOLED : public FtSwarmIO {
  // OLED display, ftSwarmControl only
  // TODO: add remote calls. Actually the display is limited to local displays.
  public:
    void display(void);
    void clearDisplay(void);
    void invertDisplay(bool i);
    void dim(bool dim);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void startscrollright(uint8_t start, uint8_t stop);
    void startscrollleft(uint8_t start, uint8_t stop);
    void startscrolldiagright(uint8_t start, uint8_t stop);
    void startscrolldiagleft(uint8_t start, uint8_t stop);
    void stopscroll(void);   
    void setRotation(uint8_t r);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillScreen(uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void setTextSize(uint8_t s);
    void setTextSize(uint8_t sx, uint8_t sy);
    void setFont(const GFXfont *f = NULL);   
    void setCursor(int16_t x, int16_t y);
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t bg);
    void setTextWrap(bool w);
    void cp437(bool x = true);
    void write(uint8_t ch);
    void write(const char *str);
    int16_t width(void);
    int16_t height(void);
    uint8_t getRotation(void);
    int16_t getCursorX(void);
    int16_t getCursorY(void);
};

class FtSwarm {
  // my swarm...
  protected:
    bool _IAmAKelda = false;
    bool _verbose = false;
    
  public:
    FtSwarmSerialNumber_t begin( bool IAmAKelda = true );  // start my swarm
    void verbose( bool on );                               // be chatty
    void setReadDelay( uint16_t readDelay );               // set delay between two measures
    void setup( void );                                    // setup
};

// There is one only
extern FtSwarm ftSwarm;
