#include <Arduino.h>
#include "ftSwarmRS.h"
#include "easyKey.h"

#include "SwOSCom.h"

#define FIRMWARE

#ifdef FIRMWARE
void setup() {

Serial.begin(115200);

/*
#if CONFIG_IDF_TARGET_ESP32S3
   pinMode(12, INPUT);
   delay(100);
   if (analogRead(12)>2000) {
    Serial.println("stopped");
    while ((analogRead(12)>2000)) delay( 500 );
   }
   Serial.println("continued");
#endif
*/

   // start the swarm
   ftSwarm.verbose(true);
   FtSwarmSerialNumber_t local = ftSwarm.begin( false );
   printf("Press any key to start setup...\n");
   

}

int i=0; int j=0;

void loop() {

  if (anyKey() ) {
   ftSwarm.setup();
   ESP.restart();    
  }

  delay(250);

  i++;
  j=i/4;

  if (i%4 == 0) {
  switch (j%10) {
    case 0: printf("\n0");
    case 5: printf("5");
    default: printf(".");
  }
  fflush(stdout);
  }
}

#else // no Firmware

/* Fixes
 *  
 *  setPower to Remote negative values
 *  
 *  
 *   -------------------------------
 *   open;
 *   anykey to start setup in firmware mode
 *   refactor enterXX-function to use interrupts
 *   
 *   UI: LightBarrier, XMMOTOR, Kreise schieben
 *   Smartphone geht nicht
 *   
 *   NVS Servo speichen in JST gefixt
 *    esp_task_wdt_init(30, false);
 *   IP ADresse wird unter 2.0.5 angezeigt
 *   negative Motorbewegung
 *   kein wifi sleep
 *   pairing im AP mode
 */

FtSwarmAnalogInput *A01;
FtSwarmAnalogInput *A02;
FtSwarmAnalogInput *A03;
FtSwarmAnalogInput *A04;
FtSwarmAnalogInput *A05;
FtSwarmAnalogInput *A06;

FtSwarmLED *LED1;
FtSwarmLED *LED2;
FtSwarmLED *LED3;

FtSwarmMotor *M1;
FtSwarmMotor *M2;

void setup( ) {

  Serial.begin(115200);

  Serial.print("Hello - This is my Swarm Test Sketch");
  Serial.println("");

  Serial.println("Swarm Begin");
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
  Serial.println("Swarm Begin successfull");

  Serial.print("Local="); Serial.println(local);

  A01 = new FtSwarmAnalogInput( local, FTSWARM_A1 );
  A02 = new FtSwarmAnalogInput( local, FTSWARM_A2 );
  A03 = new FtSwarmAnalogInput( local, FTSWARM_A3 );
  A04 = new FtSwarmAnalogInput( local, FTSWARM_A4 );
  A05 = new FtSwarmAnalogInput( local, FTSWARM_A5 );
  A06 = new FtSwarmAnalogInput( local, FTSWARM_A6 );
  Serial.println("local inputs found.");

  LED1 = new FtSwarmLED( local, FTSWARM_LED1 );
  LED2 = new FtSwarmLED( local, FTSWARM_LED2 );
  LED3 = new FtSwarmLED( local, FTSWARM_LED3 );
  LED1->setColor( CRGB::Blue );
  LED2->setColor( CRGB::Red );
  LED3->setColor( CRGB::Green );
  // LED1->setBrightness( 2 );
  Serial.println("local LEDs found.");

  M1 = new FtSwarmMotor( local, FTSWARM_M1 );
  M2 = new FtSwarmMotor( local, FTSWARM_M2 );
  M1->setSpeed(-255);
  M2->setSpeed(-255);
  Serial.println("local Motors found.");


}
void loop( ) {

  printf("A1: %d A2:%d A3:%d A4:%d A5:%d A6:%d\n", A01->getValue(), A02->getValue(), A03->getValue(), A04->getValue(), A05->getValue(), A06->getValue() );

  // just a small delay to 
  delay( 500 );

}

#endif
