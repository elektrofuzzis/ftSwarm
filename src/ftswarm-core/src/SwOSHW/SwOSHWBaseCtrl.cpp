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
#include "SwOSHW/SwOSHWHAL.h"

#include "SwOSCom.h"
 
/***************************************************
 *
 *   SwOSCtrl - Base class for controllers.
 *
 ***************************************************/

void SwOSCtrl::setupLocalInputs( FtSwarmExtMode_t extensionPort ) {

  for (uint8_t i=0; i<inputs; i++) { 
    
    if (CPU == FTSWARMPWRDRIVE_1V141 ) {

      if (i==4) {
        // general emergency button
        input[i] = new SwOSDigitalInput("EM", SWOS_NOPORT, this );
      } else {
        // normal endstops
        input[i] = new SwOSDigitalInput("ES", i, this );
      }

    } else {

      // PwrCtl
      if ( ( MAXIOS[ CPU ].pwrctl != NOPWRCTL ) && ( MAXIOS[ CPU ].pwrctl == i ) ) {
        input[i] = new SwOSAnalogInput("A", i, this );
        input[i]->setAlias( "PwrCtl" );
        input[i]->setSensorType( FTSWARM_VOLTMETER );

      // LIDAR
      } else if ( ( extensionPort == FTSWARM_EXT_LIDAR ) && ( i+1 == inputs )) {
        input[i] = new SwOSLidarInput( "LIDAR", SWOS_NOPORT, this );

      // normal input
      } else { 
        input[i] = new SwOSDigitalInput("A", i, this );
      }
    }

  }

}

void SwOSCtrl::setupLocalActors( void ) {

  for (uint8_t i=0; i<actors; i++) { 
    
    actor[i] = new SwOSActor("M", i, this );

  }

}

void SwOSCtrl::setupLocalPixels( void ) {

  for (uint8_t i=0; i<MAXLEDS; i++) { 
    led[i] = new SwOSPixel("LED", i, this);
  }

}

void SwOSCtrl::setupLocalServos( FtSwarmExtMode_t extensionPort ) {

  for (uint8_t i=0; i<servos; i++) {

    if ( ( CPU == FTSWARMRC_1V140 ) && ( GPIO_INPUT[CPU][7+i].io != GPIO_NUM_NC ) ) { 

      // test on sensor cable
      SwOSAnalogInput *poti = new SwOSAnalogInput( "RCP", i+7, this );

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
        servo[i] = new SwOSRCServo( "RCSERVO", i, this, poti, actor[i] );
        actor[i] = NULL;
      } else {
        delete poti;
      }

    } else {
      // just a modern servo
      servo[i] = new SwOSServo("SERVO", i, this);

    }

  }

}

void SwOSCtrl::setupLocalI2C( FtSwarmExtMode_t extensionPort ) {

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
  if ( extensionPort == FTSWARM_EXT_I2C_SLAVE ) { I2C = new SwOSI2C ( "I2C", this, nvs.I2CAddr ); };

}

void SwOSCtrl::setupLocalGyro( bool gyroOn ) {

    // initialize gyro if available
  if ( gyroOn  ) { 
    if    ( ( CPU == FTSWARMRS_2V0 ) || 
            ( CPU == FTSWARMRS_2V1 ) ||
            ( CPU == FTSWARMRC_1V140 ) ) gyro = new SwOSGyroLSM( "GYRO", this );
    else                                  gyro = new SwOSGyroMPU( "GYRO", this );
  }

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

  // # of inputs & actors
  if (local) {

    inputs = MAXIOS[ CPU ].inputs;
    actors = MAXIOS[ CPU ].actors;
    leds   = MAXIOS[ CPU ].leds;
    servos = MAXIOS[ CPU ].servos;

    switch ( extensionPort ) {

      // extensionPort is configured as additional outputs, add 2 actors
      case FTSWARM_EXT_OUTPUT: actors += 2; break;

      // extensionPort is configured as additional servos, add 2 servos
      case FTSWARM_EXT_SERVO: servos +=2; break;

      // extensionPort is configured as LIDAR, add 1 input
      case FTSWARM_EXT_LIDAR: inputs++; break;

    }

  } else {

    inputs = ctrlConfig.inputs;
    actors = ctrlConfig.actors;
    leds   = ctrlConfig.leds;
    servos = ctrlConfig.servos;
  }

  // define io pointer array dynamically
  input = (SwOSInput **) calloc( inputs, sizeof(SwOSInput*) );
  actor = (SwOSActor **) calloc( actors, sizeof(SwOSActor*) );
  servo = (SwOSServo **) calloc( servos, sizeof(SwOSServo*) );

  // define common hardware
  if (local) {
    setupLocalInputs( ctrlConfig.extensionPort );
    setupLocalActors();
    setupLocalServos( ctrlConfig.extensionPort );
    setupLocalPixels();
    setupLocalI2C( ctrlConfig.extensionPort );
    setupLocalGyro( ctrlConfig.gyro );
  }

}

