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
#include "SwOSHW/SwOSHWCAM.h"
#include "SwOSHW/SwOSHWHAL.h"

#include "SwOSCom.h"

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

  // GPIO-based
  for ( uint8_t i=0; i<MAXIOS[ CPU ].inputs; i++ ) {

    // PwrCtl
    if ( ( MAXIOS[ CPU ].pwrctl != NOPWRCTL ) && ( MAXIOS[ CPU ].pwrctl == i ) ) {

      io[ maxIO++ ] = new SwOSAnalogInput("PWRCTL", i, this, SWOSIO_VOLTMETER );

    // normal input
    } else { 
      sprintf( name, "A%d", i+1 );
      io[ maxIO++ ] = new SwOSDigitalInput( name, i, this, SWOSIO_DIGITAL );
    }

  }
  
  // PWRDRIVE
  if (CPU == FTSWARMPWRDRIVE_1V141 ) {

    for (uint8_t i=0; i<4; i++) { 

      if (i==4) {
        // general emergency button
        sprintf( name, "EM", i+1 );
        io[ maxIO++ ] = new SwOSDigitalInput( name, SWOS_NOPORT, this, SWOSIO_DIGITAL );

      } else {
        // normal endstops
        sprintf( name, "ES", i+1 );
        io[ maxIO++ ] = new SwOSDigitalInput( name, i, this, SWOSIO_DIGITAL );
      }

    }

  }

  // LIDAR
  if ( extensionPort == FTSWARM_EXT_LIDAR ) {
    io[ maxIO++ ] = new SwOSLidarInput( "LIDAR", this );
  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalMotors( uint8_t maxIO, uint8_t actors ) {

  char name[10];

  for (uint8_t i=0; i<actors; i++) { 
    
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
    io[ maxIO ] = new SwOSPixel( name, i, this);
    
    // store local pxiels for setState
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
    poti->addFilter(new SwOSSpike( 120, 30 ) );
    poti->addFilter(new SwOSMovingAverage(3) );

    // need to read multiple times to get consistent values
    poti->read();
    while (poti->getValueI32() == FILTER_INVALID ) {
      poti->read();
    }

    poti->read();
    if (( poti->getValueI32() > 0 ) && ( poti->getValueI32() < 4095 ) ) {
      printf("RCServo%d found.\n", i);
      sprintf( name, "RCSERVO%d", i+1 );
      SwOSMotor *motor = (SwOSMotor *) getIO( SWOSIO_MOTOR, i );
      io[ getIndex( motor ) ] = new SwOSRCServo( name, i, this, poti, motor );

    } else {

      delete poti;

    }

  }

  // just digital servos
  for ( uint8_t i=0; i<MAXIOS[ CPU ].servos; i++ ) {

    sprintf( name, "SERVO%d", i+1 );
    io[ maxIO++ ] = new SwOSServo( name, i, this);

  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalButtons( uint8_t maxIO ) {

  // create an outstanding HC165 object, if needed
  if ( ( CPU == FTSWARMCONTROL_1V3 ) && (!hc165)  ) hc165 = new SwOSHC165( "HC165", this );

  // create buttons
  for ( uint8_t i=0; i<MAXIOS[ CPU ].buttons; i++) {
    io[ maxIO++ ] = new SwOSDigitalInput( BUTTON[i], i, this, SWOSIO_BUTTON );
  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalJoysticks( uint8_t maxIO, SwOSCtrlConfig_t ctrlConfig  ) {

  char name[10];
  
  for ( uint8_t i=0; i<MAXIOS[ CPU ].joysticks; i++) {
    sprintf( name, "JOY%d", i+1 );
    io[ maxIO++ ] = new SwOSJoystick( name, i, this, ctrlConfig.zero[i][0], ctrlConfig.zero[i][1] );
  }

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalI2C( uint8_t maxIO, FtSwarmExtMode_t extensionPort ) {

  // Start I2C, if extention port is configured as I2C. 
  // ToDo I2CSlave 
  if ( ( local ) && 
       ( ( nvs.extensionPort == FTSWARM_EXT_I2C_MASTER ) ||
         ( nvs.extensionPort == FTSWARM_EXT_LIDAR ) 
       )
      ) {

    if ( GPIO_I2C[CPU][0][0] != GPIO_NUM_NC ) {
    
      // start I2C
      Wire.begin(  GPIO_I2C[CPU][0][0], GPIO_I2C[CPU][0][1] );
  
      // 400kHz only
      Wire.setClock(400000);

    }

  }

  // use parameter to handle remote devices correctly
  if ( extensionPort == FTSWARM_EXT_I2C_SLAVE ) { io[ maxIO++ ] = new SwOSI2C ( "I2C", this, nvs.I2CAddr ); };

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalGyro( uint8_t maxIO ) {

  // initialize gyro if available
  if ( ( CPU == FTSWARMRS_2V0 ) || ( CPU == FTSWARMRS_2V1 ) || ( CPU == FTSWARMRC_1V140 ) ) 
    io[ maxIO++ ] = new SwOSGyroLSM( "GYRO", this );
  else
    io[ maxIO++ ] = new SwOSGyroMPU( "GYRO", this );

  return maxIO;

}

uint8_t SwOSCtrl::setupLocalOLED( uint8_t maxIO ) {

  // initialize oled if available
  if ( CPU == FTSWARMCONTROL_1V3 ) {
    oled = new SwOSOLED( "OLED", this );
    io[ maxIO++ ] = oled;
  }

  return maxIO;

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
  
  // set my name 
  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  uint8_t actors = 0;
  uint8_t servos = 0;

  // # of inputs & actors
  if (local) {

    // calculate needed IOs
    IOs = MAXIOS[ CPU ].inputs + MAXIOS[ CPU ].actors + MAXIOS[ CPU ].servos + MAXIOS[ CPU ].joysticks + MAXLEDS;

    // Buttons + HC165?
    if ( MAXIOS[ CPU ].buttons > 0 ) IOs = IOs + MAXIOS[ CPU ].buttons + 1;

    // OLED?
    if ( CPU == FTSWARMCONTROL_1V3 ) IOs++;

    actors = MAXIOS[ CPU ].actors;
    servos = MAXIOS[ CPU ].servos;

    switch ( extensionPort ) {

      // extensionPort is configured as additional outputs, add 2 actors
      case FTSWARM_EXT_OUTPUT: actors += 2; IOs += 2; break;

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

    if ( CPU == FTSWARMPWRDRIVE_1V141 ) ftPwrDrive = new FtPwrDrive( 32, 5, 4 );
    if ( CPU == FTSWARMDUINO_1V141)     ftDuino = new SwOSDuino();

    uint8_t maxIO = 0;
    maxIO = setupLocalInputs( maxIO );
    maxIO = setupLocalMotors( maxIO, actors );
    maxIO = setupLocalServos( maxIO, servos );
    if ( MAXIOS[ CPU ].pixels ) maxIO = setupLocalPixels( maxIO );
    maxIO = setupLocalButtons( maxIO );
    maxIO = setupLocalJoysticks( maxIO, ctrlConfig );
    maxIO = setupLocalI2C( maxIO, ctrlConfig.extensionPort );
    if ( ctrlConfig.gyro ) maxIO = setupLocalGyro( maxIO );
    if ( CPU == FTSWARMCONTROL_1V3 ) maxIO = setupLocalOLED( maxIO );
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

  for ( uint8_t i=0; i<IOs; i++ ) {
    if ( ( io[i] ) &&                                                                                  // IO exists
         ( SWOSIOCLASS[ io[i]->getIOType() ] == SWOSIOCLASS[ ioType ] ) &&                             // IO has the same io class as requested
         ( ( SWOSIOCLASS[ ioType ] != SWOSIOCLASS_SINGULAR ) || ( io[i]->getIOType() == ioType ) ) &&  // if IO is class SWOSCLASS_SINGULAR, both io types need to be the same
         ( io[i]->getPort() == port )                                                                  // same port
       ) return io[i];
  }

  return NULL;   

}

char* SwOSCtrl::myType() {

  switch (CPU) {
    case FTSWARMCONTROL_1V3:    return (char *) "ftSwarmControl";
    case FTSWARMCAM_3V12:       return (char *) "ftSwarmCAM";
    case FTSWARMDUINO_1V141:    return (char *) "ftSwarmDuino";
    case FTSWARMPWRDRIVE_1V141: return (char *) "ftSwarmPwrDrive";
    default:                    return (char *) "ftSwarm";
  }

  return (char *) "";

}

FtSwarmController_t SwOSCtrl::getType() {
  return FTSWARM_NOCTRL;
}

void SwOSCtrl::read() {

  // don't send packets to myself, so I need to now last reading time
  lastContact = millis();

  if (ftDuino) {

    // get data from ftDuino
    ftDuino->read( );
    
    // errors during I2C communication?
    if ( ftDuino->getError() != 0 ) setState( ERROR );

  }

  if (ftPwrDrive) {

    // get data from ftDuino
    ftPwrDrive->read( );
    
    // ToDo
    // errors during I2C communication?
    // if ( ftPwrDrive->getError() != 0 ) setState( ERROR );

  }

  if (hc165) hc165->read();

  for (uint8_t i=0; i<IOs; i++) { if ( io[i] ) io[i]->read(); }

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

  // nothing changed?
  if ( oldIOType == newIOType ) return true;

  // in use?
  if ( io[index]->isInUse() ) {
    printf("\e[0;31mERROR: Can't change IO Type. %s.%s is in use.\e[0m\n", getName(), io[index]->getName() );
    setState( ERROR );
    return false;
  }

  // able to change?
  if ( ( SWOSIOCLASS[oldIOType] != SWOSIOCLASS[newIOType] ) || ( SWOSIOCLASS[oldIOType] == SWOSIOCLASS_SINGULAR ) ) {
    printf("\e[0;31mERROR: Can't change IO type of %s.%s  from %d to %d due to incompatible io types.\e[0m\n", getName(), io[index]->getName(), oldIOType, newIOType );
    setState( ERROR );
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
    if ( !io[index+1]->isInput() )                          return false;  // is an input
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

void SwOSCtrl::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  json->variable( "name", getHostname());
  json->variableUI8( "id", id);
  json->variableUI16( "serialNumber", serialNumber);
  json->variable( "type", myType() );
  
  json->startArray( "io" );
  jsonizeIO( json, id );
  json->endArray();

  json->endObject();

}

void SwOSCtrl::jsonizeIO( JSONize *json, uint8_t id ) {
  
  for (uint8_t i=0; i<IOs; i++) { if ( io[i] ) io[i]->jsonize( json, id ); }

}

SwOSIO* SwOSCtrl::createIO( SwOSIOType_t ioType, uint8_t port, char *name, char *alias ) {

  SwOSIO *io;

  switch ( ioType ) {
    case SWOSIO_SWITCH:
    case SWOSIO_REEDSWITCH:
    case SWOSIO_LIGHTBARRIER:
    case SWOSIO_ULTRASONIC:
    case SWOSIO_DIGITAL:         io = new SwOSDigitalInput( name, port, this, ioType ); break; 
    case SWOSIO_OHMMETER:
    case SWOSIO_THERMOMETER:
    case SWOSIO_VOLTMETER:
    case SWOSIO_LDR:
    case SWOSIO_ANALOG:          io = new SwOSAnalogInput( name, port, this, ioType );  break;
    case SWOSIO_LAMP:
    case SWOSIO_VALVE:
    case SWOSIO_COMPRESSOR:
    case SWOSIO_BUZZER:
    case SWOSIO_ENCODER:
    case SWOSIO_XMMOTOR:
    case SWOSIO_TRACTOR:
    case SWOSIO_MOTOR:           io = new SwOSMotor( name, port, this, ioType );        break; 
    case SWOSIO_STEPPER:         io = new SwOSStepper( name, port, this);               break; 
    case SWOSIO_BUTTON:          io = new SwOSDigitalInput( name, port, this, SWOSIO_BUTTON ); break;
    case SWOSIO_JOYSTICK:        io = new SwOSJoystick( name, port, this, 0, 0 );       break; 
    case SWOSIO_PIXEL:           io = new SwOSPixel( name, port, this );                break;
    case SWOSIO_SERVO:           io = new SwOSServo( name, port, this );                break; 
    case SWOSIO_OLED:            io = new SwOSOLED( name, this );                       break;
    case SWOSIO_GYRO:            io = new SwOSGyro( name, this );                       break;
    case SWOSIO_I2C:             io = new SwOSI2C( name, this, 0 );                     break;
    case SWOSIO_COUNTER:         io = new SwOSCounter( name, port, port, this );        break;
    case SWOSIO_FREQUENCYMETER:  io = new SwOSFrequencymeter( name, port, port, this ); break;
    case SWOSIO_ROTARYENCODER:   io = new SwOSCounter( name, port, port + 1, this );    break;
    case SWOSIO_CAM:             io = new SwOSCAM( name, this );                        break;
    case SWOSIO_HC165:           io = new SwOSHC165( name, this );                      break;
    case SWOSIO_LIDAR:           io = new SwOSLidarInput( name, this );                 break;
    default:                     // This should newer happen
                                 ESP_LOGE( LOGFTSWARM, "SwOSCtrl::createIO: Unkown ioType %d", ioType );
                                 setState( ERROR );
                                 forever( "" );
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

  // *** classic ftSwarm ***
  if (pixel0) pixel0->setColor( LEDCOLOR0[state] );
  if (pixel1) pixel1->setColor( LEDCOLOR0[state] );
  
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

bool SwOSCtrl::saveAlias2NVS( SwOSCom *com ) {
  // save in local nvs

  nvs_handle_t my_handle;

  ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );
  saveAliasToNVS( my_handle );
  ESP_ERROR_CHECK( nvs_commit( my_handle ) );
  
  return true;

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
      ESP_LOGE( LOGFTSWARM, "Can't send data to user event." );
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
      ESP_LOGE( LOGFTSWARM, "SwOSCtrl::ioConfig: index out of range %X", index );

    } else if ( io[index] ) {
      ESP_LOGE( LOGFTSWARM, "SwOSCtrl::ioConfig: io exists already %X", index );

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

    case CMD_SAVEALIAS2NVS:           return saveAlias2NVS( com );
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
        if ( ptr + len + 3 > MAXSTATECMDPAYLOAD ) {
          printf( "\e[0;31mERROR: STATE2COM PAYLOAD excceded %s\e[0m\n" );
          setState( ERROR );
          forever( "" );
        }

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

  uint8_t ptr = 0;

  for ( uint8_t i=0; i<com->data.stateCmd.items; i++ ) {
    if (io[i]) ptr += io[i]->popState( &(com->data.stateCmd.payload[ptr]) );
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
  
  // swarm data
  strcpy( com->data.registerCmd.swarmName, nvs.swarmName );
  com->data.registerCmd.swarmPIN = nvs.swarmPIN; 


}

// ToDO

void SwOSCtrl::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSObj::saveAliasToNVS( my_handle );

  for ( uint8_t i=0; i<IOs; i++) if (io[i]) io[i]->saveAliasToNVS( my_handle );

}

void SwOSCtrl::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSObj::loadAliasFromNVS( my_handle );
  for ( uint8_t i=0; i<IOs; i++) if (io[i]) io[i]->loadAliasFromNVS( my_handle );

}

void SwOSCtrl::sendIOConfig( MacAddr destination ) {

  SwOSCom ioConfig( destination, serialNumber, CMD_IOCONFIG );

  // IOs
  for (uint8_t i=0; i<IOs;i++) if (io[i]) ioConfig.pushIO( i, io[i]->getIOType(), io[i]->getPort(), io[i]->getName(), io[i]->getAlias() ); 

  // hostname - identifies last data
  ioConfig.pushHostname( getHostname(), getAlias() ); 

  // send
  ioConfig.flushBuffer();

}

bool SwOSCtrl::hasGyro( void ) {
  // test if HW has a gyro

  // already initialized or HW with integrated gyro
  if ( ( CPU == FTSWARMRS_2V0 ) ||
       ( CPU == FTSWARMRS_2V1 ) ||
       ( CPU == FTSWARMRC_1V140 )
     ) return true;

  // check on MPU6050
  return Wire.requestFrom( 0x68, 1 );

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