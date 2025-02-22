/*
 * SwOSHWXX.cpp
 *
 * Standard controller (ftSwarmJST, ftSwarmControl, ftSwarmCAM) hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include "SwOSHW/SwOSHWXXCtrl.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"

/***************************************************
 *
 *   SwOSSwarmXX
 *
 ***************************************************/

 SwOSSwarmXX::SwOSSwarmXX( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda, FtSwarmExtMode_t extensionPort, bool gyroOn ) : SwOSCtrl (SN, macAddr, local, CPU, IAmKelda, extensionPort ) {

  gyro = NULL;
  I2C  = NULL;

  // Start I2C, if extention port is configured as I2C. 
  // ToDo I2CSlave 
  if ( ( local ) && 
       ( ( nvs.extensionPort == FTSWARM_EXT_I2C_MASTER ) ||
         ( nvs.extensionPort == FTSWARM_EXT_LIDAR ) 
       )
      ) {
    
    switch (CPU) {

      case FTSWARMJST_1V0:      Wire.begin( 13, 12 ); break;

      case FTSWARMCONTROL_1V3: 
      case FTSWARMJST_1V15:     Wire.begin( 21, 22 ); break;  

      case FTSWARMXL_1V00:      Wire.begin( 33, 21 ); break;

      case FTSWARMRS_2V0: 
      case FTSWARMRS_2V1:       Wire.begin( 8, 9 );   break;
  
      default:                  break; // CAM

    }

  }

  // use parameter to handle remote devices correctly
  if ( extensionPort == FTSWARM_EXT_I2C_SLAVE ) { I2C = new SwOSI2C ( "I2C", this, nvs.I2CAddr ); };

  // initialize gyro if available
  if ( gyroOn  ) { 
    if ( ( _CPU == FTSWARMRS_2V0 ) || ( _CPU == FTSWARMRS_2V1 ) ) gyro = new SwOSGyroLSM( "GYRO", this );
    else                                                          gyro = new SwOSGyroMPU( "GYRO", this );
  }
  
}

SwOSSwarmXX::~SwOSSwarmXX() {

  if (gyro) delete( gyro );
  if (I2C)  delete( I2C );

}

void SwOSSwarmXX::read( void ) {

  SwOSCtrl::read();
  if (I2C) I2C->read();
  if (gyro) gyro->read();

}

void SwOSSwarmXX::unsubscribe( void ) {

  if (gyro) gyro->unsubscribe();
  if (I2C)  I2C->unsubscribe();

}

bool SwOSSwarmXX::hasGyro( void ) {
  // test if HW has a gyro

  // already initialized or HW with integrated gyro
  if ( ( gyro ) || 
       ( _CPU == FTSWARMRS_2V0 ) ||
       ( _CPU == FTSWARMRS_2V1 ) 
     ) return true;

  // check on MPU6050
  return Wire.requestFrom( 0x68, 1 );

}

void SwOSSwarmXX::factorySettings( void ) {

  SwOSCtrl::factorySettings();
  if (gyro) gyro->setAlias("");
  if (I2C)  I2C->setAlias("");
  
}

SwOSIO *SwOSSwarmXX::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  SwOSIO *result = SwOSCtrl::getIO( ioType, port );
  if (result) return result;

  switch (ioType) {
    case FTSWARM_GYRO: return gyro;
    case FTSWARM_I2C:  return I2C;
  }

  return NULL;

}


SwOSIO *SwOSSwarmXX::getIO( const char *name ) {

  SwOSIO *result = SwOSCtrl::getIO( name );
  if (result) return result;

  if ( (gyro) && (gyro->equals(name) ) ) return gyro;
  if ( (I2C)  && (I2C->equals(name) ) )  return I2C;

  return NULL;

}

bool SwOSSwarmXX::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  if ( ( com->data.cmd == CMD_I2CREGISTER ) && (I2C) ) {
      I2C->setRegister( com->data.I2CRegisterCmd.reg, com->data.I2CRegisterCmd.value );
      return true;
  } else {
      return SwOSCtrl::OnDataRecv(com);
  }

  return true;

}

bool SwOSSwarmXX::isInUse( void ) {

  if (SwOSCtrl::isInUse() ) return true;

  if ( ( gyro ) && ( gyro->isInUse() ) ) return true;
  if ( ( I2C )  && ( I2C->isInUse() ) )  return true;

  return false;

}