SwOSCtrl::~SwOSCtrl() {
  
  if ( subscribedCtrlName ) delete subscribedCtrlName;
  
  for ( uint8_t i=0; i<inputs; i++)  { if ( input[i] ) delete( input[i] ); }
  for ( uint8_t i=0; i<actors; i++)  { if ( actor[i] ) delete( actor[i] ); }
  for ( uint8_t i=0; i<MAXLEDS;i++)  { if ( led[i] )   delete( led[i] ); }
  for ( uint8_t i=0; i<servos; i++ ) { if ( servo[i] ) delete servo[i]; }

  if (gyro) delete( gyro );
  if (I2C)  delete( I2C );
  
}

void SwOSCtrl::lock( void ) {

   xSemaphoreTake( xAccessLock, portMAX_DELAY );

}


void SwOSCtrl::unlock( void ) {

   xSemaphoreGive( xAccessLock );

}


void SwOSCtrl::halt( void ) {


  for (uint8_t i=0; i<actors; i++) { 
    if ( actor[i] ) { actor[i]->setSpeed(0); actor[i]->apply(); }
  }

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

  for ( uint8_t i=0; i<inputs; i++ ) { if ( input[i] ) input[i]->unsubscribe(); }
  for ( uint8_t i=0; i<actors; i++ ) { if ( actor[i] ) actor[i]->unsubscribe(); }
  for ( uint8_t i=0; i<MAXLEDS; i++) { if ( led[i] )   led[i]->unsubscribe(); }
  for ( uint8_t i=0; i<servos; i++ ) { if ( servo[i] ) servo[i]->unsubscribe(); }

  if (gyro) gyro->unsubscribe();
  if (I2C)  I2C->unsubscribe();
  
}

void SwOSCtrl::factorySettings( void ) {

  setAlias("");
  for ( uint8_t i=0; i<inputs; i++) { if ( input[i] ) input[i]->setAlias( "" ); }
  for ( uint8_t i=0; i<actors; i++) { if ( actor[i] ) actor[i]->setAlias( "" ); }
  for ( uint8_t i=0; i<MAXLEDS; i++)   { if ( led[i] )   led[i]->setAlias( "" ); }
  for ( uint8_t i=0; i<servos; i++) { if ( servo[i] ) servo[i]->setAlias( "" ); }

  if (gyro) gyro->setAlias("");
  if (I2C)  I2C->setAlias("");

}

bool SwOSCtrl::cmdAlias( const char *obj, const char *alias) {

  char device[30];
  uint8_t pos = 0;
  uint8_t port;

  // scan for first digit
  while ( (!isdigit(obj[pos]) ) && (obj[pos]!='\0') ) pos++;

  // device not found?
  if (( pos==0) || ( pos>30) ) return false;

  // get device
  uint8_t i;
  for (i=0; i<pos;i++) { device[i] = toupper(obj[i]); }
  device[i]='\0';

  // get port
  port = atoi(&obj[pos])-1;

  // check on trailing digits only
  while ( obj[pos]!='\0') {
    if (!isdigit(obj[pos]) ) return false;
    pos++;
  }

  // now start interpreting
  if (strcmp( "FTSWARM", device) == 0)                                               { setAlias( alias );            return true; }
  else if ( ( strcmp(device, "A") == 0 )     && ( port < inputs) )                   { input[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "M") == 0 )     && ( port < actors) )                   { actor[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "LED") == 0 )   && ( port < MAXLEDS) && (led[port] )  ) { led[port]->setAlias(alias);   return true; }
  else if ( ( strcmp(device, "SERVO") == 0 ) && ( port < servos ) && (servo[port]) ) { servo[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "GYRO")  == 0 ) && ( gyro ) )                           { gyro->setAlias(alias);        return true; }
  else if ( ( strcmp(device, "I2C")   == 0 ) && ( I2C ) )                            { I2C->setAlias(alias);         return true; }
  
  // specific hardware?
  return cmdAlias( device, port, alias );

}

