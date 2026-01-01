/*
 * SwOSHWBaseCtrl.cpp
 *
 * Basic classes for controller hardware impelmentation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWDigital.h"
#include "SwOSHW/SwOSHWAnalog.h"
#include "SwOSHW/SwOSHWActor.h"
#include "SwOSHW/SwOSHWDisplay.h"
#include "SwOSHW/SwOSHWCounter.h"
#include "SwOSHW/SWOSHWCam.h"
#include "SwOSHW/SwOSHWHAL.h"
#include "SwOSCom.h"
#include "SwOSLog.h"

// local pixels & oled
SwOSPixel *pixel0 = NULL;
SwOSPixel *pixel1 = NULL;
SwOSOLED  *oled   = NULL;

/***************************************************
 *
 *   SwOSCtrl - Base class for controllers.
 *
 ***************************************************/

uint8_t SwOSCtrl::setupLocalInputs( uint8_t maxIO ) {

  char name[10];

  for ( uint8_t i=0; i<MAXIOS[ CPU ].inputs; i++ ) {

    // PwrCtl
    if ( ( MAXIOS[ CPU ].pwrctl != NOPWRCTL ) && ( MAXIOS[ CPU ].pwrctl == i ) ) {

      io[ maxIO++ ] = pwrctl = new SwOSAnalogInput("PWRCTL", i, this, SWOSIO_POWER );

    // ftPwrDrive
    } else if (CPU == FTSWARMPWRDRIVE_1V141 ) {

      if ( i==4 ) {

        io[ maxIO++ ] = new SwOSDigitalInput( "EM", SWOS_NOPORT, this, SWOSIO_DIGITAL );

      } else {

        sprintf( name, "ES%d", i+1 );
        io[ maxIO++ ] = new SwOSDigitalInput( name, i, this, SWOSIO_DIGITAL );

      }

    // normal input
    } else { 
      sprintf( name, "A%d", i+1 );
      io[ maxIO++ ] = new SwOSDigitalInput( name, i, this, SWOSIO_DIGITAL );
    }

  }

  // LIDAR
  if ( extensionPort == FTSWARM_EXT_LIDAR ) {
    io[ maxIO++ ] = new SwOSLidarInput( "LIDAR", this );
  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalMotors( uint8_t maxIO, uint8_t motors ) {

  char name[10];

  for (uint8_t i=0; i<motors; i++) { 
    
    sprintf( name, "M%d", i+1 );
    if (CPU == FTSWARMPWRDRIVE_1V141 ) io[ maxIO++ ] = new SwOSStepper( name, i, this );
    else                               io[ maxIO++ ] = new SwOSDCMotor( name, i, this, SWOSIO_MOTOR );

  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalPixels( uint8_t maxIO ) {

  char name[10];
  for (uint8_t i=0; i<MAXLEDS; i++) { 

    sprintf( name, "LED%d", i+1 );
    io[ maxIO ] = new SwOSPixel( name, i, this );
    
    // store local pixels for setState
    if ( i < MAXIOS[CPU].pixels ) {
      if ( i == 0 ) pixel0 = (SwOSPixel*) io[ maxIO ];
      if ( i == 1 ) pixel1 = (SwOSPixel*) io[ maxIO ];
    }

    maxIO++;

  }

  return maxIO;

}

uint8_t SwOSCtrl::getIndex( SwOSIO *x ) {

  for ( uint8_t i=0; i<IOs; i++ ) {
    if ( io[i] == x ) return i;
  }

  return 255;

}

uint8_t SwOSCtrl::setupLocalServos( uint8_t maxIO, uint8_t servos ) {

  char name[10];

  for (uint8_t i=0; i<MAXIOS[ CPU ].rcservos; i++) {

    // test on sensor cable
    sprintf( name, "RCP%d", i+1 );
    SwOSAnalogInput *poti = new SwOSAnalogInput( name, i+7, this, SWOSIO_ANALOG );

    // need different filters
    poti->deleteFilter();
    poti->addFilter(new SwOSMovingAverage(5) );
    poti->addFilter(new SwOSSHR(2) );

    // need to read multiple times to get consistent values
    poti->operate();
    while (poti->getValueI32() == FILTER_INVALID ) {
      poti->operate();
    }

    poti->operate();
    if ( poti->getValueI32() < 1023 ) {
      sprintf( name, "RC%d", i+1 );
      SwOSDCMotor *motor = (SwOSDCMotor *) getIO( SWOSIO_RCMOTOR, i );
      io[ getIndex( motor ) ] = new SwOSRCServo( name, i, this, poti, motor );

    } else {

      delete poti;

    }

  }

  // just digital servos
  for ( uint8_t i=0; i<MAXIOS[ CPU ].servos; i++ ) {

    sprintf( name, "SERVO%d", i+1 );
    io[ maxIO++ ] = new SwOSDigitalServo( name, i, this);

  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalButtons( uint8_t maxIO ) {

  // create an outstanding HC165 object, if needed

  if ( ( MAXIOS[CPU].HC165 ) && (!hc165) ) hc165 = new SwOSHC165( "HC165", this );

  // create buttons
  for ( uint8_t i=0; i<MAXIOS[ CPU ].buttons; i++) {
    io[ maxIO++ ] = new SwOSDigitalInput( BUTTON[i], i, this, SWOSIO_BUTTON );
  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalJoysticks( uint8_t maxIO, SwOSCtrlConfig_t ctrlConfig  ) {

  char joy[10];
  char subio[10];

  SwOSDigitalInput* button;
  SwOSAnalogInput*  lr;
  SwOSAnalogInput*  fb;
  
  for ( uint8_t i=0; i<MAXIOS[ CPU ].joysticks; i++) {
    
    // joystick name
    sprintf( joy, "JOY%d", i+1 );

    // button is already created, just lookup
    button = (SwOSDigitalInput*) getIO( SWOSIO_BUTTON, FTSWARM_J1 + i );
    
    // lr poti
    sprintf( subio, "%sLR", joy);
    lr = new SwOSAnalogInput( subio, MAXIOS[ getCPU() ].firstJPoti + 2*i,     this, SWOSIO_JOYSTICK_POTI );
    io[ maxIO++] = (SwOSIO*) lr;
    
    // fb poti
    sprintf( subio, "%sFB", joy);
    fb = new SwOSAnalogInput( subio, MAXIOS[ getCPU() ].firstJPoti + 2*i + 1, this, SWOSIO_JOYSTICK_POTI );
    io[ maxIO++] = (SwOSIO*) fb;

    // create joystick
    io[ maxIO++ ] = new SwOSJoystick( joy, i, this, button, lr, fb );

  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalI2C( uint8_t maxIO, FtSwarmExtMode_t extensionPort ) {

  if (!local) return maxIO;

  // external I2C
  if ( ( GPIO_I2C[CPU][0][0] != GPIO_NUM_NC ) && ( ( nvs.extensionPort == FTSWARM_EXT_I2C_MASTER ) || ( nvs.extensionPort == FTSWARM_EXT_LIDAR ) ) ) {
    Wire.begin( GPIO_I2C[CPU][0][0], GPIO_I2C[CPU][0][1], 400000 );
  }

  // internal I2C
  if ( GPIO_I2C[CPU][1][0] != GPIO_NUM_NC ) {
    Wire1.begin( GPIO_I2C[CPU][1][0], GPIO_I2C[CPU][1][1], 400000 );
  }

  // use parameter to handle remote devices correctly
  if ( extensionPort == FTSWARM_EXT_I2C_SLAVE ) { io[ maxIO++ ] = new SwOSI2C ( "I2C", this, nvs.I2CAddr ); };

  // ftPwrDrive
  if ( CPU == FTSWARMPWRDRIVE_1V141 ) ftPwrDrive = new FtPwrDrive( 32, &Wire1 ); 

  // ftDuino
  if ( CPU == FTSWARMDUINO_1V141)     ftDuino    = new SwOSDuino( &Wire1 );

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalGyro( uint8_t maxIO ) {

  // initialize gyro if available
  if ( ( CPU == FTSWARMRS_2V0 ) || ( CPU == FTSWARMRS_2V1 ) || ( CPU == FTSWARMRC_1V140 ) || ( CPU == FTSWARMCONTROL_1V3UC ) )
    // TODO FTSWARMRC_1V140+ FTSWARMCONTROL_1V3UC Gyro implementation
    io[ maxIO++ ] = new SwOSGyroLSM( "GYRO", this );
  else
    io[ maxIO++ ] = new SwOSGyroMPU( "GYRO", this );

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalOLED( uint8_t maxIO ) {

  // initialize oled if available
  if ( MAXIOS[CPU].OLED ) {
    oled = new SwOSOLED( "OLED", this );
    io[ maxIO++ ] = oled;
  }

  return maxIO;

}

void SwOSCtrl::setupLocalCommonHardware( void ) {

  // Setup local common hardwrae like TIMER0

  // Timer0 to be used with PWM outputs
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_LOW_SPEED_MODE,
    .duty_resolution  = LEDC_TIMER_12_BIT,
    .timer_num        = LEDC_TIMER_0,
    .freq_hz          = 15000,
    .clk_cfg          = LEDC_AUTO_CLK,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // Enable complex fading
  ESP_ERROR_CHECK( ledc_fade_func_install( 0 ) );

}

SwOSCtrl::SwOSCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig  ):SwOSObj() {

  // copy master data
  this->IAmKelda      = ctrlConfig.IAmKelda;
  this->serialNumber  = SN;
  this->local         = local;
  this->CPU           = ctrlConfig.CPU;
  this->lastContact   = millis();
  this->extensionPort = ctrlConfig.extensionPort;
  this->macAddr.set( macAddr );
  this->pixels        = ctrlConfig.pixels;
  
  // set my name 
  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  uint8_t motors = 0;
  uint8_t servos = 0;

  // # of inputs & actors
  if (local) {

    // calculate needed IOs
    IOs = MAXIOS[ CPU ].inputs + MAXIOS[ CPU ].motors + MAXIOS[ CPU ].servos + MAXIOS[ CPU ].joysticks * 3 + MAXLEDS;

    // Buttons + HC165?
    if ( MAXIOS[ CPU ].buttons > 0 ) IOs = IOs + MAXIOS[ CPU ].buttons + 1;

    // OLED?
    if ( MAXIOS[CPU].OLED ) IOs++;

    motors = MAXIOS[ CPU ].motors;
    servos = MAXIOS[ CPU ].servos;

    switch ( extensionPort ) {

      // extensionPort is configured as additional outputs, add 2 motors
      case FTSWARM_EXT_OUTPUT: motors += 2; IOs += 2; break;

      // extensionPort is configured as additional servos, add 2 servos
      case FTSWARM_EXT_SERVO: servos +=2;   IOs += 2; break;

      // extensionPort is configured as LIDAR, add 1 io
      case FTSWARM_EXT_LIDAR: IOs++; break;

    }

  } else {
    IOs = ctrlConfig.IOs;
  }

  // define io pointer array dynamically
  io = (SwOSIO **) calloc( IOs, sizeof(SwOSIO*) );

  // define common hardware
  if (local) { 

    setupLocalCommonHardware();

    uint8_t maxIO = 0;
    maxIO = setupLocalI2C( maxIO, ctrlConfig.extensionPort );
    maxIO = setupLocalInputs( maxIO );
    maxIO = setupLocalMotors( maxIO, motors );
    maxIO = setupLocalServos( maxIO, servos );
    if ( MAXIOS[ CPU ].pixels ) maxIO = setupLocalPixels( maxIO );
    maxIO = setupLocalButtons( maxIO );
    maxIO = setupLocalJoysticks( maxIO, ctrlConfig );
    if ( ctrlConfig.gyro ) maxIO = setupLocalGyro( maxIO );
    if ( MAXIOS[CPU].OLED ) maxIO = setupLocalOLED( maxIO );

  }

}

SwOSCtrl::~SwOSCtrl() {
  
  if ( subscribedCtrlName ) delete subscribedCtrlName;
  
  for ( uint8_t i=0; i<IOs; i++) { if ( io[i] ) delete( io[i] ); }
 
}

void SwOSCtrl::lock( void ) {

   xSemaphoreTake( xAccessLock, portMAX_DELAY );

}


void SwOSCtrl::unlock( void ) {

   xSemaphoreGive( xAccessLock );

}


void SwOSCtrl::halt( void ) {


  for (uint8_t i=0; i<IOs; i++) if ( io[i] ) { io[i]->halt(); }

}

char *SwOSCtrl::subscribe( char *ctrlName  ) {
  
  isSubscribed = true;

  if (subscribedCtrlName) delete subscribedCtrlName;

  // only if I don't know my external name, store it
  if (!subscribedCtrlName) {
    subscribedCtrlName = (char *)malloc( strlen(ctrlName)+1 );
    strcpy( subscribedCtrlName, ctrlName );
  }

  return subscribedCtrlName;

}

void SwOSCtrl::unsubscribe( bool cascade ) {

  isSubscribed = false;

  if (!cascade) return;

  for ( uint8_t i=0; i<IOs; i++ ) { if ( io[i] ) io[i]->unsubscribe(); }

}

void SwOSCtrl::factorySettings( void ) {

  setAlias("");
  for ( uint8_t i=0; i<IOs; i++) { if ( io[i] ) io[i]->setAlias( "" ); }

}

SwOSIO *SwOSCtrl::getIO( const char *name) {

  for ( uint8_t i=0; i<IOs;  i++) { if ( ( io[i] ) && ( io[i]->equals(name) ) ) { return io[i]; } }
  return NULL;

}

SwOSIO *SwOSCtrl::getIO( SwOSIOType_t ioType, FtSwarmPort_t port) {

  // test all IOs
  for ( uint8_t i=0; i<IOs; i++ ) {

    // existing IO?
    if ( io[i] ) {

      if ( ( SWOSIOCLASS[ io[i]->getIOType() ] == SWOSIOCLASS[ ioType ] ) &&                             // IO has the same io class as requested
           ( ( SWOSIOCLASS[ ioType ] != SWOSIOCLASS_SINGULAR ) || ( io[i]->getIOType() == ioType ) ) &&  // if IO is class SWOSCLASS_SINGULAR, both io types need to be the same
           ( io[i]->getPort() == port )                                                                  // same port
         ) { 
        
        // compatible, but not the same type? need to convert it!
        if ( io[i]->getIOType() != ioType ) changeIOType( i, ioType );
        
        // all good now...
        return io[i]; 

      }

    }

  }

  return NULL;   

}

FtSwarmController_t SwOSCtrl::getType() {

  switch ( CPU ) {
    case FTSWARMDUINO_1V141:    return FTSWARMDUINO;
    case FTSWARMPWRDRIVE_1V141: return FTSWARMPWRDRIVE;
    case FTSWARMCAM_3V12:       return FTSWARMCAM;
    case FTSWARMCONTROL_1V3UC:
    case FTSWARMCONTROL_1V3:    return FTSWARMCONTROL;
    case FTSWARM_NOVERSION:     return FTSWARM_NOCTRL;
    default:                    return FTSWARM;
  };

}

void SwOSCtrl::operate() {

  // don't send packets to myself, so I need to now last reading time
  lastContact = millis();

  if (ftDuino) {

    // get data from ftDuino
    ftDuino->operate( );
    
    // errors during I2C communication?
    if ( ftDuino->getError() != 0 ) SWARM_LOG_ERROR( "ftDuino I2C connection broken." );

  }

  if (ftPwrDrive) {

    // get data from ftPwrDrive
    ftPwrDrive->operate( );
    
    // errors during I2C communication?
    if ( ftPwrDrive->getError() != 0 ) SWARM_LOG_ERROR( "ftPwrDrive I2C error %d.", ftPwrDrive->getError() );

  }

  if (hc165) hc165->operate();

  for (uint8_t i=0; i<IOs; i++) { if ( io[i] ) io[i]->operate(); }

}

bool SwOSCtrl::isInUse( void ) {

  for (uint8_t i=0; i<IOs; i++) { if ( ( io[i] ) && ( io[i]->isInUse() ) ) return true; }

  return false;

}


bool SwOSCtrl::isI2CSwarmCtrl( void ) {

  return ( CPU == FTSWARMPWRDRIVE_1V141 ) || ( CPU == FTSWARMDUINO_1V141 );

}

const char *SwOSCtrl::version( FtSwarmVersion_t v) {
  switch (v) {
  case FTSWARM_NOVERSION:     return "??";
  case FTSWARMJST_1V0:        return "1.0";
  case FTSWARMXL_1V00:        return "1.0.0";
  case FTSWARMJST_1V15:       return "1.15";
  case FTSWARMCONTROL_1V3UC:
  case FTSWARMCONTROL_1V3:    return "1.3";
  case FTSWARMRC_1V140:       return "1.4.0";
  case FTSWARMDUINO_1V141:  
  case FTSWARMPWRDRIVE_1V141: return "1.4.1";
  case FTSWARMRS_2V0:         return "2.0";
  case FTSWARMRS_2V1:         return "2.1.0";
  case FTSWARMCAM_3V12:       return "3.1.2";
  default:                    return  "??";
  }
}

const char *SwOSCtrl::getVersionCPU() {
  return version(CPU);
}

char *SwOSCtrl::getHostname( void ) {

  if ( (_alias) && (_alias[0]!='\0') ) {
    return _alias ;
  } else {
    return _name;
  }

}

bool able2ChangeIOType( SwOSIOType_t ioType ) {

  return ( ioType == SWOSIO_DIGITAL ) ||
         ( ioType == SWOSIO_SWITCH ) ||
         ( ioType == SWOSIO_REEDSWITCH ) ||
         ( ioType == SWOSIO_LIGHTBARRIER ) ||
         ( ioType == SWOSIO_ANALOG ) ||
         ( ioType == SWOSIO_VOLTMETER ) ||
         ( ioType == SWOSIO_OHMMETER ) ||
         ( ioType == SWOSIO_THERMOMETER ) ||
         ( ioType == SWOSIO_COUNTER )  ||
         ( ioType == SWOSIO_COUNTER )  ||
         ( ioType == SWOSIO_ROTARYENCODER )  ||
         ( ioType == SWOSIO_FREQUENCYMETER ) 
         ;
}


bool SwOSCtrl::changeIOType( uint8_t index, SwOSIOType_t newIOType ) {

  // do I exist?
  if ( index >= IOs )                        return false;
  if ( !io[index] )                          return false; 

  // get my type
  SwOSIOType_t oldIOType = io[index]->getIOType();

  // printf("changeIOType %s %d %d\n", io[index]->getName(), oldIOType, newIOType);

  // nothing changed?
  if ( oldIOType == newIOType ) return true;

  // in use?
  if ( io[index]->isInUse() ) {
    SWARM_LOG_ERROR( "Can't change IO Type. %s.%s is in use.", getName(), io[index]->getName() );
    return false;
  }

  // able to change?
  if ( ( SWOSIOCLASS[oldIOType] != SWOSIOCLASS[newIOType] ) || ( SWOSIOCLASS[oldIOType] == SWOSIOCLASS_SINGULAR ) ) {
    SWARM_LOG_ERROR("Can't change IO type of %s.%s  from %d to %d due to incompatible io types.", getName(), io[index]->getName(), oldIOType, newIOType );
    return false;
  }

  // now we're changing the type
  SwOSIO *oldIO;

  // RotaryEncoder needs a second port
  if ( newIOType == SWOSIO_ROTARYENCODER ) {

    // check if the next io is the secondary port
    if ( index+1 >= IOs )                                   return false;  // index in range?
    if ( !io[index+1] )                                     return false;  // io exists?
    if ( io[index+1]->isInUse() )                           return false;  // free to use?
    if ( !io[index+1]->isGPIOInput() )                      return false;  // is an input
    if ( io[index+1]->getPort() != io[index]->getPort()+1 ) return false;  // is the next port?
    
    // unregister secondary port
    oldIO = io[index+1];
    io[index+1] = NULL; 
    delete oldIO;

  }

  // register the new one
  oldIO = io[index];
  io[index] = createIO( newIOType, io[index]->getPort(), io[index]->getName(), io[index]->getAlias() );
  delete oldIO;

  // if it's an remote port, change remote site as well
  if ( !isLocal() ) {
    SwOSCom IOType( macAddr, serialNumber, CMD_SETIOTYPE );
    IOType.data.setIOTypeCmd.index     = index;
    IOType.data.setIOTypeCmd.newIOType = newIOType;
    IOType.send();
  }

  return true;

}

void SwOSCtrl::serializeEvents( Serialize *serialize ) {

  for (uint8_t i=0; i<IOs; i++) { 

    if ( ( io[i] ) && ( io[i]->isEventInput() ) ) ( ( SwOSInput* ) io[i])->serializeEvents( serialize );

  }

}

void SwOSCtrl::serialize( Serialize *serialize ) {

  serialize->startObject( );
  serialize->item( SERIALIZE_LITERAL_NAME, getHostname());
  serialize->item( SERIALIZE_LITERAL_SERIALNUMBER, serialNumber);
  serialize->item( SERIALIZE_LITERAL_TYPE, getType() );
  serialize->item( SERIALIZE_LITERAL_STATE, getState() );
  
  serialize->startArray( SERIALIZE_LITERAL_IO );
  serializeIO( serialize );
  serialize->endArray();

  serialize->endObject();

}

void SwOSCtrl::serializeIO( Serialize *serialize ) {

  for (uint8_t i=0; i<IOs; i++) { 
    
    if ( ( io[i] ) && ( io[i]->showInApi() ) ) {

      if ( io[i]->getIOType() == SWOSIO_PIXEL ) {

        // show pixels only, if they are marked as show in WebUI or the pixel is used
        if ( ( io[i]->getPort() < pixels ) || ( io[i]->isInUse() ) ) io[i]->serialize( serialize ); 

      } else if ( ( io[i]->getIOType() == SWOSIO_BUTTON ) && ( io[i]->getPort() >= FTSWARM_J1 ) && ( io[i]->getPort() <= FTSWARM_J2 ) ) {
        
        // don't show J1 und J2 as singular Buttons in UI, they're part of JOY1/JOY2

      } else {
        // all other stuff
        io[i]->serialize( serialize ); 
      }

    }

  }

}

SwOSIO* SwOSCtrl::createIO( SwOSIOType_t ioType, uint8_t port, const char *name, const char *alias ) {

  // new io
  SwOSIO *io;

  // joystick only
  SwOSDigitalInput* button;
  SwOSAnalogInput*  lr;
  SwOSAnalogInput*  fb;
  
  switch ( ioType ) {
    case SWOSIO_SWITCH:
    case SWOSIO_REEDSWITCH:
    case SWOSIO_LIGHTBARRIER:
    case SWOSIO_ULTRASONIC:
    case SWOSIO_DIGITAL:          io = new SwOSDigitalInput( name, port, this, ioType ); 
                                  break; 

    case SWOSIO_OHMMETER:
    case SWOSIO_THERMOMETER:
    case SWOSIO_VOLTMETER:
    case SWOSIO_LDR:
    case SWOSIO_POWER:
    case SWOSIO_ANALOG:           io = new SwOSAnalogInput( name, port, this, ioType );  
                                  break;

    case SWOSIO_LAMP:
    case SWOSIO_VALVE:
    case SWOSIO_COMPRESSOR:
    case SWOSIO_BUZZER:
    case SWOSIO_ENCODER:
    case SWOSIO_XSMOTOR:
    case SWOSIO_XMMOTOR:
    case SWOSIO_TRACTOR:
    case SWOSIO_WHEELDRIVE:
    case SWOSIO_MINIMOTOR:
    case SWOSIO_SMOTOR:
    case SWOSIO_POWERMOTOR:
    case SWOSIO_MMOTOR:
    case SWOSIO_RCMOTOR:
    case SWOSIO_MOTOR:            io = new SwOSDCMotor( name, port, this, ioType );
                                  break; 

    case SWOSIO_STEPPER:          io = new SwOSStepper( name, port, this);
                                  break; 

    case SWOSIO_BUTTON:           io = new SwOSDigitalInput( name, port, this, SWOSIO_BUTTON );
                                  break;

    case SWOSIO_JOYSTICK:         button = (SwOSDigitalInput*) getIO( SWOSIO_BUTTON, FTSWARM_J1 + port );
                                  lr     = (SwOSAnalogInput*)  getIO( SWOSIO_JOYSTICK_POTI, MAXIOS[ getCPU() ].firstJPoti + 2* port );
                                  fb     = (SwOSAnalogInput*)  getIO( SWOSIO_JOYSTICK_POTI, MAXIOS[ getCPU() ].firstJPoti + 2* port +1 );
                                  io     = new SwOSJoystick( name, port, this, button, lr, fb );
                                  break; 

    case SWOSIO_PIXEL:            io = new SwOSPixel( name, port, this );  
                                  break; 

    case SWOSIO_SERVO:            io = new SwOSServo( name, port, this ); 
                                  break; 

    case SWOSIO_OLED:             io = new SwOSOLED( name, this ); 
                                  break; 

    case SWOSIO_GYRO:             io = new SwOSGyro( name, this ); 
                                  break; 

    case SWOSIO_I2C:              io = new SwOSI2C( name, this, 0 );
                                  break; 

    case SWOSIO_COUNTER:          io = new SwOSCounter( name, port, SWOS_NOPORT, this );       
                                  break; 

    case SWOSIO_FREQUENCYMETER:   io = new SwOSFrequencymeter( name, port, port + 1, this );
                                  break; 

    case SWOSIO_ROTARYENCODER:    io = new SwOSCounter( name, port, port + 1, this );    
                                  break; 

    case SWOSIO_CAM:              io = new SwOSCAM( name, this );                        
                                  break; 

    case SWOSIO_HC165:            io = new SwOSHC165( name, this );                      
                                  break; 

    case SWOSIO_LIDAR:            io = new SwOSLidarInput( name, this );                 
                                  break; 

    case SWOSIO_JOYSTICK_POTI:    // io = new SwOSJoystick( name, this );
                                  break;

    default:                      // This should newer happen
                                  SWARM_LOG_FATAL( "SwOSCtrl::createIO: Unkown ioType %d", ioType );
    }
  
  if (alias) io->setAlias( alias );

  return io;
  
}

SwOSMotor* SwOSCtrl::getMotor( uint8_t index ) {

  SwOSIO *io = this->io[ index ];
  
  if (!io)             return NULL;
  if (!io->isMotor() ) return NULL;

  return (SwOSMotor*) io;

}

SwOSMotor* SwOSCtrl::getMotor( char *name ) {

  SwOSIO *io = getIO( name );

  if (!io)             return NULL;
  if (!io->isMotor() ) return NULL;

  return (SwOSMotor*) io;

}

SwOSCAM* SwOSCtrl::getCAM( char *name ) {

  SwOSIO *io = getIO( name );

  if (!io)           return NULL;
  if (!io->isCAM() ) return NULL;

  return (SwOSCAM*) io;

}

SwOSCounter* SwOSCtrl::getCounter( uint8_t index ) {

  SwOSIO *io = this->io[ index ];

  if (!io)               return NULL;
  if (!io->isCounter() ) return NULL;

  return (SwOSCounter*) io;

}

SwOSI2C* SwOSCtrl::getI2C( uint8_t index ) {

  SwOSIO *io = this->io[ index ];

  if (!io)           return NULL;
  if (!io->isI2C() ) return NULL;

  return (SwOSI2C*) io;

}

SwOSPixel* SwOSCtrl::getPixel( char *name ) {

  SwOSIO *io = getIO( name );

  if (!io)             return NULL;
  if (!io->isPixel() ) return NULL;

  return (SwOSPixel*) io;

}

SwOSServo* SwOSCtrl::getServo( char *name ) {

  SwOSIO *io = getIO( name );

  if (!io)             return NULL;
  if (!io->isServo() ) return NULL;

  return (SwOSServo*) io;

}

SwOSServo* SwOSCtrl::getServo( uint8_t index ) {

  SwOSIO *io = this->io[ index ];

  if (!io)             return NULL;
  if (!io->isServo() ) return NULL;

  return (SwOSServo *) io;

}

SwOSStepper* SwOSCtrl::getStepper( uint8_t index ) {

  SwOSIO *io = this->io[ index ];

  if (!io)               return NULL;
  if (!io->isStepper() ) return NULL;

  return (SwOSStepper *) io;

}

bool SwOSCtrl::apiActorCmd( char *id, int cmd ) {
  // send a actor command (from api)

  SwOSMotor *io = getMotor( id );

  if (!io) return false;
  
  io->setMotionType( (FtSwarmMotion_t) cmd );
  return true;

}

bool SwOSCtrl::apiActorSpeed( char *id, int speed ) {
  // send a actor command (from api)

  SwOSMotor *io = getMotor( id );

  if (!io) return false;
  
  io->setSpeed( speed );
  io->apply();
  return true;

}

bool SwOSCtrl::apiLEDBrightness( char *id, int brightness ) {
  // send a LED command (from api)

  SwOSPixel *io = getPixel( id );
  if (!io) return false;
  
  io->setBrightness( brightness );
  return true;

}

bool SwOSCtrl::apiLEDColor( char *id, int color ) {
  // send a LED command (from api)

  SwOSPixel *io = getPixel( id );
  if (!io) return false;
  
  io->setColor( color );
  return true;

}

bool SwOSCtrl::apiServoOffset( char * id, int offset ) {
  // send a Servo command (from api)

  SwOSServo *io = getServo( id );

  if (!io) return false;
  
  io->setOffset( offset );
  return true;

}

bool SwOSCtrl::apiServoPosition( char * id, int position ) {
  // send a Servo command (from api)

  SwOSServo *io = getServo( id );
  if (!io) return false;
  
  io->setPosition( position );
  return true;
  
}

bool SwOSCtrl::apiCAMStreaming( char *id, bool onOff ) {
  // set CAM framzesize / resolution

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMFramesize( char *id, int framesize ) {
  // set CAM framzesize / resolution

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMQuality( char *id, int quality ) { 
  // set CAM quality

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMBrightness( char *id, int brightness ) { 
  // set CAM brightness

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMContrast( char *id, int contrast ) { 
  // set CAM contrast

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMSaturation( char *id, int saturation ) { 
  // set CAM saturation

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMSpecialEffect( char *id, int effect ) { 
  // set CAM special effect

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMWbMode( char *id, int wbMode ) { 
  // set CAM wbMode

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMVFlip( char *id,bool vFlip ) { 
  // set CAM V-Flip 

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMHMirror( char *id,bool hMirror ) { 
  // set CAM hMirror

  // will be implemented in SwOSSwarmCAM
  return false;

}

void SwOSCtrl::setState( SwOSState_t state, uint8_t members, char *SSID ) {
  // visualizes controller's state like booting, error,...

  this->state = state;

  // *** classic ftSwarm ***
  if (pixel0) pixel0->setColor( LEDCOLOR0[state] );
  if (pixel1) pixel1->setColor( LEDCOLOR1[state] );
  
  // *** ftSwarmControl ***
  if (!oled) return;

  // rember old values
  uint8_t sx, sy;
  oled->getTextSize( &sx, &sy );
  int16_t cx, cy;
  oled->getCursor( &cx, &cy );
  
  int16_t w = oled->getWidth();

  // clear status bar
  oled->drawRect( 0, -YELLOWPIXELS, w, YELLOWPIXELS, true, false );

  // status message
  if ( ( state == RUNNING ) && (SSID) ) {
    char _SSID[15];
    strncpy( _SSID, SSID, 14 );
    oled->write( _SSID, w/2, -YELLOWPIXELS, FTSWARM_ALIGNCENTER, false );
  } else {
    oled->write( (char *) OLEDMSG[state], w/2, -YELLOWPIXELS, FTSWARM_ALIGNCENTER, false );
  }

  // members
  if ( members > 0) {
    char m[10];
    sprintf( m, "%d", members );
    oled->write( m, w, -YELLOWPIXELS, FTSWARM_ALIGNRIGHT, false );
  }

  // Kelda
  if (IAmKelda) oled->write( (char *) "K", 0, -YELLOWPIXELS, FTSWARM_ALIGNLEFT, false );

  // cool line
  oled->drawLine( 0, -5, w, -5, true );

  // restore values
  oled->setCursor( cx, cy );
  oled->setTextSize( sx, sy );

  // show on display
  oled->display();
   
}

void SwOSCtrl::identify( void ) {
  
  if (local) {
    setState( IDENTIFY );
  
  } else {
    SwOSCom identify( macAddr, serialNumber, CMD_IDENTIFY );
    identify.send();

  }

}

unsigned long SwOSCtrl::networkAge( void ) { 

  unsigned long age = millis() - lastContact;

  return age;

}

bool SwOSCtrl::setPixel( SwOSCom *com ) {

  if (!io[com->data.pixelCmd.index]) return false;
  if (!io[com->data.pixelCmd.index]->isPixel() ) return false;

  SwOSPixel *pixel = (SwOSPixel *)io[com->data.pixelCmd.index];
  pixel->setBrightness( com->data.pixelCmd.brightness );
  pixel->setColor( com->data.pixelCmd.color );

  return true;

}

bool SwOSCtrl::setActorSpeed( SwOSCom *com ) {

  SwOSMotor *io = getMotor( com->data.actorSpeedCmd.index );
  if (!io) return false;
  
  io->setMotionType( com->data.actorSpeedCmd.motionType );
  io->setAcceleration( com->data.actorSpeedCmd.rampUpT, com->data.actorSpeedCmd.rampUpY );
  io->setSpeed( com->data.actorSpeedCmd.speed );
  io->apply();
  
  return true;

}

bool SwOSCtrl::setStepperDistance( SwOSCom *com ) {

  SwOSStepper *io = getStepper( com->data.actorSpeedCmd.index );
  if (!io) return false;

  io->setDistance( com->data.actorStepperCmd.paraml, com->data.actorStepperCmd.paramb );  
  return true;

}

bool SwOSCtrl::setStepperPosition( SwOSCom *com ) {

  SwOSStepper *io = getStepper( com->data.actorSpeedCmd.index );
  if (!io) return false;

  io->setPosition( com->data.actorStepperCmd.paraml );
  return true;

}

bool SwOSCtrl::stepperHoming( SwOSCom *com ) {

  SwOSStepper *io = getStepper( com->data.actorSpeedCmd.index );
  if (!io) return false;

  io->homing( com->data.actorStepperCmd.paraml ); 
  return true;

}

bool SwOSCtrl::setStepperHomingOffset( SwOSCom *com ) {

  SwOSStepper *io = getStepper( com->data.actorSpeedCmd.index );
  if (!io) return false;

  io->setHomingOffset( com->data.actorStepperCmd.paraml ); 
  return true;

}

bool SwOSCtrl::stepperStartStop( SwOSCom *com ) {

  SwOSStepper *io = getStepper( com->data.actorStepperCmd.index );
  if (!io) return false;

  io->startStop( com->data.actorStepperCmd.paramb ); 
  return true;

}

bool SwOSCtrl::resetCounter( SwOSCom *com ) {

  SwOSCounter *io = getCounter( com->data.counterCmd.index );
  if (!io) return false;

  io->resetCounter();
  return true;

}

bool SwOSCtrl::userEvent( SwOSCom *com ) {

  if ( com->data.userEventCmd.trigger ) {

    // send trigger event to local procedure
    if ( xQueueSend( myOSNetwork.userEvent, com, ESPNOW_MAXDELAY ) != pdTRUE ) {
      SWARM_LOG_ERROR( "Can't send data to user event." );
    }

  } else {

    // got some user event data
    if (isSubscribed) {
      printf("S: %s", subscribedCtrlName );
      for ( uint8_t i=0; i<com->data.userEventCmd.size; i++ ) printf(" %02X", com->data.userEventCmd.payload[i]);
      printf("\n");
    }
  }

  return true;

}

bool SwOSCtrl::setServo( SwOSCom *com ) {

  SwOSServo *io = getServo( com->data.servoCmd.index );
  if (!io) return false;
  
  io->setOffset( com->data.servoCmd.offset );
  io->setPosition( com->data.servoCmd.position );
  return true;

}

bool SwOSCtrl::I2CRegister( SwOSCom *com ) {

  SwOSI2C *io = getI2C( com->data.I2CRegisterCmd.index );
  if (!io) return false;
  
  io->setRegister( com->data.I2CRegisterCmd.reg, com->data.I2CRegisterCmd.value );
  return true;

}

bool SwOSCtrl::setParameter( SwOSCom *com ) {

  if ( com->data.parameterCmd.index >= IOs ) return false;

  SwOSIO *io = this->io[ com->data.parameterCmd.index ];
  
  if (!io) return false;

  io->setParameter( com->data.parameterCmd.parameter );

  return true;

}

bool SwOSCtrl::ioConfig( SwOSCom *com ) {

  uint8_t      index;
  SwOSIOType_t ioType;
  uint8_t      port;
  char         *name;
  char         *alias;

  while ( com->popIO( &index, &ioType,  &port, &name, &alias ) ) {

    if ( index == 254 ) {
      // hostname
      setName( name );
      setAlias( alias );
      setComState( COMSTATE_ONLINE );

    } else if ( index >= IOs ) {
      SWARM_LOG_ERROR( "SwOSCtrl::ioConfig: index out of range %X", index );

    } else if ( io[index] ) {
      // set IOType + alias name as transmitted
      changeIOType( index, ioType );
      io[index]->setAlias( alias );

    } else {
      // any type of io
      io[index] = createIO( ioType, port, name, alias );

    }

  }

  return true;

}

void SwOSCtrl::tick( void ) {
  lastContact = millis();
}

bool SwOSCtrl::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  tick();
    
  switch (com->data.cmd) {

    case CMD_SAVE:                    save( com->data.saveCmd.scope ); return true;
    case CMD_REBOOT:                  ESP.restart();
    case CMD_SETWIFI:                 setWifi( com->data.wifiCmd.mode, com->data.wifiCmd.SSID, com->data.wifiCmd.PSK ); return true;
    case CMD_STATE:                   return recvState( com );
    case CMD_SETPIXEL:                return setPixel( com );
    case CMD_SETACTORSPEED:           return setActorSpeed( com );
    case CMD_RESETCOUNTER:            return resetCounter( com );
    case CMD_SETSTEPPERDISTANCE:      return setStepperDistance( com );
    case CMD_SETSTEPPERPOSITION:      return setStepperPosition( com );
    case CMD_STEPPERHOMING:           return stepperHoming( com );
    case CMD_SETSTEPPERHOMINGOFFSET:  return setStepperHomingOffset( com );
    case CMD_STEPPERSTARTSTOP:        return stepperStartStop( com );
    case CMD_IDENTIFY:                identify(); return true;
    case CMD_USEREVENT:               return userEvent( com );
    case CMD_IOCONFIG:                return ioConfig( com );
    case CMD_SETIOTYPE:               return changeIOType( com->data.setIOTypeCmd.index, com->data.setIOTypeCmd.newIOType );
    case CMD_SETSERVO:                return setServo( com );
    case CMD_I2CREGISTER:             return I2CRegister( com );
    case CMD_SETPARAMETER:            return setParameter( com );
  }

  return false;

}

SwOSCom *SwOSCtrl::state2Com( MacAddr destination ) {

  SwOSCom *com = new SwOSCom( destination, serialNumber, CMD_STATE );
  uint8_t index = 0;
  uint8_t ptr = 0;
  uint8_t len;

  for ( uint8_t i=0; i<IOs; i++ ) {

    if ( io[i] ) {

      // add IO's index to payload buffer
      com->data.stateCmd.payload[ptr] = i;

      // write io's payload to buffer, return 0 if I'm don't have a state
      len = io[i]->pushState( &com->data.stateCmd.payload[ptr+1] );

      // move ptr to next io
      if (len>0) {

        // shouldn't happen at all
        if ( ptr + len + 3 > MAXSTATECMDPAYLOAD ) SWARM_LOG_FATAL( "STATE2COM PAYLOAD excceded." );

        // move ptr
        ptr = ptr + len + 1;
        index++;

      }

    }

  }

  // mark end of list
  com->data.stateCmd.items = index + 1;

  return com;

}

bool SwOSCtrl::recvState( SwOSCom *com ) {

  uint8_t ptr = 0;  // ptr in payload buffer
  uint8_t index;    // index of io

  for ( uint8_t i=0; i<com->data.stateCmd.items; i++ ) {
    index = com->data.stateCmd.payload[ptr++];
    if ( (index < IOs) && (io[index]) ) ptr += io[index]->popState( &(com->data.stateCmd.payload[ptr]) );
  }

  return true;
 
} 

void SwOSCtrl::registerMe( SwOSCom *com ){

  if (!com) return;

  // controller data
  com->data.registerCmd.ctrlConfig.ctrlType      = getType();
  com->data.registerCmd.ctrlConfig.CPU           = getCPU();
  com->data.registerCmd.ctrlConfig.IAmKelda      = IAmKelda;
  com->data.registerCmd.ctrlConfig.extensionPort = extensionPort;
  com->data.registerCmd.ctrlConfig.IOs           = IOs;
  com->data.registerCmd.ctrlConfig.pixels        = pixels;
  
  // swarm data
  strcpy( com->data.registerCmd.swarmName, nvs.swarmName );
  com->data.registerCmd.swarmPIN = nvs.swarmPIN; 


}

void SwOSCtrl::saveToNVS( void ) {

  nvs_handle_t my_handle;
  ESP_ERROR_CHECK( nvs_open(NVSNAMESPACE, NVS_READWRITE, &my_handle) );

  SwOSObj::saveToNVS( my_handle );

  for ( uint8_t i=0; i<IOs; i++) if (io[i]) io[i]->saveToNVS( my_handle );

  ESP_ERROR_CHECK( nvs_commit( my_handle ) );

}

void SwOSCtrl::loadFromNVS( void ) {

  nvs_handle_t my_handle;
  ESP_ERROR_CHECK( nvs_open( NVSNAMESPACE, NVS_READONLY, &my_handle) );

  SwOSObj::loadFromNVS( my_handle );
  for ( uint8_t i=0; i<IOs; i++) if (io[i]) io[i]->loadFromNVS( my_handle );

}

void SwOSCtrl::sendIOConfig( MacAddr destination ) {

  SwOSCom ioConfig( destination, serialNumber, CMD_IOCONFIG );

  // IOs
  for (uint8_t i=0; i<IOs;i++) if (io[i]) ioConfig.pushIO( i, io[i]->getIOType(), io[i]->getPort(), io[i]->getName(), io[i]->getAlias() ); 

  // hostname - identifies last data
  ioConfig.pushHostname( getName(), getAlias() ); 

  // send
  ioConfig.flushBuffer();

}

bool SwOSCtrl::hasGyro( void ) {
  // test if HW has a gyro

  // already initialized or HW with integrated gyro
  if ( ( CPU == FTSWARMRS_2V0 ) ||
       ( CPU == FTSWARMRS_2V1 ) ||
       ( CPU == FTSWARMRC_1V140 ) ||
       ( CPU == FTSWARMCONTROL_1V3UC )
     ) return true;

  // check on MPU6050
  Wire.beginTransmission(0x68);
  return (Wire.endTransmission(true) == 0);

}

bool SwOSCtrl::hasExtPort( void ) {

  return HASEXTPORT[CPU];

}

void SwOSCtrl::setMicrostepMode( uint8_t mode ) {
  // set microstep mode
  
  microstepMode = mode;

  if (!isLocal()) {

    // send remote
    SwOSCom cmd( macAddr, serialNumber, CMD_SETMICROSTEPMODE );
    cmd.data.ctrlCmd.microstepMode = mode;
    cmd.send( );

  } else if ( ( getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( ftPwrDrive ) ) {
    // set local
    ftPwrDrive->setMicrostepMode( mode );
  }

}

uint8_t SwOSCtrl::getMicrostepMode( void ) {
  // get microstep mode
  
  return microstepMode;

}

void SwOSCtrl::deleteEvents( void ) {

  SwOSInput* input;

  for ( uint8_t i=0; i<IOs; i++ ) {

    if ( ( io[i] ) && ( io[i]->isEventInput() ) ) {

      input = (SwOSInput *) io[i];
      io[i]->lock();
      input->deleteEvents();
      io[i]->unlock();

    }

  }

}

void SwOSCtrl::setWifi( FtSwarmWifi_t mode, char *SSID, char*PSK ) {

  if (isLocal()) {
    nvs.wifiMode = mode;
    strcpy( nvs.wifiSSID, SSID );
    strcpy( nvs.wifiPwd, PSK );

  } else {
    SwOSCom cmd( macAddr, serialNumber, CMD_SETMICROSTEPMODE );
    cmd.data.wifiCmd.mode = mode;
    strcpy( cmd.data.wifiCmd.SSID, SSID );
    strcpy( cmd.data.wifiCmd.PSK, PSK );
    cmd.send();
  }

}

void SwOSCtrl::reboot( void ) {

  if (isLocal()) {
    ESP.restart();

  } else {
    SwOSCom cmd( macAddr, serialNumber, CMD_REBOOT );
    cmd.send();
  }

}

void SwOSCtrl::save( uint8_t scope ){

  if (local) {

    if ( ( scope == 0) || ( scope == 1 ) ) nvs.save();
    if ( ( scope == 0) || ( scope == 2 ) ) saveToNVS();
    if ( ( scope == 0) || ( scope == 3 ) ) nvs.saveEvents();

  } else {
    SwOSCom cmd( macAddr, serialNumber, CMD_SAVE );
    cmd.data.saveCmd.scope = scope;
    cmd.send();
  }

}