void SwOSSwarmXX::_sendAlias( SwOSCom *alias ) {

  SwOSCtrl::_sendAlias( alias );

  // gyro
  if (gyro) alias->sendBuffered( gyro->getName(), gyro->getAlias() ); 

}

bool isInputType( FtSwarmIOType_t ioType ) {

  return ( ioType == FTSWARM_DIGITALINPUT ) ||
         ( ioType == FTSWARM_ANALOGINPUT ) ||
         ( ioType == FTSWARM_ROTARYINPUT ) ||
         ( ioType == FTSWARM_COUNTERINPUT ) ||
         ( ioType == FTSWARM_FREQUENCYINPUT );
}

bool SwOSSwarmXX::changeIOType( uint8_t port, FtSwarmIOType_t oldIOType, FtSwarmIOType_t newIOType ) {

  // check on compatible types
  if (!isInputType( oldIOType) ) return false;
  if (!isInputType( newIOType) ) return false;

  // register the new one
  SwOSInput *io;
  switch ( newIOType ) {

    case FTSWARM_DIGITALINPUT:    io = new SwOSDigitalInput("A", port, this ); 
                                  break;

    case FTSWARM_ANALOGINPUT:     io = new SwOSAnalogInput("A", port, this );
                                  break;

    case FTSWARM_COUNTERINPUT:    io = new SwOSCounter("A", port, 255, this ); 
                                  break;

    case FTSWARM_ROTARYENCODER:   io = new SwOSCounter("A", port, port+1, this ); 
                                  if ( port+1 < inputs ) { 
                                    // cleanup next input, it's used now
                                    SwOSInput *old = input[port+1];
                                    input[port+1] = NULL;
                                    if ( old ) delete old;
                                  }
                                  break;

    case FTSWARM_FREQUENCYINPUT:  io = new SwOSFrequencymeter("A", port, 255, this ); 
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

SwOSCom *SwOSSwarmXX::state2Com( MacAddr destination ) {

  SwOSCom *com = SwOSCtrl::state2Com( destination );

  // copy I2C registers
  if (I2C) memcpy( com->data.stateCmd.i2cValue, I2C->myRegister, MAXI2CREGISTERS );
  // if (gyro) memcpy( com->data.stateCmd.gyro, gyro->)

  return com;

}

bool SwOSSwarmXX::recvState( SwOSCom *com ) {

  if (!SwOSCtrl::recvState(com) ) return false;

  if (I2C) memcpy( I2C->myRegister,  com->data.stateCmd.i2cValue, MAXI2CREGISTERS );
  return true;
 
} 

/***************************************************
 *
 *   SwOSSwarmJST
 *
 ***************************************************/

SwOSSwarmJST::SwOSSwarmJST( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda, FtSwarmExtMode_t extensionPort, bool gyro ):SwOSSwarmXX( SN, macAddr, local, CPU, IAmKelda, extensionPort, gyro ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  // define specific hardware
  switch ( CPU ) {
    case FTSWARMRS_2V1:     servos = ( extensionPort == FTSWARM_EXT_SERVO ) ? 4:2;
                            break;

    case FTSWARMRS_2V0:
    case FTSWARMJST_1V0:
    case FTSWARMJST_1V15:   servos = ( extensionPort == FTSWARM_EXT_SERVO ) ? 3:1;
                            break;

    default:                servos = ( extensionPort == FTSWARM_EXT_SERVO ) ? 2:0;
                            break;
  }

  for (uint8_t i=0; i<MAXSERVOS; i++) {
    if ( i>= servos ) {
      servo[i] = NULL;
    } else {
      servo[i] = new SwOSServo("SERVO", i, this);
    }
  }

  if ( extensionPort == FTSWARM_EXT_LIDAR ) {
    input[inputs++] = new SwOSLidarInput( "LIDAR", 99, this );
  }

}

SwOSSwarmJST::SwOSSwarmJST( SwOSCom *com ):SwOSSwarmJST( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmKelda, com->data.registerCmd.extensionPort, FTSWARM_GYRO_OFF ) {
  
}

SwOSSwarmJST::~SwOSSwarmJST() {
  
  for ( uint8_t i=0; i<MAXSERVOS; i++ ) { if ( servo[i] ) delete servo[i]; }

}

bool SwOSSwarmJST::isInUse( void ) {

  if ( SwOSSwarmXX::isInUse() ) return true;
  for ( uint8_t i=0; i<MAXSERVOS; i++ ) { if ( ( servo[i] ) && ( servo[i]->isInUse() ) ) return true; }

  return false;

}

void SwOSSwarmJST::unsubscribe( void ) {
  
  for ( uint8_t i=0; i<MAXSERVOS; i++ ) { if ( servo[i] ) servo[i]->unsubscribe(); }

}

void SwOSSwarmJST::factorySettings( void ) {

  SwOSSwarmXX::factorySettings();
  for (uint8_t i=0; i<MAXSERVOS; i++) { if ( servo[i] ) servo[i]->setAlias( "" ); }
  
}

bool SwOSSwarmJST::cmdAlias( char *device, uint8_t port, const char *alias) {

  // test on my specific hardware
  if      ( ( strcmp(device, "SERVO") == 0 ) && (port < MAXSERVOS ) && (servo[port])) { servo[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "GYRO")  == 0 ) && (port = 255) && (gyro) )              { gyro->setAlias(alias);        return true; }
  else if ( ( strcmp(device, "I2C")   == 0 ) && (port = 255) && (I2C) )               { I2C->setAlias(alias);         return true; }
  else return false;

}

SwOSIO *SwOSSwarmJST::getIO( const char *name) {

  // check on base class hardware
  SwOSIO *IO = SwOSSwarmXX::getIO(name);
  if ( IO != NULL ) { return IO; }

  // check on specific hardware
  for (uint8_t i=0;i<MAXSERVOS;i++) { if ( (servo[i]) && ( servo[i]->equals(name) ) ) { return servo[i]; } }

  if ( (I2C) && ( I2C->equals(name) ) ) { return I2C; }

  return NULL;

}

SwOSIO *SwOSSwarmJST::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  // check on base class hardware
  SwOSIO *IO = SwOSSwarmXX::getIO(ioType, port);
  if ( IO != NULL ) { return IO; }

  if ( ioType == FTSWARM_SERVO) return ( (port<MAXSERVOS)?servo[port]:NULL);
  if ( ioType == FTSWARM_I2C)   return ( I2C );
  
  return NULL;

}