bool SwOSCtrl::cmdAlias( char *device, uint8_t port, const char *alias) {

  // own hardware options are already interpreted
  return false;

}

SwOSIO *SwOSCtrl::getIO( const char *name) {

  for ( uint8_t i=0; i<inputs;  i++) { if ( ( input[i] ) && ( input[i]->equals(name) ) ) { return input[i]; } }
  for ( uint8_t i=0; i<actors;  i++) { if ( ( actor[i] ) && ( actor[i]->equals(name) ) ) { return actor[i]; } }
  for ( uint8_t i=0; i<MAXLEDS; i++) { if ( ( led[i] )   && ( led[i]->equals(name) ) )   { return led[i]; } }
  for ( uint8_t i=0; i<servos;  i++) { if ( ( servo[i] ) && ( servo[i]->equals(name) ) ) { return servo[i]; } }

  if ( (gyro) && (gyro->equals(name) ) ) return gyro;
  if ( (I2C)  && (I2C->equals(name) ) )  return I2C;

  return NULL;
}

SwOSIO *SwOSCtrl::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  switch (ioType) {
    case FTSWARM_COUNTERINPUT:
    case FTSWARM_ROTARYINPUT:
    case FTSWARM_FREQUENCYINPUT:
    case FTSWARM_INPUT: 
    case FTSWARM_DIGITALINPUT:
    case FTSWARM_ANALOGINPUT : return ( ( port<inputs)?input[ port ]:NULL);
    
    case FTSWARM_ACTOR:        return ( ( port<actors)?actor[ port ]:NULL);
    
    case FTSWARM_PIXEL:        return ( ( port<MAXLEDS)?led[ port ]:NULL);

    case FTSWARM_SERVO:        return ( ( port<servos)?servo[port]:NULL);

    case FTSWARM_GYRO:         return gyro;

    case FTSWARM_I2C:          return I2C;
    
    default: return NULL;   

  }

}

char* SwOSCtrl::myType() {
  return (char *) "UNDEFINED";
}

FtSwarmController_t SwOSCtrl::getType() {
  return FTSWARM_NOCTRL;
}

void SwOSCtrl::read() {

  // don't send packets to myself, so I need to now last reading time
  lastContact = millis();

  for (uint8_t i=0; i<inputs; i++) { if ( input[i] ) input[i]->read();   }
  for (uint8_t i=0; i<actors; i++) { if ( actor[i] ) actor[i]->read();   }
  for (uint8_t i=0; i<servos; i++) { if ( servo[i] ) servo[i]->adjust(); }

  if (gyro) gyro->read();
  if (I2C) I2C->read();

}

bool SwOSCtrl::isInUse( void ) {

  for (uint8_t i=0; i<inputs; i++) { if ( ( input[i] ) && ( input[i]->isInUse() ) ) return true; }
  for (uint8_t i=0; i<actors; i++) { if ( ( actor[i] ) && ( actor[i]->isInUse() ) ) return true; }
  for (uint8_t i=0; i<servos; i++) { if ( ( servo[i] ) && ( servo[i]->isInUse() ) ) return true; }
  
  if ( ( gyro ) && ( gyro->isInUse() ) ) return true;
  if ( ( I2C )  && ( I2C->isInUse() ) )  return true;

  return false;

}


bool SwOSCtrl::isI2CSwarmCtrl( void ) {

  return ( CPU == FTSWARMPWRDRIVE_1V141 ) || ( CPU == FTSWARMDUINO_1V141 );

}

