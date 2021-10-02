#include "ftSwarmMotor.h"
// #include "ftSwarmInput.h"
#include "ftSwarmLED.h"
#include "ftSwarmServo.h"

#include "ftSwarm.h"
#include "SPI.h"

// ftSwarmDigitalInput A1( "", FTSWARM_A1 );

FtSwarmMotor M1( "", FTSWARM_M1 );
FtSwarmMotor M2( "", FTSWARM_M2 );

FtSwarmLED LED1( "", FTSWARM_LED1 );
FtSwarmLED LED2( "", FTSWARM_LED2 );

//ftSwarmServo Servo( "" );

SPIClass spi = SPIClass(VSPI);

void setup() {
  // put your setup code here, to run once:

  while (!Serial);
  Serial.begin(115200);  

  LED1.setBrightness( 10 );

  //Servo.setPosition(1000);
  pinMode( 32, INPUT );
  pinMode( 34, INPUT );
  pinMode( 36, INPUT );
  pinMode( 39, INPUT );

  
#define HC165_LOAD   18
#define HC165_CLKINH 19

#define SCK  12
#define MISO 14
#define MOSI 13
#define CS   HC165_CLKINH


  pinMode( HC165_LOAD, OUTPUT );
  pinMode( HC165_CLKINH, OUTPUT );
  pinMode( SCK, OUTPUT );
  pinMode( MISO, INPUT );

}

int lalala = AIN1;

void loop() {
  // put your main code here, to run repeatedly:


  digitalWrite(HC165_LOAD, LOW);
  //delayMicroseconds(5);
  digitalWrite(HC165_LOAD, HIGH);
  //delayMicroseconds(5);

  digitalWrite(SCK, LOW); // Clock aus
  //delayMicroseconds(5);
  digitalWrite(HC165_CLKINH, LOW);

  // die 8 Zustände A...H einlesen
  byte states = shiftIn(MISO, SCK, MSBFIRST);

  Serial.println( states, BIN );
  
  digitalWrite(HC165_CLKINH, HIGH); 

/*

    Serial.print( analogRead( IO32 ) );
    Serial.print(" " );
    Serial.print( analogRead( IO34 ) );
    Serial.print(" " );
    Serial.print( analogRead( IO36 ) );
    Serial.print(" " );
    Serial.println( analogRead( IO39 ) );

      uint8_t hc165;

  // load data into register
  digitalWrite( HC165_LOAD, LOW );
  delay(1);
  digitalWrite( HC165_LOAD, HIGH );

  // just be safe on timing
  delay(1);

  // transfer data

  hc165 = SPI.transfer(0);


  Serial.println( hc165 );

    
    delay(100);

  LED1.setPixelColor( BLACK );
  LED2.setPixelColor( WHITE );
  delay(2000);
  LED1.setPixelColor( LIME );
  LED2.setPixelColor( GREEN );
  delay(2000);
  LED1.setPixelColor( YELLOW );
  LED2.setPixelColor( CYAN );
  delay(2000);
  LED1.setPixelColor( MAGENTA );
  LED2.setPixelColor( SILVER );
  delay(2000);
  LED1.setPixelColor( RED );
  LED2.setPixelColor( MAROON );
  delay(2000);
  LED1.setPixelColor( BLUE );
  LED2.setPixelColor( NAVY );
  delay(2000);
  
  for (int i=0; i<256; i++ ) {
    M1.left( i );
    delay(100);
  }


  Serial.println("left");
  M1.left( 64 );
  M2.left( 64 );
  delay( 5000 );

  Serial.println("right");
  M1.right( 128 );
  M2.right( 128 );
  delay( 5000 );

  Serial.println("coast");
  M1.coast();
  M2.coast();
  delay( 5000 );

  Serial.println("brake");
  M1.brake();
  M2.brake();
  delay( 5000 );
 
  for (int i=0; i<5; i++ ) {
    digitalWrite(IO0, 0 );
    digitalWrite(IO2, 0 );
    digitalWrite(IO4, 0 );
    digitalWrite(IO5, 0 );
    digitalWrite(IO23, 0 );

    switch( i ) {
      case 0: digitalWrite(IO0, 1 ); break;
      case 1: digitalWrite(IO2, 1 ); break;
      case 2: digitalWrite(IO4, 1 ); break;
      case 3: digitalWrite(IO5, 1 ); break;
      case 4: digitalWrite(IO23, 1 ); break;
      default: break;
    }

    delay( 100 );
      
  }
*/
  
}