char* SwOSSwarmJST::myType() {
  return (char *) "ftSwarm";
}

FtSwarmController_t SwOSSwarmJST::getType() {
  return FTSWARM;
}

void SwOSSwarmJST::jsonizeIO( JSONize *json, uint8_t id) {

  SwOSSwarmXX::jsonizeIO(json, id);

  for (uint8_t i=0; i<servos; i++) { if ( servo[i] ) servo[i]->jsonize( json, id ); } 
  if (gyro)  { gyro->jsonize(json, id); }

}

bool SwOSSwarmJST::apiServoOffset( char *id, int offset ) {
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

bool SwOSSwarmJST::apiServoPosition( char *id, int position ) {
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

bool SwOSSwarmJST::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  // check if SwOSCrtl knows the cmd
  if ( SwOSSwarmXX::OnDataRecv( com ) ) return true;

  switch ( com->data.cmd ) {
    case CMD_SETSERVO:
      if (servo[com->data.servoCmd.index]) {
        servo[com->data.servoCmd.index]->setOffset( com->data.servoCmd.offset, true );
        servo[com->data.servoCmd.index]->setPosition( com->data.servoCmd.position, true );
      }
      return true;
  }

  return false;

}

void SwOSSwarmJST::_sendAlias( SwOSCom *alias ) {

  SwOSSwarmXX::_sendAlias( alias );

  // servo
  for (uint8_t i=0; i<MAXSERVOS;i++) if (servo[i]) alias->sendBuffered( servo[i]->getName(), servo[i]->getAlias() ); 

}

void SwOSSwarmJST::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSSwarmXX::saveAliasToNVS( my_handle );

  if (gyro) gyro->saveAliasToNVS( my_handle );
  if (I2C)  I2C->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<MAXSERVOS; i++ ) if (servo[i]) servo[i]->saveAliasToNVS( my_handle );
  
}

