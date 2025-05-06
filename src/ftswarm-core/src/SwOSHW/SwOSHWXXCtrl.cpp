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
#include "SwOSHW/SwOSHWHAL.h"

/***************************************************
 *
 *   SwOSSwarmXX
 *
 ***************************************************/

SwOSSwarmXX::SwOSSwarmXX( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig ) : SwOSCtrl (SN, macAddr, local, ctrlConfig ) {

}

/***************************************************
 *
 *   SwOSSwarmJST
 *
 ***************************************************/

SwOSSwarmJST::SwOSSwarmJST( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig ):SwOSSwarmXX( SN, macAddr, local, ctrlConfig ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

}

SwOSSwarmJST::SwOSSwarmJST( SwOSCom *com ):SwOSSwarmJST( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.ctrlConfig ) {
  
}

char* SwOSSwarmJST::myType() {
  return (char *) "ftSwarm";
}

FtSwarmController_t SwOSSwarmJST::getType() {
  return FTSWARM;
}


/***************************************************
 *
 *   SwOSSwarmControl
 *
 ***************************************************/

SwOSSwarmControl::SwOSSwarmControl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig ):SwOSSwarmXX( SN, macAddr, local, ctrlConfig ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  // define specific hardware
  for (uint8_t i=0; i<4; i++) { button[i]   = new SwOSButton("S", i, this); }
  for (uint8_t i=0; i<2; i++) { button[4+i] = new SwOSButton("F", i, this); }
  for (uint8_t i=0; i<2; i++) { button[6+i] = new SwOSButton("J", i, this); }
  for (uint8_t i=0; i<2; i++) { 
    joystick[i] = new SwOSJoystick("JOY", i, this, ctrlConfig.zero[i][0], ctrlConfig.zero[i][1]); 
  }
  hc165 = new SwOSHC165("HC165", this);
  oled  = new SwOSOLED("OLED", this );

}

SwOSSwarmControl::SwOSSwarmControl( SwOSCom *com ):SwOSSwarmControl( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.ctrlConfig ) {
  
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
  if      ( ( strcmp(device, "S") == 0 )    && (port < 4) )           { button[port]->setAlias(alias);   return true; }
  else if ( ( strcmp(device, "F") == 0 )    && (port < 2) )           { button[port+4]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "J") == 0 )    && (port < 2) )           { button[port+6]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "JOY") == 0 )  && (port < 2) )           { joystick[port]->setAlias(alias); return true; }

  else if ( ( strcmp(device, "OLED") == 0 ) && (port = SWOS_NOPORT) ) { oled->setAlias(alias);           return true; }

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

void SwOSSwarmControl::_sendAlias( SwOSCom *alias ) {

  /*
  SwOSSwarmXX::sendAlias( alias );

  // buttons
  for (uint8_t i=0; i<8;i++) alias->sendBuffered( button[i]->getName(), button[i]->getAlias() ); 

  // joystick
  for (uint8_t i=0; i<2;i++) alias->sendBuffered( joystick[i]->getName(), joystick[i]->getAlias() ); 

  // oled
  if (oled) alias->sendBuffered( oled->getName(), oled->getAlias() ); 
  */

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

SwOSSwarmCAM::SwOSSwarmCAM( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig ):SwOSSwarmXX( SN, macAddr, local, ctrlConfig ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  if (local) cam = new SwOSCAM( "CAM", this );

}

SwOSSwarmCAM::SwOSSwarmCAM( SwOSCom *com ):SwOSSwarmCAM( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.ctrlConfig ) {
 
}

SwOSSwarmCAM::~SwOSSwarmCAM() {

  if (cam) delete cam;
  
}

bool SwOSSwarmCAM::cmdAlias( char *device, uint8_t port, const char *alias) {

  if ( ( strcmp(device, "CAM") == 0 ) && (port = SWOS_NOPORT) ) { 
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

  if (cam) cam->setStreaming( onOff, false );
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
