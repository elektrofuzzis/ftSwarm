/*
 * SwOSHWI2CCtrl.cpp
 *
 * I2C based controller (ftSwarmDuino & ftSwarmPwrDrive) implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include "SwOSHW/SwOSHWI2CCtrl.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWActor.h"
 
 /***************************************************
 *
 *   SwOSSwarmI2CCtrl
 *
 ***************************************************/

 SwOSSwarmI2CCtrl::SwOSSwarmI2CCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda ): SwOSCtrl (SN, macAddr, local, CPU,  IAmKelda, FTSWARM_EXT_OFF ) {

  Wire.begin( 5, 4);

}

SwOSSwarmI2CCtrl::~SwOSSwarmI2CCtrl() {

}

/***************************************************
 *
 *   SwOSSwarmPwrDrive - ftPwrDrive implementation as I2C Slave
 *
 ***************************************************/

SwOSSwarmPwrDrive::SwOSSwarmPwrDrive( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda ): SwOSSwarmI2CCtrl (SN, macAddr, local, CPU,  IAmKelda ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  if (local) pwrDrive = new ftPwrDrive( 32, 5, 4 );
  
}

SwOSSwarmPwrDrive::SwOSSwarmPwrDrive( SwOSCom *com ):SwOSSwarmPwrDrive( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmKelda ) {
  
}

SwOSSwarmPwrDrive::~SwOSSwarmPwrDrive() {

}

char* SwOSSwarmPwrDrive::myType() {
  return (char *) "FTSWARMPWRDRIVE";
}

FtSwarmController_t SwOSSwarmPwrDrive::getType() {
  return FTSWARMPWRDRIVE;
}

SwOSCom *SwOSSwarmPwrDrive::state2Com( MacAddr destination ) {

  SwOSCom *com = new SwOSCom( destination, serialNumber, CMD_STATE );

  for (uint8_t i=0; i<inputs; i++ ) { 
    com->data.stepperStateCmd.inputValue[i] = input[i]->getValueI32();
  };
  
  for (uint8_t i=0; i<actors; i++ ) {
    com->data.stepperStateCmd.isHoming[i] = actor[i]->isHoming();
    com->data.stepperStateCmd.isRunning[i] = actor[i]->isRunning(); 
    com->data.stepperStateCmd.position[i] = actor[i]->getPosition();
    com->data.stepperStateCmd.distance[i] = actor[i]->getDistance();
  }

  return com;

}

bool SwOSSwarmPwrDrive::recvState( SwOSCom *com ) {
  
  for (uint8_t i=0; i<inputs; i++ ) { input[i]->setValue( com->data.stepperStateCmd.inputValue[i] ); };
  for (uint8_t i=0; i<actors; i++ ) {
    actor[i]->setPosition( com->data.stepperStateCmd.position[i], true );
    // actor[i]->setDistance( com->data.stepperStateCmd.distance[i], false, true );
    actor[i]->setIsHoming( com->data.stepperStateCmd.isHoming[i] );
    actor[i]->setIsRunning( com->data.stepperStateCmd.isRunning[i] );
  }
  return true;
 
} 

void SwOSSwarmPwrDrive::read( void ) {

  uint8_t state[4];
  long    position[4];
  long    distance[4];

  // don't send packets to myself, so I need to now last reading time
  _lastContact = millis();
  
  // I'm alive
  pwrDrive->Watchdog( 500 );

  // get input & motor states
  pwrDrive->getStateAll( &state[0], &state[1], &state[2], &state[3] );
  pwrDrive->getPositionAll( &position[0], &position[1], &position[2], &position[3] );
  pwrDrive->getStepsToGoAll( &distance[0], &distance[1], &distance[2], &distance[3] );
  
  for ( uint8_t i=0; i<4; i++ ) {

    // flag 1 motor is running, flag 2 endstop, flag 3 EMS, flag 4 homing
    input[i]->setValue( ( ( state[i] & 0x02 ) > 0 ) );
    
    // MERKER
    actor[i]->setValue( distance[i], position[i], ( ( state[i] & 0x08 ) > 0 ), state[i] & 0x01 );
  
  }
  
  input[4]->setValue( ( state[0] & 0x04 ) >> 2 );

}


void SwOSSwarmPwrDrive::setMicrostepMode( uint8_t mode, bool dontSendToRemote ) {
  // set microstep mode
  
  _microstepMode = mode;

  if   (!isLocal()) {

    if (!dontSendToRemote) {
      // send remote
      SwOSCom cmd( macAddr, serialNumber, CMD_SETMICROSTEPMODE );
      cmd.data.ctrlCmd.microstepMode = mode;
      cmd.send( );
    }

  } else if ( ( getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->setMicrostepMode( mode );

  }

}

uint8_t SwOSSwarmPwrDrive::getMicrostepMode( void ) {
  // get microstep mode
  // FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP

  return _microstepMode;

}

bool SwOSSwarmPwrDrive::OnDataRecv( SwOSCom *com ) {

  if (!com) return false;

  if ( com->data.cmd == CMD_SETMICROSTEPMODE ) {
      setMicrostepMode( com->data.ctrlCmd.microstepMode, true );
      return true;
  } else {
      return SwOSCtrl::OnDataRecv(com);
  }

  return true;

}

/***************************************************
 *
 *   SwOSSwarmDuino - ftDuino implementation as I2C Slave
 *
 ***************************************************/

SwOSSwarmDuino::SwOSSwarmDuino( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda ): SwOSSwarmI2CCtrl (SN, macAddr, local, CPU,  IAmKelda ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  ftDuino = new FtDuino();

}

SwOSSwarmDuino::SwOSSwarmDuino( SwOSCom *com ):SwOSSwarmDuino( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmKelda ) {
  
}

SwOSSwarmDuino::~SwOSSwarmDuino() {

  if (ftDuino) delete ftDuino;

}

char* SwOSSwarmDuino::myType() {
  return (char *) "FTSWARMDUINO";
}

FtSwarmController_t SwOSSwarmDuino::getType() {
  return FTSWARMDUINO;
}

void SwOSSwarmDuino::read() {

  uint16_t value[8];

  if ( ftDuino ) {
    
    // get input mesurements from ftDuino
    ftDuino->getState( value );
    
    // set input measurements
    for (uint8_t i=0; i<8; i++) { 
      if ( input[i] ) input[i]->setReading( value[i] );
    }

    // errors during I2C communication?
    if ( ftDuino->getError() != 0 ) i2cerror++; else i2cerror = 0;
    if ( i2cerror > 5 ) setState( ERROR );
  }

}