void SwOSSwarmJST::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSSwarmXX::loadAliasFromNVS( my_handle );

  if (gyro) gyro->loadAliasFromNVS( my_handle );
  if (I2C)  I2C->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<MAXSERVOS; i++ ) if (servo[i]) servo[i]->loadAliasFromNVS( my_handle );
  
}

/***************************************************
 *
 *   SwOSSwarmControl
 *
 ***************************************************/

SwOSSwarmControl::SwOSSwarmControl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda, int16_t zero[2][2], uint8_t displayType, FtSwarmExtMode_t extentionPort, bool gyroOn ):SwOSSwarmXX( SN, macAddr, local, CPU,  IAmKelda, extentionPort, gyroOn ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  // define specific hardware
  for (uint8_t i=0; i<4; i++) { button[i]   = new SwOSButton("S", i, this); }
  for (uint8_t i=0; i<2; i++) { button[4+i] = new SwOSButton("F", i, this); }
  for (uint8_t i=0; i<2; i++) { button[6+i] = new SwOSButton("J", i, this); }
  for (uint8_t i=0; i<2; i++) { 
    if (zero) joystick[i] = new SwOSJoystick("JOY", i, this, zero[i][0], zero[i][1]); 
    else      joystick[i] = new SwOSJoystick("JOY", i, this, 0, 0 ); 
  }
  hc165 = new SwOSHC165("HC165", this);
  oled  = new SwOSOLED("OLED", this, displayType);

}

SwOSSwarmControl::SwOSSwarmControl( SwOSCom *com ):SwOSSwarmControl( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmKelda, NULL, 1, FTSWARM_EXT_OFF, FTSWARM_GYRO_OFF ) {
  
}

SwOSSwarmControl::~SwOSSwarmControl() {
  
  if ( oled  ) delete oled;

}

void SwOSSwarmControl::unsubscribe( void ) {
  
  for ( uint8_t i=0; i<8; i++ ) { if ( button[i] ) button[i]->unsubscribe(); }
  for ( uint8_t i=0; i<2; i++ ) { if ( joystick[i] ) joystick[i]->unsubscribe(); }
  if ( oled  ) oled->unsubscribe();

}

bool SwOSSwarmControl::isInUse( void ) {

  if ( SwOSSwarmXX::isInUse() ) return true;
  
  for ( uint8_t i=0; i<8; i++ ) { if ( ( button[i] )   && ( button[i]->isInUse() ) ) return true; }
  for ( uint8_t i=0; i<2; i++ ) { if ( ( joystick[i] ) && ( joystick[i]->isInUse() ) ) return true; }

  if ( ( oled ) && ( oled->isInUse() ) ) return true;

  return false;

}

void SwOSSwarmControl::factorySettings( void ) {

  SwOSSwarmXX::factorySettings();
  for (uint8_t i=0; i<8; i++) { if ( button[i] ) button[i]->setAlias( "" ); }
  if (hc165) hc165->setAlias("");

  if (oled)  oled->setAlias("");

}

bool SwOSSwarmControl::cmdAlias( char *device, uint8_t port, const char *alias) {

  // just test on specific hardware
  if      ( ( strcmp(device, "S") == 0 )    && (port < 4) )   { button[port]->setAlias(alias);   return true; }
  else if ( ( strcmp(device, "F") == 0 )    && (port < 2) )   { button[port+4]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "J") == 0 )    && (port < 2) )   { button[port+6]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "JOY") == 0 )  && (port < 2) )   { joystick[port]->setAlias(alias); return true; }

  else if ( ( strcmp(device, "OLED") == 0 ) && (port = 255) ) { oled->setAlias(alias);           return true; }

  else return false;

}

SwOSIO *SwOSSwarmControl::getIO( const char *name ) {

  // check on base base class hardware
  SwOSIO *IO = SwOSSwarmXX::getIO(name);
  if ( IO != NULL ) { return IO; }

  // check on specific hardware
  for (uint8_t i=0;i<8;i++) { if (button[i]->equals(name) )   { return button[i]; } }
  for (uint8_t i=0;i<2;i++) { if (joystick[i]->equals(name) ) { return joystick[i]; } }

  if ( (oled) && ( oled->equals(name) ) ) { return oled; }

  return NULL;

}