const char *SwOSCtrl::version( FtSwarmVersion_t v) {
  switch (v) {
  case FTSWARM_NOVERSION:     return "??";
  case FTSWARMJST_1V0:        return "1.0";
  case FTSWARMCONTROL_1V3:    return "1.3";
  case FTSWARMJST_1V15:       return "1.15";
  case FTSWARMRS_2V0:         return "2.0";
  case FTSWARMRS_2V1:         return "2.1.0";
  case FTSWARMCAM_3V12:       return "3.1.2";
  case FTSWARMDUINO_1V141:  
  case FTSWARMPWRDRIVE_1V141: return "1.4.1";
  case FTSWARMXL_1V00:        return "1.0.0";
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

bool isInputType( FtSwarmIOType_t ioType ) {

  return ( ioType == FTSWARM_DIGITALINPUT ) ||
         ( ioType == FTSWARM_ANALOGINPUT ) ||
         ( ioType == FTSWARM_ROTARYINPUT ) ||
         ( ioType == FTSWARM_COUNTERINPUT ) ||
         ( ioType == FTSWARM_FREQUENCYINPUT );
}

bool SwOSCtrl::changeIOType( uint8_t port, FtSwarmIOType_t oldIOType, FtSwarmIOType_t newIOType ) {

  // check on compatible IO types
  if ( !isInputType( oldIOType) ) return false;
  if ( !isInputType( newIOType) ) return false;

  // register the new one
  SwOSInput *io    = NULL;

  switch ( newIOType ) {

    case FTSWARM_DIGITALINPUT:    io = new SwOSDigitalInput("A", port, this ); 
                                  break;

    case FTSWARM_ANALOGINPUT:     io = new SwOSAnalogInput("A", port, this );
                                  break;

    case FTSWARM_COUNTERINPUT:    io = new SwOSCounter("A", port, SWOS_NOPORT, this ); 
                                  break;

    case FTSWARM_ROTARYENCODER:   io = new SwOSCounter("A", port, port+1, this ); 
                                  if ( port+1 < inputs ) { 
                                    // cleanup next input, it's used now
                                    SwOSInput *old = input[port+1];
                                    input[port+1] = NULL;
                                    if ( old ) delete old;
                                  }
                                  break;

    case FTSWARM_FREQUENCYINPUT:  io = new SwOSFrequencymeter("A", port, SWOS_NOPORT, this ); 
                                  break;

    default: return false;
  }

  // if old port exits, transfer needed properties and kill it
  if (input[port]) {
    char alias[MAXIDENTIFIER];
    strcpy( alias, input[port]->getAlias() );
    io->setAlias( alias );
    delete input[port];
  }

  // assign new port
  input[port] = io;

  // if it's an remote port, change remote site as well
  if ( !isLocal() ) {
    SwOSCom IOType( macAddr, serialNumber, CMD_CHANGEIOTYPE );
    IOType.data.changeIOTypeCmd.index     = port;
    IOType.data.changeIOTypeCmd.oldIOType = oldIOType;
    IOType.data.changeIOTypeCmd.newIOType = newIOType;
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
  
  for (uint8_t i=0; i<inputs; i++) { if ( input[i] ) input[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<actors; i++) { if ( actor[i] ) actor[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<leds;   i++) { if ( led[i]   ) led[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<servos; i++) { if ( servo[i] ) servo[i]->jsonize( json, id ); } 

  if (gyro)  { gyro->jsonize(json, id); }

}

bool SwOSCtrl::apiActorCmd( char *id, int cmd ) {
  // send a actor command (from api)

  // search IO
  for (uint8_t i=0; i<actors; i++) {

    if ( actor[i]->equals(id) ) {
      // found
      actor[i]->setMotionType( (FtSwarmMotion_t) cmd );
      return true;
    }
  }

  return false;
}

bool SwOSCtrl::apiActorSpeed( char *id, int speed ) {
  // send a actor command (from api)

  // search IO
  for (uint8_t i=0; i<actors; i++) {

    if ( actor[i]->equals(id) ) {
      // found
      actor[i]->setSpeed( speed );
      actor[i]->apply();
      return true;
    }
  }

  return false;
}

bool SwOSCtrl::apiLEDBrightness( char *id, int brightness ) {
  // send a LED command (from api)

  // search IO
  for (uint8_t i=0; i<MAXLEDS; i++) {

    if ( (led[i]) && ( led[i]->equals(id) ) ) {
      // found
      led[i]->setBrightness( brightness );
      return true;
    }
  }

  return false;

}

bool SwOSCtrl::apiLEDColor( char *id, int color ) {
  // send a LED command (from api)

  // search IO
  for (uint8_t i=0; i<MAXLEDS; i++) {

    if ( (led[i]) && ( led[i]->equals(id) ) ) {
      // found
      led[i]->setColor( color );
      return true;
    }
  }

  return false;

}

bool SwOSCtrl::apiServoOffset( char * id, int offset ) {
  // send a Servo command (from api)

  // search IO
  for (uint8_t i=0; i<MAXSERVOS; i++) {

    if ( (servo[i]) && ( servo[i]->equals(id) ) ) {
      // found
      servo[i]->setOffset( offset, false );
      return true;
    }
  }

  return false;

}

bool SwOSCtrl::apiServoPosition( char * id, int position ) {
  // send a Servo command (from api)

  // search IO
  for (uint8_t i=0; i<MAXSERVOS; i++) {

    if ( (servo[i]) && ( servo[i]->equals(id) ) ) {
      // found
      servo[i]->setPosition( position, false );
      return true;
    }
  }

  return false;

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


bool SwOSCtrl::maintenanceMode() {
  return false;
}

void SwOSCtrl::setState( SwOSState_t state, uint8_t members, char *SSID ) {
  // visualizes controller's state like booting, error,...
  
  if (led[0]) led[0]->setColor( LEDCOLOR0[state] );
  if (led[1]) led[1]->setColor( LEDCOLOR1[state] );

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

bool SwOSCtrl::setLED( SwOSCom *com ) {

  if (led[com->data.ledCmd.index]) {
    led[com->data.ledCmd.index]->setBrightness( com->data.ledCmd.brightness );
    led[com->data.ledCmd.index]->setColor( com->data.ledCmd.color );
  }

  return true;

}

bool SwOSCtrl::setSensorType( SwOSCom *com ) {

  if ( input[com->data.sensorCmd.index]->getIOType() == FTSWARM_DIGITALINPUT) { 
    ((SwOSDigitalInput *)input[com->data.sensorCmd.index])->setSensorType( com->data.sensorCmd.sensorType, com->data.sensorCmd.normallyOpen );
  } else {
    ((SwOSAnalogInput *)input[com->data.sensorCmd.index])->setSensorType( com->data.sensorCmd.sensorType );
  }

  return true;

}

bool SwOSCtrl::setActorSpeed( SwOSCom *com ) {

  actor[com->data.actorSpeedCmd.index]->setMotionType( com->data.actorSpeedCmd.motionType );
  actor[com->data.actorSpeedCmd.index]->setAcceleration( com->data.actorSpeedCmd.rampUpT, com->data.actorSpeedCmd.rampUpY );
  actor[com->data.actorSpeedCmd.index]->setSpeed( com->data.actorSpeedCmd.speed );
  actor[com->data.actorSpeedCmd.index]->apply();
  
  return true;

}

bool SwOSCtrl::resetCounter( SwOSCom *com ) {

  if ( ( input[com->data.counterCmd.index] ) && ( input[com->data.counterCmd.index]->getIOType() == FTSWARM_COUNTERINPUT ) )
    static_cast<SwOSCounter *>(input[com->data.counterCmd.index])->resetCounter();
  
  return true;

}

bool SwOSCtrl::setActorType( SwOSCom *com ) {

  actor[com->data.actorTypeCmd.index]->setActorType( com->data.actorTypeCmd.actorType, com->data.actorTypeCmd.highResolution, true );
  actor[com->data.actorTypeCmd.index]->apply();

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
  
  // set servo position + offset
  if (servo[com->data.servoCmd.index]) {
    servo[com->data.servoCmd.index]->setOffset( com->data.servoCmd.offset, true );
    servo[com->data.servoCmd.index]->setPosition( com->data.servoCmd.position, true );
  }

  return true;

}

SwOSIO* SwOSCtrl::ioConfig( FtSwarmIOType_t ioType, FtSwarmSensor_t sensorType, uint8_t port, char *name, char *alias ) {

  SwOSIO *io;

  switch ( ioType ) {
    case FTSWARM_INPUT:           io = new SwOSInput( name, port, this, sensorType );    break;
    case FTSWARM_DIGITALINPUT:    io = new SwOSDigitalInput( name, port, this );         break; 
    case FTSWARM_ANALOGINPUT:     io = new SwOSAnalogInput( name, port, this );          break;
    case FTSWARM_ACTOR:           io = new SwOSActor( name, port, this );                break; 
    case FTSWARM_BUTTON:          io = new SwOSButton( name, port, this ) ;              break;
    case FTSWARM_JOYSTICK:        io = new SwOSJoystick( name, port, this, 0, 0 );       break; 
    case FTSWARM_PIXEL:           io = new SwOSPixel( name, port, this );                break;
    case FTSWARM_SERVO:           io = new SwOSServo( name, port, this );                break; 
    case FTSWARM_OLED:            io = new SwOSOLED( name, this );                       break;
    case FTSWARM_GYRO:            io = new SwOSGyro( name, this );                       break;
    case FTSWARM_I2C:             io = new SwOSI2C( name, this, 0 );                     break;
    case FTSWARM_COUNTERINPUT:    io = new SwOSCounter( name, port, port, this );        break;
    case FTSWARM_FREQUENCYINPUT:  io = new SwOSFrequencymeter( name, port, port, this ); break;

    case FTSWARM_ROTARYINPUT:     
    case FTSWARM_CAM:             
    case FTSWARM_HC165:           
    default:                      // This should newer happen
                                  ESP_LOGE( LOGFTSWARM, "SwOSCtrl::ioConfig: Unkown ioType %d", ioType );
                                  setState( ERROR );
                                  forever("");
    }
  
  if (alias) io->setAlias( alias );

  return io;
  
}

bool SwOSCtrl::ioConfig( SwOSCom *com ) {

  SwOSIO          *io;
  FtSwarmIOType_t ioType;
  FtSwarmSensor_t sensorType;
  uint8_t         port;
  char            *name;
  char            *alias;

  while ( com->getNextIO( &ioType, &sensorType, &port, &name, &alias ) ) {

    if (ioType == FTSWARM_MAXIOTYPE ) {
      // hostname
      // setName( name );
      setAlias( alias );

    } else {
      // any type of io
      io = ioConfig( ioType, sensorType, port, name, alias );
      if      ( io->isInput() ) input[port] = (SwOSInput *) io;
      else if ( io->isActor() ) actor[port] = (SwOSActor *) io;
      else if ( io->isGyro() )  gyro        = (SwOSGyro  *) io;
      else if ( io->isI2C() )   I2C         = (SwOSI2C   *) io;
      // else if ( io->isOLED() )  OLED        = (SwOSOLED  *) io;
      else if ( io->isServo() ) servo[port] = (SwOSServo *) io;
      else if ( io->isPixel() ) led[port]   = (SwOSPixel *) io;
    }

  }

  return true;

}

bool SwOSCtrl::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  lastContact = millis();
    
  switch (com->data.cmd) {

    case CMD_SAVEALIAS2NVS:           return saveAlias2NVS( com );
    case CMD_STATE:                   return recvState( com );
    case CMD_SETLED:                  return setLED( com );
    case CMD_SETSENSORTYPE:           return setSensorType( com );
    case CMD_SETACTORSPEED:           return setActorSpeed( com );
    case CMD_RESETCOUNTER:            return resetCounter( com );
    case CMD_SETSTEPPERDISTANCE:      actor[com->data.actorStepperCmd.index]->setDistance( com->data.actorStepperCmd.paraml, com->data.actorStepperCmd.paramb, true ); return true;
    case CMD_SETSTEPPERPOSITION:      actor[com->data.actorStepperCmd.index]->setPosition( com->data.actorStepperCmd.paraml, true ); return true;
    case CMD_STEPPERHOMING:           actor[com->data.actorStepperCmd.index]->homing( com->data.actorStepperCmd.paraml ); return true;
    case CMD_SETSTEPPERHOMINGOFFSET:  actor[com->data.actorStepperCmd.index]->setHomingOffset( com->data.actorStepperCmd.paraml ); return true;
    case CMD_STEPPERSTARTSTOP:        actor[com->data.actorStepperCmd.index]->startStop( com->data.actorStepperCmd.paramb ); return true;
    case CMD_SETACTORTYPE:            return setActorType(com);
    case CMD_IDENTIFY:                identify(); return true;
    case CMD_USEREVENT:               return userEvent( com );
    case CMD_IOCONFIG:                return ioConfig( com );
    case CMD_CHANGEIOTYPE:            changeIOType( com->data.changeIOTypeCmd.index, com->data.changeIOTypeCmd.oldIOType, com->data.changeIOTypeCmd.newIOType ); return true;
    case CMD_SETSERVO:                return setServo( com );
    case CMD_I2CREGISTER:             if (I2C) I2C->setRegister( com->data.I2CRegisterCmd.reg, com->data.I2CRegisterCmd.value ); return true;

  }

  return false;

}

SwOSCom *SwOSCtrl::state2Com( MacAddr destination ) {

  SwOSCom *com = new SwOSCom( destination, serialNumber, CMD_STATE );

  int16_t FB, LR;

  for (uint8_t i=0; i<inputs; i++ ) { com->data.stateCmd.inputValue[i] = input[i]->getValueI32(); }; 
  if (gyro) gyro->state2com( com );
  if (I2C)  memcpy( com->data.stateCmd.i2cValue, I2C->myRegister, MAXI2CREGISTERS );

  return com;

}

bool SwOSCtrl::recvState( SwOSCom *com ) {
  
  for (uint8_t i=0; i<inputs; i++ ) { input[i]->setValue( com->data.stateCmd.inputValue[i] ); };
  if (gyro) gyro->recvState( com );
  if (I2C)  memcpy( I2C->myRegister,  com->data.stateCmd.i2cValue, MAXI2CREGISTERS );

  return true;
 
} 

void SwOSCtrl::registerMe( SwOSCom *com ){

  if (!com) return;

  // controller data
  com->data.registerCmd.ctrlConfig.ctrlType      = getType();
  com->data.registerCmd.ctrlConfig.CPU           = getCPU();
  com->data.registerCmd.ctrlConfig.IAmKelda      = IAmKelda;
  com->data.registerCmd.ctrlConfig.extensionPort = extensionPort;
  com->data.registerCmd.ctrlConfig.inputs        = inputs;
  com->data.registerCmd.ctrlConfig.actors        = actors;
  com->data.registerCmd.ctrlConfig.leds          = leds;
  com->data.registerCmd.ctrlConfig.servos        = servos;
  
  // swarm data
  strcpy( com->data.registerCmd.swarmName, nvs.swarmName );
  com->data.registerCmd.swarmPIN = nvs.swarmPIN; 


}

void SwOSCtrl::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSObj::saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<inputs; i++ )  if (input[i]) input[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<actors; i++ )  if (actor[i]) actor[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<MAXLEDS; i++ ) if (led[i])   led[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<servos; i++ )  if (servo[i]) servo[i]->saveAliasToNVS( my_handle );

  if (gyro) gyro->saveAliasToNVS( my_handle );
  if (I2C)  I2C->saveAliasToNVS( my_handle );
}

void SwOSCtrl::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSObj::loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<inputs; i++ )  if (input[i]) input[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<actors; i++ )  if (actor[i]) actor[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<MAXLEDS; i++ ) if (led[i])   led[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<servos; i++ )  if (servo[i]) servo[i]->loadAliasFromNVS( my_handle );

  if (gyro) gyro->loadAliasFromNVS( my_handle );
  if (I2C)  I2C->loadAliasFromNVS( my_handle );

}

void SwOSCtrl::sendIOConfig( MacAddr destination ) {

  SwOSCom ioConfig( destination, serialNumber, CMD_IOCONFIG );

  // hostname
  ioConfig.sendHostname( (char *)"HOSTNAME", getAlias() ); 

  // input
  for (uint8_t i=0; i<inputs;i++) if (input[i]) ioConfig.sendIO( input[i]->getIOType(), input[i]->getSensorType(), i, input[i]->getName(), input[i]->getAlias() ); 
  
  // actor
  for (uint8_t i=0; i<actors;i++) if (actor[i]) ioConfig.sendIO( actor[i]->getIOType(), i, actor[i]->getName(), actor[i]->getAlias() ); 
  
  // LED
  for (uint8_t i=0; i<MAXLEDS;i++) if (led[i]) ioConfig.sendIO( input[i]->getIOType(), i, led[i]->getName(), led[i]->getAlias() ); 

  // servo
  for (uint8_t i=0; i<SERVOS;i++) if (servo[i]) ioConfig.sendIO( servo[i]->getIOType(), i, servo[i]->getName(), servo[i]->getAlias() ); 

  // gyro
  if (gyro) ioConfig.sendIO( gyro->getIOType(), gyro->getName(), gyro->getAlias() ); 

}

bool SwOSCtrl::hasGyro( void ) {
  // test if HW has a gyro

  // already initialized or HW with integrated gyro
  if ( ( gyro ) || 
       ( CPU == FTSWARMRS_2V0 ) ||
       ( CPU == FTSWARMRS_2V1 ) ||
       ( CPU == FTSWARMRC_1V140 )
     ) return true;

  // check on MPU6050
  return Wire.requestFrom( 0x68, 1 );

}

bool SwOSCtrl::hasExtPort( void ) {

  return HASEXTPORT[CPU];

}