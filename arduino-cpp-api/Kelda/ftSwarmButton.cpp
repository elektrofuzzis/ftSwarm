//////////////////////////////////////////////////////////////////
//
// FtSwarmButton.h
//
// ftSwarm Switch Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#include "FtSwarmButton.h"


// Local class ftSwarmHC165 to represent the local shift register.
// This class should have only one instance!

class ftSwarmHC165 {
  protected:
  public:
    ftSwarmHC165();
      // constructor
    uint8_t getValue();
      // read register
};

ftSwarmHC165::ftSwarmHC165( ) {
  
  pinMode( LD165, OUTPUT );
  pinMode( CS165, OUTPUT );
  pinMode( CLK165, OUTPUT );
  pinMode( MISO165, INPUT );
    
}

uint8_t ftSwarmHC165::getValue() {

  uint8_t myValue;

  // load parallel inputs into shift register
  digitalWrite(LD165,  LOW);
  digitalWrite(LD165,  HIGH);

  // prepare CLK/CS
  digitalWrite(CLK165, LOW); // set clock off
  digitalWrite(CS165,  LOW); // CS

  // get data
  myValue = shiftIn(MISO165, CLK165, MSBFIRST);

  // CS
  digitalWrite(CS165, HIGH);

  return myValue;
  
}

ftSwarmHC165 hc165;

/////////////////////////////////////////////////////////////
//
// now start implementation of the button class
//
/////////////////////////////////////////////////////////////

FtSwarmButton::FtSwarmButton( char* UUID, uint8_t switchPort ) : FtSwarmObj( UUID, switchPort ) {
  
  // noting to initialize, just use hc165. (which is aready initialized)
  
}


FtSwarmButtonState FtSwarmButton::getState() {

  if (isLocal()) {

    uint8_t val = hc165.getValue();
    if ( val & (1 << _port) ) {
      return PRESSED;
    } else {
      return RELEASED;
    }
    
  } else {

    return (FtSwarmButtonState) rpc( GETSTATE );
    
  }
}

boolean FtSwarmButton::isPressed( ) {
  return ( getState() == PRESSED );
}

boolean FtSwarmButton::isReleased( ) {
  return ( getState() == RELEASED );
}

char* FtSwarmButton::classname() {
  return "FtSwarmButton";
}