SwOSIO *SwOSSwarmControl::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  // check on base base class hardware
  SwOSIO *IO = SwOSSwarmXX::getIO(ioType, port);
  if ( IO != NULL ) { return IO; }
  
  switch (ioType) {
    case FTSWARM_JOYSTICK: return ( (port<2)?joystick[port]:NULL);
    case FTSWARM_BUTTON:   return ( (port<8)?button[port]:NULL);
    case FTSWARM_OLED:     return oled;
  }

  return NULL;
  
}

char* SwOSSwarmControl::myType() {
  return (char *) "ftSwarmControl";
}

FtSwarmController_t SwOSSwarmControl::getType() {
  return FTSWARMCONTROL;
}

void SwOSSwarmControl::jsonizeIO( JSONize *json, uint8_t id) {

  SwOSSwarmXX::jsonizeIO(json, id);

  for (uint8_t i=0; i<6; i++) { button[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<2; i++) { joystick[i]->jsonize( json, id ); } 
  if (gyro)  { gyro->jsonize(json, id); }

}


void SwOSSwarmControl::read() {

  SwOSSwarmXX::read();

  // get joystick readings
  for (uint8_t i=0; i<2; i++) {
    joystick[i]->read();
  }

  // get buttons via hc165
  hc165->read();

  // transfer result to buttons
  uint8_t v;
  v = hc165->getValue( );
  for (uint8_t i=0; i<8; i++) { button[i]->setState( v & (1<<i), _firstRead ); }

  // _firstRead allows to suppress a toggle event on buttons during startup
  _firstRead = false;

}

void SwOSSwarmControl::setState( SwOSState_t state, uint8_t members, char *SSID ) {
  // visualizes controller's state like booting, error,...


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

SwOSCom *SwOSSwarmControl::state2Com( MacAddr destination ) {

  SwOSCom *com = SwOSSwarmXX::state2Com( destination );

  int16_t FB, LR;
  for (uint8_t i=0; i<2; i++ ) { 
    joystick[i]->getValue( &FB, &LR );
    com->data.stateCmd.FB[i] = FB;
    com->data.stateCmd.LR[i] = LR;
  };
  
  com->data.stateCmd.hc165 = hc165->getValue();

  return com;
}

bool SwOSSwarmControl::recvState( SwOSCom *com ) {

  SwOSSwarmXX::recvState( com );
  
  for (uint8_t i=0; i<2; i++ ) { joystick[i]->setValue( com->data.stateCmd.FB[i], com->data.stateCmd.LR[i] ); };
      
  // set HC165 value & buttons
  uint8_t hc = com->data.stateCmd.hc165;
  hc165->setValue( hc );
  for (uint8_t i=0; i<8; i++) { button[i]->setState( hc & (1<<i) ); }

  return true;
 
} 

bool SwOSSwarmControl::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  // check if SwOSCrtl knows the cmd
  if ( SwOSSwarmXX::OnDataRecv( com ) ) return true;

  return false;

}

void SwOSSwarmControl::_sendAlias( SwOSCom *alias ) {

  SwOSSwarmXX::_sendAlias( alias );

  // buttons
  for (uint8_t i=0; i<8;i++) alias->sendBuffered( button[i]->getName(), button[i]->getAlias() ); 

  // joystick
  for (uint8_t i=0; i<2;i++) alias->sendBuffered( joystick[i]->getName(), joystick[i]->getAlias() ); 

  // oled
  if (oled) alias->sendBuffered( oled->getName(), oled->getAlias() ); 

}


void SwOSSwarmControl::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSSwarmXX::saveAliasToNVS( my_handle );

  if (oled) oled->saveAliasToNVS( my_handle );

  for (uint8_t i=0; i<8; i++ ) button[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<2; i++ ) joystick[i]->saveAliasToNVS( my_handle );
  
}


void SwOSSwarmControl::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSSwarmXX::loadAliasFromNVS( my_handle );

  if (oled) oled->loadAliasFromNVS( my_handle );
  
  for (uint8_t i=0; i<8; i++ ) button[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<2; i++ ) joystick[i]->loadAliasFromNVS( my_handle );
  
}

