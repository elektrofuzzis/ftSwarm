//////////////////////////////////////////////////////////////////
//
// ftSwarmSwitch.h
//
// ftSwarm Switch Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#include "ftSwarmSwitch.h"
#include "SPI.h"

#define HC165_LOAD   IO18
#define HC165_CLKINH IO19

ftSwarmSwitch::ftSwarmSwitch( char* UUID, uint8_t switchPort ) : ftSwarmObj( UUID, switchPort ) {

  if (isLocal()) {
    
    // set PinModes
    pinMode( HC165_LOAD, OUTPUT );
    pinMode( HC165_CLKINH, OUTPUT );
    digitalWrite( HC165_LOAD, HIGH );
    digitalWrite( HC165_CLKINH, HIGH );
    
    // start SPI
    SPI.begin( SPI_CLK, SPI_MISO, SPI_MOSI );
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE2);
    
  } else {
    
  }
  
}

uint8_t ftSwarmSwitch::getHC165() {

  uint8_t hc165;

  // load data into register
  digitalWrite( HC165_LOAD, LOW );
  delay(1);
  digitalWrite( HC165_LOAD, HIGH );

  // just be safe on timing
  delay(1);

  // transfer data
  digitalWrite( HC165_LOAD, LOW );
  hc165 = SPI.transfer(0);
  digitalWrite( HC165_LOAD, HIGH );

  return hc165;
  
}

ftSwarmSwitchState ftSwarmSwitch::getState() {

  if (isLocal()) {

    uint8_t hc165 = getHC165();
    if ( hc165 & (1 << _Port) ) {
      return PRESSED;
    } else {
      return RELEASED;
    }
    
  } else {
    return UNKNOWN;
  }
}

boolean ftSwarmSwitch::isPressed( ) {
  return ( getState() == PRESSED );
}

boolean ftSwarmSwitch::isReleased( ) {
  return ( getState() == RELEASED );
}
