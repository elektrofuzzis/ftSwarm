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

SwOSCtrl::SwOSCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda, FtSwarmExtMode_t extensionPort  ):SwOSObj() {

  // copy master data
  this->IAmKelda = IAmKelda;
  serialNumber = SN;
  this->macAddr.set( macAddr );
  _local = local;
  _CPU = CPU;
  _lastContact = millis();

  // set my name 
  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  // # of inputs & actors
  inputs = MAXIOS[ CPU ].inputs;
  actors = MAXIOS[ CPU ].actors;
  leds   = MAXIOS[ CPU ].leds;
  servos = MAXIOS[ CPU ].servos;

  // extensionPort is configured as additional outputs, add 2 actors
  if ( extensionPort == FTSWARM_EXT_OUTPUT ) actors += 2;

  // extensionPort is configured as additional servos, add 2 servos
  if ( extensionPort == FTSWARM_EXT_SERVO ) servos +=2;

  // define io pointer array dynamically
  input = (SwOSInput **) calloc( inputs, sizeof(SwOSInput*) );
  actor = (SwOSActor **) calloc( actors, sizeof(SwOSActor*) );
  servo = (SwOSServo **) calloc( servos, sizeof(SwOSServo*) );

  // define common hardware
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
      } else {
        input[i] = new SwOSDigitalInput("A", i, this );
      }
    }

  }

  for (uint8_t i=0; i<actors; i++) { 
    
    actor[i] = new SwOSActor("M", i, this );

  }

  for (uint8_t i=0; i<MAXLEDS; i++) { 
    led[i] = new SwOSPixel("LED", i, this);
  }

  for (uint8_t i=0; i<servos; i++) {

    servo[i] = NULL;

    if ( ( _CPU == FTSWARMRC_1V140 ) && ( GPIO_INPUT[_CPU][7+i].io != GPIO_NUM_NC ) ) { 

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

SwOSCtrl::~SwOSCtrl() {
  
  if ( _subscribedCtrlName ) delete _subscribedCtrlName;
  
  for ( uint8_t i=0; i<inputs; i++) { if ( input[i] ) delete( input[i] ); }
  for ( uint8_t i=0; i<actors; i++) { if ( actor[i] ) delete( actor[i] ); }
  for ( uint8_t i=0; i<MAXLEDS; i++) { if ( led[i] )   delete( led[i] ); }
  for ( uint8_t i=0; i<servos; i++ ) { if ( servo[i] ) delete servo[i]; }
  
}

void SwOSCtrl::lock( void ) {

   xSemaphoreTake( _xAccessLock, portMAX_DELAY );

}


void SwOSCtrl::unlock( void ) {

   xSemaphoreGive( _xAccessLock );

}


void SwOSCtrl::halt( void ) {


  for (uint8_t i=0; i<actors; i++) { 
    if ( actor[i] ) { actor[i]->setSpeed(0); actor[i]->apply(); }
  }

}

char *SwOSCtrl::subscribe( char *ctrlName  ) {
  
  _isSubscribed = true;

  if (_subscribedCtrlName) delete _subscribedCtrlName;

  // only if I don't know my external name, store it
  if (!_subscribedCtrlName) {
    _subscribedCtrlName = (char *)malloc( strlen(ctrlName)+1 );
    strcpy( _subscribedCtrlName, ctrlName );
  }

  return _subscribedCtrlName;

}

void SwOSCtrl::unsubscribe( bool cascade ) {

  _isSubscribed = false;

  if (!cascade) return;

  for ( uint8_t i=0; i<inputs; i++ ) { if ( input[i] ) input[i]->unsubscribe(); }
  for ( uint8_t i=0; i<actors; i++ ) { if ( actor[i] ) actor[i]->unsubscribe(); }
  for ( uint8_t i=0; i<MAXLEDS; i++) { if ( led[i] )   led[i]->unsubscribe(); }
  for ( uint8_t i=0; i<servos; i++ ) { if ( servo[i] ) servo[i]->unsubscribe(); }
  
}

void SwOSCtrl::factorySettings( void ) {

  setAlias("");
  for ( uint8_t i=0; i<inputs; i++) { if ( input[i] ) input[i]->setAlias( "" ); }
  for ( uint8_t i=0; i<actors; i++) { if ( actor[i] ) actor[i]->setAlias( "" ); }
  for ( uint8_t i=0; i<MAXLEDS; i++)   { if ( led[i] )   led[i]->setAlias( "" ); }
  for ( uint8_t i=0; i<servos; i++) { if ( servo[i] ) servo[i]->setAlias( "" ); }

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
  _lastContact = millis();

  for (uint8_t i=0; i<inputs; i++) { if ( input[i] ) input[i]->read();   }
  for (uint8_t i=0; i<actors; i++) { if ( actor[i] ) actor[i]->read();   }
  for (uint8_t i=0; i<servos; i++) { if ( servo[i] ) servo[i]->adjust(); }


}

bool SwOSCtrl::isInUse( void ) {

  for (uint8_t i=0; i<inputs; i++) { if ( ( input[i] ) && ( input[i]->isInUse() ) ) return true; }
  for (uint8_t i=0; i<actors; i++) { if ( ( actor[i] ) && ( actor[i]->isInUse() ) ) return true; }
  for (uint8_t i=0; i<servos; i++) { if ( ( servo[i] ) && ( servo[i]->isInUse() ) ) return true; }

  return false;

}


bool SwOSCtrl::isI2CSwarmCtrl( void ) {

  return ( _CPU == FTSWARMPWRDRIVE_1V141 ) || ( _CPU == FTSWARMDUINO_1V141 );

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
  return version(_CPU);
}

char *SwOSCtrl::getHostname( void ) {

  if ( (_alias) && (_alias[0]!='\0') ) {
    return _alias ;
  } else {
    return _name;
  }

}

bool SwOSCtrl::changeIOType( uint8_t port, FtSwarmIOType_t oldIOType, FtSwarmIOType_t newIOType ) {

  // not in standard, maybe overloaded
  return false;

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
  
  for (uint8_t i=0; i<inputs; i++)      { if ( input[i] ) input[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<actors; i++)      { if ( actor[i] ) actor[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<nvs.RGBLeds; i++) { if ( led[i]   ) led[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<servos; i++)      { if ( servo[i] ) servo[i]->jsonize( json, id ); } 

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
  
  if (_local) {
    setState( IDENTIFY );
  
  } else {
    SwOSCom identify( macAddr, serialNumber, CMD_IDENTIFY );
    identify.send();

  }

}

unsigned long SwOSCtrl::networkAge( void ) { 

  unsigned long age = millis() - _lastContact;

  return age;

}

bool SwOSCtrl::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  _lastContact = millis();

  nvs_handle_t my_handle;
    
  switch (com->data.cmd) {

    case CMD_SAVEALIAS2NVS:         // save in local nvs
                                    ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );
                                    saveAliasToNVS( my_handle );
                                    ESP_ERROR_CHECK( nvs_commit( my_handle ) );
                                    return true;

    case CMD_STATE:                 return recvState( com );

    case CMD_SETLED:                if (led[com->data.ledCmd.index]) {
                                        led[com->data.ledCmd.index]->setBrightness( com->data.ledCmd.brightness );
                                        led[com->data.ledCmd.index]->setColor( com->data.ledCmd.color );
                                      }
                                      return true;

    case CMD_SETSENSORTYPE:           if ( input[com->data.sensorCmd.index]->getIOType() == FTSWARM_DIGITALINPUT) { 
                                        ((SwOSDigitalInput *)input[com->data.sensorCmd.index])->setSensorType( com->data.sensorCmd.sensorType, com->data.sensorCmd.normallyOpen );
                                      } else {
                                        ((SwOSAnalogInput *)input[com->data.sensorCmd.index])->setSensorType( com->data.sensorCmd.sensorType );
                                      }
                                      return true;

    case CMD_SETACTORSPEED:           actor[com->data.actorSpeedCmd.index]->setMotionType( com->data.actorSpeedCmd.motionType );
                                      actor[com->data.actorSpeedCmd.index]->setAcceleration( com->data.actorSpeedCmd.rampUpT, com->data.actorSpeedCmd.rampUpY );
                                      actor[com->data.actorSpeedCmd.index]->setSpeed( com->data.actorSpeedCmd.speed );
                                      actor[com->data.actorSpeedCmd.index]->apply();
                                      return true;

    case CMD_RESETCOUNTER:            if ( ( input[com->data.counterCmd.index] ) && ( input[com->data.counterCmd.index]->getIOType() == FTSWARM_COUNTERINPUT ) )
                                        static_cast<SwOSCounter *>(input[com->data.counterCmd.index])->resetCounter();
                                      return true;

    case CMD_SETSTEPPERDISTANCE:      actor[com->data.actorStepperCmd.index]->setDistance( com->data.actorStepperCmd.paraml, com->data.actorStepperCmd.paramb, true );
                                      return true;

    case CMD_SETSTEPPERPOSITION:      actor[com->data.actorStepperCmd.index]->setPosition( com->data.actorStepperCmd.paraml, true );
                                      return true;

    case CMD_STEPPERHOMING:           actor[com->data.actorStepperCmd.index]->homing( com->data.actorStepperCmd.paraml );
                                      return true;

    case CMD_SETSTEPPERHOMINGOFFSET:  actor[com->data.actorStepperCmd.index]->setHomingOffset( com->data.actorStepperCmd.paraml );
                                      return true;

    case CMD_STEPPERSTARTSTOP:        actor[com->data.actorStepperCmd.index]->startStop( com->data.actorStepperCmd.paramb );
                                      return true;

    case CMD_SETACTORTYPE:            actor[com->data.actorTypeCmd.index]->setActorType( com->data.actorTypeCmd.actorType, com->data.actorTypeCmd.highResolution, true );
                                      actor[com->data.actorTypeCmd.index]->apply();
                                      return true;

    case CMD_IDENTIFY:                identify();
                                      return true;

    case CMD_USEREVENT:               if ( com->data.userEventCmd.trigger ) {

                                        // send trigger event to local procedure
                                        if ( xQueueSend( myOSNetwork.userEvent, com, ESPNOW_MAXDELAY ) != pdTRUE ) {
                                          ESP_LOGE( LOGFTSWARM, "Can't send data to user event." );
                                        }

                                      } else {
        
                                        // got some user event data
                                        if (_isSubscribed) {
                                          printf("S: %s", _subscribedCtrlName );
                                          for ( uint8_t i=0; i<com->data.userEventCmd.size; i++ ) printf(" %02X", com->data.userEventCmd.payload[i]);
                                          printf("\n");
                                        }
                                      }
                                      return true;

    case CMD_ALIAS:                   // get all entries in datagramm
                                      for (uint8_t i=0; i<MAXALIAS; i++) {
                                        if (com->data.aliasCmd.alias[i].name[0] != '\0') {
                                          // entry isn't empty
                                          if ( strcmp( com->data.aliasCmd.alias[i].name, "HOSTNAME" ) == 0 ) {
                                            // hostname
                                            setAlias( com->data.aliasCmd.alias[i].alias );
                                          } else {
                                          // an IO port?
                                          cmdAlias( com->data.aliasCmd.alias[i].name, com->data.aliasCmd.alias[i].alias );
                                          }
                                        }
                                        // receving alias cmds: set controlle "online"
                                        setComState( COMSTATE_ONLINE );
                                      }
                                      return true;

    case CMD_CHANGEIOTYPE:            // change IO Type
                                      changeIOType( com->data.changeIOTypeCmd.index, com->data.changeIOTypeCmd.oldIOType, com->data.changeIOTypeCmd.newIOType );
                                      return true;

    case CMD_SETSERVO:                // set servo position + offset
                                      if (servo[com->data.servoCmd.index]) {
                                        servo[com->data.servoCmd.index]->setOffset( com->data.servoCmd.offset, true );
                                        servo[com->data.servoCmd.index]->setPosition( com->data.servoCmd.position, true );
                                      }

  }

  return false;

}

SwOSCom *SwOSCtrl::state2Com( MacAddr destination ) {

  SwOSCom *com = new SwOSCom( destination, serialNumber, CMD_STATE );

  int16_t FB, LR;

  for (uint8_t i=0; i<inputs; i++ ) { com->data.stateCmd.inputValue[i] = input[i]->getValueI32(); };

  return com;

}

bool SwOSCtrl::recvState( SwOSCom *com ) {
  
  for (uint8_t i=0; i<inputs; i++ ) { input[i]->setValue( com->data.stateCmd.inputValue[i] ); };
  return true;
 
} 

void SwOSCtrl::registerMe( SwOSCom *com ){

  if (!com) return;

  // meta data
  com->data.registerCmd.ctrlType   = getType();
  com->data.registerCmd.versionCPU = getCPU();
  com->data.registerCmd.IAmKelda   = IAmKelda;
  strcpy( com->data.registerCmd.swarmName, nvs.swarmName );
  com->data.registerCmd.swarmPIN   = nvs.swarmPIN; 

  // extention port
  com->data.registerCmd.extensionPort = nvs.extensionPort;

  if ( getType() != FTSWARMCONTROL ) com->data.registerCmd.leds = leds;

}

void SwOSCtrl::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSObj::saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<inputs; i++ )  if (input[i]) input[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<actors; i++ )  if (actor[i]) actor[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<MAXLEDS; i++ ) if (led[i])   led[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<servos; i++ )  if (servo[i]) servo[i]->saveAliasToNVS( my_handle );
}

void SwOSCtrl::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSObj::loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<inputs; i++ )  if (input[i]) input[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<actors; i++ )  if (actor[i]) actor[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<MAXLEDS; i++ ) if (led[i])   led[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<servos; i++ )  if (servo[i]) servo[i]->loadAliasFromNVS( my_handle );
}

void SwOSCtrl::_sendAlias( SwOSCom *alias ) {

    // hostname
  alias->sendBuffered( (char *)"HOSTNAME", getAlias() ); 

  // input
  for (uint8_t i=0; i<inputs;i++) if (input[i]) alias->sendBuffered( input[i]->getName(), input[i]->getAlias() ); 
  
  // actor
  for (uint8_t i=0; i<actors;i++) if (actor[i]) alias->sendBuffered( actor[i]->getName(), actor[i]->getAlias() ); 
  
  // LED
  for (uint8_t i=0; i<MAXLEDS;i++) 
    if (led[i]) alias->sendBuffered( led[i]->getName(), led[i]->getAlias() ); 

  // servo
  for (uint8_t i=0; i<SERVOS;i++) if (servo[i]) alias->sendBuffered( servo[i]->getName(), servo[i]->getAlias() ); 

}

void SwOSCtrl::sendAlias( MacAddr destination ) {

  SwOSCom alias( destination, serialNumber, CMD_ALIAS );
  _sendAlias( &alias );
  alias.flushBuffer( );
  
}

bool SwOSCtrl::hasExtPort( void ) {

  return HASEXTPORT[_CPU];

}