boolean SwOSSwarmControl::getRemoteControl( void ) {
  return _remoteControl;
}

void SwOSSwarmControl::setRemoteControl( boolean remoteControl ) {
  _remoteControl = remoteControl;
}


/***************************************************
 *
 *   SwOSSwarmCAM
 *
 ***************************************************/

SwOSSwarmCAM::SwOSSwarmCAM( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda ):SwOSSwarmXX( SN, macAddr, local, CPU,  IAmKelda, FTSWARM_EXT_OFF, FTSWARM_GYRO_OFF ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  if (local) cam = new SwOSCAM( "CAM", this );

}

SwOSSwarmCAM::SwOSSwarmCAM( SwOSCom *com ):SwOSSwarmCAM( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmKelda ) {
 
}

SwOSSwarmCAM::~SwOSSwarmCAM() {

  if (cam) delete cam;
  
}

bool SwOSSwarmCAM::cmdAlias( char *device, uint8_t port, const char *alias) {

  if ( ( strcmp(device, "CAM") == 0 ) && (port = 255) ) { 
    cam->setAlias(alias); 
    return true; 
  }

  return false;

}

SwOSIO *SwOSSwarmCAM::getIO( const char *name ) {

  // check on base base class hardware
  SwOSIO *IO = SwOSCtrl::getIO(name);
  if ( IO != NULL ) { return IO; }

  if ( (cam) && ( cam->equals(name) ) ) { return cam; }

  return NULL;

}

SwOSIO *SwOSSwarmCAM::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  // check on base base class hardware
  SwOSIO *IO = SwOSCtrl::getIO(ioType, port);
  if ( IO != NULL ) { return IO; }

  if (ioType == FTSWARM_CAM ) return cam;

  return NULL;
  
}

char* SwOSSwarmCAM::myType() {
  return (char *) "ftSwarmCAM";
}

FtSwarmController_t SwOSSwarmCAM::getType() {
  return FTSWARMCAM;
}

void SwOSSwarmCAM::jsonizeIO( JSONize *json, uint8_t id) {

  SwOSCtrl::jsonizeIO(json, id);

  if (cam) cam->jsonize( json, id ); 

}


void SwOSSwarmCAM::setState( SwOSState_t state, uint8_t members, char *SSID ) {
  // visualizes controller's state like booting, error,...
   
}


bool SwOSSwarmCAM::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  // check if SwOSCrtl knows the cmd
  if ( SwOSCtrl::OnDataRecv( com ) ) return true;

  // ToDO: add CAM Commands

  return false;

}

void SwOSSwarmCAM::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSCtrl::saveAliasToNVS( my_handle );
  if (cam) cam->saveAliasToNVS( my_handle );
  
}


void SwOSSwarmCAM::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSCtrl::loadAliasFromNVS( my_handle );
  if (cam) cam->loadAliasFromNVS( my_handle );
  
}


bool SwOSSwarmCAM::apiCAMStreaming( char *id, bool onOff ) {
  // set CAM framzesize / resolution

  if (cam) cam->streaming( onOff, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMFramesize( char *id, int framesize ) {
  // set CAM framzesize / resolution

  if (cam) cam->setFramesize( (framesize_t) framesize, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMQuality( char *id, int quality ) { 
  // set CAM quality

  if (cam) cam->setQuality( quality, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMBrightness( char *id, int brightness ) { 
  // set CAM brightness

  if (cam) cam->setBrightness( brightness, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMContrast( char *id, int contrast ) { 
  // set CAM contrast

  if (cam) cam->setContrast( contrast, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMSaturation( char *id, int saturation ) { 
  // set CAM saturation

  if (cam) cam->setSaturation( saturation, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMSpecialEffect( char *id, int specialEffect ) { 
  // set CAM special effect

  if (cam) cam->setSpecialEffect( specialEffect, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMWbMode( char *id, int wbMode ) { 
  // set CAM wbMode

  if (cam) cam->setWbMode( wbMode, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMVFlip( char *id, bool vFlip ) { 
  // set CAM V-Flip 

  if (cam) cam->setVFlip( vFlip, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMHMirror( char *id, bool hMirror ) { 
  // set CAM H-Mirror

  if (cam) cam->setHMirror( hMirror, false );
  return true;

}
