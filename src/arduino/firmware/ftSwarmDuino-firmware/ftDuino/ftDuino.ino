/*
 * ftDuino.ino
 *
 * ftDuino firmware sketch to to communicate with ftPwrDrive
 * 
 * (C) 2023 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <Wire.h>
#include <Ftduino.h>

// #define SHOWCOMMUNICATION

// ftPwrDrive commands
typedef enum { I2C_MOTOR_SET, I2C_INPUT_SET_MODE, I2C_GETSTATE } SwOSCommand_t;

// State buffer containts input measurements
uint16_t stateBuffer[8];

volatile bool dataReceived = false; 
bool connectMessagePrinted = false;

// get input data
void fillStateBuffer() {

  for (uint8_t i=0; i<8; i++) {
    stateBuffer[i] = ftduino.input_get( i );
  }

}

// send data based on an I2C request
void I2C_TxHandler(void) {

  Wire.write( (uint8_t *) stateBuffer, sizeof( stateBuffer ) );

}

// receive incomming I2C data
void I2C_RxHandler(int numBytes)
{
  uint8_t buffer[4];
  uint8_t b;

  dataReceived = true;

  #ifdef SHOWCOMMUNICATION
    Serial.print( numBytes ); Serial.println( " bytes received.");
  #endif

  // read incoming data
  for (uint8_t i=0; i< numBytes; i++ ) {  
    b = Wire.read();
    if (i<4) {
      buffer[i] = b;
    }
  }

  #ifdef SHOWCOMMUNICATION
    for (uint8_t i=0; i<4; i++ ) {
      Serial.print( buffer[i]);
      Serial.print(" ");
    }
    Serial.println("");
  #endif

  // interpret cmd
  switch ( buffer[0] ) {
    
    case I2C_GETSTATE:        fillStateBuffer();
                              break;

    case I2C_MOTOR_SET:       ftduino.motor_set( buffer[1], buffer[2], buffer[3] );
                              break;

    case I2C_INPUT_SET_MODE:  ftduino.input_set_mode( buffer[1], buffer[2] );
                              break;

  }

}

void setup() {

  Serial.begin( 115200 );

  Serial.println( "ftDuino/ftPwrDrive firmware V1.00" );
  Serial.println( "" );
  Serial.println( "(C) 2023 Christian Bergschneider & Stefan Fuss" );
  Serial.println( "" );

  // initialize ftDuino, all inputs "digital"
  ftduino.init();
  for (uint8_t i=0; i<8; i++) {
    ftduino.input_set_mode( i, Ftduino::SWITCH );
  }

  Wire.begin(0x20); 
  Wire.onReceive( I2C_RxHandler );
  Wire.onRequest( I2C_TxHandler );


}

void loop() {

  if ( ( dataReceived ) && (!connectMessagePrinted) ) {
    connectMessagePrinted = true;
    Serial.println( "ftPwrDrive connected.");
  }
  
  #ifdef SHOWCOMMUNICATION
    Serial.println( "I'm alive.");
  #endif

  delay(1000);

}
