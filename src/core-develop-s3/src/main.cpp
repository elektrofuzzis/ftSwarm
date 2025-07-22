#include <Arduino.h>
#include <Wire.h>

#include "SwOS.h"
#include "SwOSSwarm.h"
#include "easyKey.h"
#include "SwOSFilter.h"
#include <fastLed.h>
#include "pb_encode.h"
#include "ftswarm.pb.h"

//#define FIRMWARE
// #define SEILBAHN
// #define CONVENTION24
// #define PIDTEST
// #define LSM

void setup() {

    Serial.begin(115200);

  _ftSwarm_Controller pbctl = {
    .cpu = (_ftSwarm_version) FTSWARMRC_1V140
  };

  uint8_t buffer[256];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  bool ok = pb_encode(&stream, ftSwarm_Controller_fields, &pbctl);
  printf("ok %d bytes %d\n", ok, stream.bytes_written );

  while(1) loop;

  firmware();
  ESP.restart();

  
}


void loop() {

  delay(500);
  /*

  setPWM( LEDC_CHANNEL_0, 0);
  delay(500);

  setPWM( LEDC_CHANNEL_0, 2048);
  delay(500);

  setPWM( LEDC_CHANNEL_0, 4095); */

  delay(500);

}

#ifdef PIDTEST

FtSwarmMotor       *motor = NULL;
FtSwarmAnalogInput *poti  = NULL;

void setup( ) {

  Serial.begin(115200);

  ftSwarm.begin(true);
  motor = new FtSwarmMotor( "motor" );
  poti  = new FtSwarmAnalogInput( "poti");

  while (!poti->getValue()) delay(50);

}

class PIDControl {

  public:
  
    float kp;
    float ki;
    float kd;
    float max_integral;
    float min_integral;
    float max_output;
    float min_output;
    float integral = 0;
    float last_error = 0;
    float desired_value;

    PIDControl( float desired_value, float kp, float ki, float kd, float min_integral, float max_integral, float min_output, float max_output );
    float solve( float sensor );
    void reset( float sensor ) { integral = 0; last_error = 0; last_error = desired_value - sensor; };

};

PIDControl::PIDControl( float desired_value, float kp, float ki, float kd, float min_integral, float max_integral, float min_output, float max_output ) {
  this->desired_value = desired_value;
  this->kp = kp;
  this->ki = ki;
  this->kd = kd;
  this->min_integral = min_integral;
  this->max_integral = max_integral;
  this->min_output   = min_output;
  this->max_output   = max_output;
}

float PIDControl::solve( float sensor ) {

  float error = desired_value - sensor;
  integral += error;
  integral = max( integral, max_integral );
  integral = min( integral, min_integral );
  float derivative = ( error - last_error);
  float output = kp * error + ki * integral + kd * derivative;
  output = min( output, max_output );
  output = max( output, min_output );
  last_error = error;
  return output;

}

PIDControl pid( 800, 2, 1, 0, 0, 100, -512, 512);

void run(void){

  pid.reset( poti->getValue() );

  printf("start %d\n", poti->getValue() );

  while ( abs( poti->getValue() - pid.desired_value ) > 10 ) {

    int16_t speed = pid.solve( poti->getValue() );
    if (abs(speed)<64) { 
      if ( speed < 0 ) speed = -64;
      else speed = 64;
    }
    motor->setSpeed( speed );
    // if ( poti->getValue() - pid.desired_value < 0 ) speed = -speed;
    printf("pos %d speed %d\n", poti->getValue(), speed );
    delay(50);

  }

  motor->setSpeed(0);
  printf("stop %d\n", poti->getValue() );

}

void test( int16_t speed, int16_t duration ) {

  motor->setSpeed( speed );
  delay( duration );
  motor->setSpeed( 0 );

}

void loop() {

  Menu menu;

  while (1) {

    printf("value %d\n", poti->getValue() );

    menu.start( "RC PID Test", 14 );
    menu.addF("desired_value", pid.desired_value, 1 );
    menu.addF("ki", pid.ki, 2 );
    menu.addF("kp", pid.kp, 3 );
    menu.addF("kd", pid.kd, 4 );
    menu.addF("min_integral", pid.min_integral, 5 );
    menu.addF("max_integral", pid.max_integral, 6 );
    menu.addF("min_output", pid.min_output, 7 );
    menu.addF("max_output", pid.max_output, 8 );
    menu.add("Testlauf -", "", 9);
    menu.add("Testlauf +", "", 10);
    menu.add("run", "", 11);

    switch( menu.userChoice() ) {
      
      case 0:  break;
      case 1:  pid.desired_value = enterNumberF( "Please enter desired value: ", pid.desired_value, 0, 4096 );
               break;
      case 2:  pid.ki = enterNumberF( "Please enter ki: ", pid.ki, 0, 20 );
               break;      
      case 3:  pid.kp = enterNumberF( "Please enter kp: ", pid.kp, 0, 20 );
               break; 
      case 4:  pid.kd = enterNumberF( "Please enter kp: ", pid.kd, 0, 20 );
               break; 
      case 5:  pid.min_integral = enterNumberF( "Please enter min_integral: ", pid.min_integral, 0, 4096 );
               break; 
      case 6:  pid.max_integral = enterNumberF( "Please enter max_integral: ", pid.max_integral, 0, 4096 );
               break; 
      case 7:  pid.min_output = enterNumberF( "Please enter min_output: ", pid.min_output, -4096, 4096 );
               break; 
      case 8:  pid.max_output = enterNumberF( "Please enter max_output: ", pid.max_output, -4096, 4096 );
               break; 
      case 9:  test( -128, 500 );
               break;
      case 10: test( +128, 500 );
               break;
      case 11: run();
               break;
      default: break;
    }
    
  }

}

#endif

#ifdef LSM

#include "lsm6dsr_reg.h"

TwoWire internalI2C = TwoWire(1);

static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,  uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void platform_delay(uint32_t ms) { delay(ms); };

static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,uint16_t len) {

  internalI2C.beginTransmission(((uint8_t)(((0xD7U) >> 1) & 0x7F)));
  internalI2C.write(reg);
  internalI2C.write(bufp, len);
  internalI2C.endTransmission();

  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {

  internalI2C.beginTransmission(((uint8_t)(((0xD7U) >> 1) & 0x7F)));
  internalI2C.write(reg);
  internalI2C.endTransmission(false);

  internalI2C.requestFrom(((uint8_t)(((0xD7U) >> 1) & 0x7F)), (uint8_t) len);

  int i=0;
  while (internalI2C.available()) {
    bufp[i] = internalI2C.read();
    i++;
  }

  return 0;
}

lsm6dsr_ctx_t dev_ctx;

void setup() {

  Serial.begin(115200);

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
  delay(200);
  
  internalI2C.begin( 4, 5 );
  internalI2C.setClock(400000);

  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = &internalI2C;

  /* Check device ID */
  uint8_t whoamI;
  lsm6dsr_device_id_get(&dev_ctx, &whoamI);
  if (LSM6DSR_ID != whoamI ) {
    log_e("no LSM6 found\n");
    while(1) delay(500);
  }

  /* Restore default configuration */
  printf("Restore default configuration\n");
  lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);

  uint8_t rst;
  do {
    lsm6dsr_reset_get(&dev_ctx, &rst);
  } while (rst);

  /* Disable I3C interface */
  lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);
  /* Enable Block Data Update */
  lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

  // from lsm6dsr_STdC/examples/lsm6dsr_offset.c
  /* Accelerometer X,Y,Z axis user offset correction expressed
   * in two’s complement. Set X to 48mg, Y tp 64 mg, Z to -127 mg
   */
  uint8_t offset[3] = { 15, 255-20, 33 };
  //uint8_t offset[3] = { 0x30, 0x40, 0x7E };
  lsm6dsr_xl_usr_offset_x_set(&dev_ctx, &offset[0]);
  lsm6dsr_xl_usr_offset_y_set(&dev_ctx, &offset[1]);
  lsm6dsr_xl_usr_offset_z_set(&dev_ctx, &offset[2]);
  lsm6dsr_xl_usr_offset_set(&dev_ctx, PROPERTY_ENABLE);

  lsm6dsr_all_sources_t all_source;

  /* Set Output Data Rate */
  lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_12Hz5);
  lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_12Hz5);
  /* Set full scale */
  lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);
  lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_2000dps);
  /* Configure filtering chain(No aux interface)
   * Accelerometer - LPF1 + LPF2 path
   */
  lsm6dsr_xl_hp_path_on_out_set(&dev_ctx, LSM6DSR_LP_ODR_DIV_100);
  lsm6dsr_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);

  printf("*done*\n");

  pedo_count_sample_t x;

}

void loop() { // lsm6dsr_multi_read_fifo.c

  uint8_t reg;
  
  int16_t data_raw_acceleration[3];
  float acceleration_mg[3];

  int16_t data_raw_angular_rate[3];
  float angular_rate_mdps[3];

  int16_t data_raw_temperature;
  float temperature_degC;

  unsigned long t0 = micros();  

  /* Read output only if new xl value is available */
  lsm6dsr_xl_flag_data_ready_get(&dev_ctx, &reg);

  if (reg) {
    /* Read acceleration field data */
    memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
    lsm6dsr_acceleration_raw_get(&dev_ctx, (uint8_t *) data_raw_acceleration );
    unsigned long t1 = micros();
    printf("time: %lu\t", t1-t0);
    acceleration_mg[0] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[0]);
    acceleration_mg[1] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[1]);
    acceleration_mg[2] = lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[2]);
    printf( "Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\t", acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);

    float ax = acceleration_mg[0] / 100;
    float ay = acceleration_mg[1] / 100;
    float az = acceleration_mg[2] / 100;
  
    float pitch = asin( ax / sqrtf( ax*ax + ay*ay+ az*az ) );
    float roll  = atan( ay / az );

    pitch *= 180/M_PI;
    roll  *= 180/M_PI; 

    printf( "pitch: %4.2f\troll %4.2f\n", pitch, roll );
  }

  /*

  lsm6dsr_gy_flag_data_ready_get(&dev_ctx, &reg);

  if (reg) {
    // Read angular rate field data 
    memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
    lsm6dsr_angular_rate_raw_get(&dev_ctx, (uint8_t *) data_raw_angular_rate);
    angular_rate_mdps[0] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[0]);
    angular_rate_mdps[1] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[1]);
    angular_rate_mdps[2] = lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[2]);
    printf( "Angular rate [mdps]:%4.2f\t%4.2f\t%4.2f\t", angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2] );
  }

  lsm6dsr_temp_flag_data_ready_get(&dev_ctx, &reg);

  if (reg) {
    // Read temperature data 
    memset(&data_raw_temperature, 0x00, sizeof(int16_t));
    lsm6dsr_temperature_raw_get(&dev_ctx, (uint8_t *) &data_raw_temperature);
    temperature_degC = lsm6dsr_from_lsb_to_celsius( data_raw_temperature );
    printf( "Temperature [degC]:%6.2f\n", temperature_degC );
  }

  */

  delay(500);

}

#endif

#ifdef GYROTEST

#include "SwOS.h"

FtSwarmGyro *gyro;

void setup( ) {

  Serial.begin(115200);

  FtSwarmSerialNumber_t local = ftSwarm.begin(true);

  firmware();
  ESP.restart();

  gyro = new FtSwarmGyro( local, 99 );

}

void loop() {

  float y, p, r, w, x, z;

  gyro->getYawPitchRoll( &y, &p, &r );
  printf("YPR y: %f\tp: %f\tr: %f\n", y, p, r );

  gyro->getQuaternion( &w, &x, &y, &z );
  printf("QUA w: %f\tx: %f\ty: %f\tz: %f\n", w, x, y, z );

  gyro->getEuler( &x, &y, &z );
  printf("EUL x: %f\ty: %f\tz: %f\n", x, y, z );

  gyro->getAcceleration( &x, &y, &z );
  printf("ACC x: %f\ty: %f\tz: %f\n\n", x, y, z );

  delay(500);

}
#endif

#ifdef FIRMWARE
#include "SwOS.h"
#include "easyKey.h"
#include <FastLED.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_err.h>

#include "SwOSNVS.h"

void setup( ) {

  Serial.begin(115200);

  // ftSwarm.begin(true);
  
  firmware();
  ESP.restart();

}

void loop() {


  delay(250);

}
  
#endif

#ifdef SEILBAHN

#define SEILBAHNVERSION "1.0.0"

#define MAXMOTOR 20

#define SPEEDUP "SPEEDUP"
#define SLOWDOWN "SLOWDOWN"
#define TRANSPORT "TRANSPORT"
#define ROTARY1 "ROTARY1"
#define ROTARY2 "ROTARY2"
#define SPEEDOMETER "SPEEDOMETER"

#define CONTROLDELAY 100

TaskHandle_t controlTask;

bool stopControl = false;

class MotorBlock {
  public:
    uint8_t motors = MAXMOTOR;
    int16_t minSpeed = 0;
    int16_t maxSpeed = MAXSPEED4096;
    void load( const char *block, nvs_handle_t my_handle );
    void save( const char *block, nvs_handle_t my_handle );
    
};

void MotorBlock::load( const char *block, nvs_handle_t my_handle ) {

  char tag[40];
  
  sprintf( tag, "%s.motors", block );
  nvs_get_u8( my_handle, tag, &motors );
  
  sprintf( tag, "%s.min", block );
  nvs_get_i16( my_handle, tag, &minSpeed );

  sprintf( tag, "%s.max", block );
  nvs_get_i16( my_handle, tag, &maxSpeed );
}

void MotorBlock::save( const char *block, nvs_handle_t my_handle ) {

  char tag[40];
  
  sprintf( tag, "%s.motors", block );
  ESP_ERROR_CHECK( nvs_set_u8( my_handle, tag, motors ) );
  
  sprintf( tag, "%s.min", block );
  ESP_ERROR_CHECK( nvs_set_i16( my_handle, tag, minSpeed ) );

  sprintf( tag, "%s.max", block );
  ESP_ERROR_CHECK( nvs_set_i16( my_handle, tag, maxSpeed ) );
}

class MyConfig {

  public:
    MotorBlock speedUp, slowDown, transport;
    int16_t maxFrequency = 1000;

    void load( void );
    void save( void );
    void setMaxFrequency( void );

};

void MyConfig::load( void ) {

  // Open
  nvs_handle_t myHandle;
  ESP_ERROR_CHECK( nvs_open("ftSeilbahn", NVS_READWRITE, &myHandle) );

  speedUp.load("up", myHandle);
  slowDown.load("down", myHandle);
  transport.load("trans", myHandle);

  nvs_get_i16( myHandle, "maxFreq", &maxFrequency );

}

void MyConfig::save( void ) {

  // Open
  nvs_handle_t myHandle;
  ESP_ERROR_CHECK( nvs_open("ftSeilbahn", NVS_READWRITE, &myHandle) );

  speedUp.save("up", myHandle);
  slowDown.save( "down", myHandle);
  transport.save("trans", myHandle);

  ESP_ERROR_CHECK( nvs_set_i16( myHandle, "maxFreq", maxFrequency ) );

  // commit
  ESP_ERROR_CHECK( nvs_commit( myHandle ) );

}

void MyConfig::setMaxFrequency( void ) {

  FtSwarmFrequencymeter *frequencyMeter = new FtSwarmFrequencymeter( ROTARY1 );
  maxFrequency = frequencyMeter->getFrequency();

}

MyConfig myConfig;

bool checkIO( const char *prefix, uint8_t nCount ) {

  bool ok = true;
  char ioName[40];

  for (uint8_t i=0; i<nCount; i++) {
    sprintf( ioName, "%s%d", prefix, i );
    if (!ftSwarm.IOAvaliable( (const char*) ioName) ) {
      printf("%s not found\n", ioName);
      ok = false;
    }
  }

  return ok;

}

void createMotor( const char *type, MotorBlock *block, FtSwarmMotor *motor[MAXMOTOR] ) {
  
  char alias[40];

  for (uint8_t i=0; i<MAXMOTOR; i++) {

    if (i < block->motors ) {
      sprintf( alias, "%s%d", type, i+1 );
      motor[i] = new FtSwarmMotor( alias, true );

    } else {
      motor[i] = NULL;
    }
  }

}

void setMotor( int16_t frequency, int16_t speed, int16_t maxFrequency, MotorBlock *block, FtSwarmMotor *motor[MAXMOTOR] ) {

  float   speedf1 = (float) frequency / (float) maxFrequency;
  float   speedf2 = (float) speed / float(1800) *0.2 + 0.8;  // trim only 20%
  float   speedf3 = speedf1 * speedf2 * block->maxSpeed;
  int16_t speedi = speedf3;

  // printf( "freq %d speed %d f1 %f f2 %f f3 %f i %d\n", frequency, speed, speedf1, speedf2, speedf3, speedi);

  for (uint8_t i=0; i < block->motors; i++ ) {
    motor[i]->setSpeed( speedi );
  }

}

void control( void *parameter ) {

  // get configuration
  MyConfig runningConfig;
  memcpy( &runningConfig, parameter, sizeof( MyConfig ) );

 // allocate HW
  FtSwarmMotor *speedUp[MAXMOTOR];
  createMotor( SPEEDUP, &runningConfig.speedUp, speedUp );

  FtSwarmMotor *slowDown[MAXMOTOR];
  createMotor( SLOWDOWN, &runningConfig.slowDown, slowDown );

  FtSwarmMotor *transport[MAXMOTOR];
  createMotor( TRANSPORT, &runningConfig.transport, transport );

  FtSwarmFrequencymeter *rotary = new FtSwarmFrequencymeter( ROTARY1 );

  FtSwarmAnalogInput *speedometer = new FtSwarmAnalogInput( SPEEDOMETER );

  int16_t frequencyBuffer[5] = {0,0,0,0,0};
  int16_t frequency = 0;
  int16_t last_frequency = -1;
  uint8_t frequency8 = 0;
  int16_t speed = 0;

  while ( !stopControl ) {

    // mean value
    frequency = 0;
    for (uint8_t i=0; i<5; i++ ) {
      frequency += rotary->getFrequency();
      vTaskDelay( CONTROLDELAY / portTICK_PERIOD_MS );
    }
    frequency = frequency / 5;

    // speed correction poto
    speed = speedometer->getValue();

    // send freq to swarm
    // if (frequency != last_frequency ) {

      last_frequency = frequency;

      // expect only values between 0 and 255
      if (abs(frequency) > 255 ) frequency8 = 255;
      else                       frequency8 = abs( frequency );

      // send data
      Wire.beginTransmission( 0x33 );
      Wire.write(0);            // Register 0
      Wire.write(frequency8);   // Frequency, expect something between 0 and 255
      Wire.endTransmission();

    //}

    setMotor( frequency, speed, runningConfig.maxFrequency, &runningConfig.speedUp, speedUp );
    setMotor( frequency, speed, runningConfig.maxFrequency, &runningConfig.slowDown, slowDown );
    setMotor( frequency, speed, runningConfig.maxFrequency, &runningConfig.transport, transport );

    vTaskDelay( CONTROLDELAY / portTICK_PERIOD_MS );

  }

  ftSwarm.halt();

  vTaskDelete( controlTask );

}

void motorMenu( char *type, MotorBlock *motorBlock ) {

  Menu menu;

  while (1) {

    menu.start(type, 20 );

    menu.add("Number of motors", motorBlock->motors, 1);
    menu.add("Minimum speed", motorBlock->minSpeed, 2);
    menu.add("Maximum speed", motorBlock->maxSpeed, 3);

    switch( menu.userChoice() ) {
    
      case 0:   // exit
                return;

      case 1:   motorBlock->motors = enterNumber( "Enter number of motors: ", motorBlock->motors, 1, 20 );
                checkIO( type, motorBlock->motors );
                break;

      case 2:   motorBlock->minSpeed = enterNumber( "Enter minimum speed [0..max speed]: ", motorBlock->minSpeed, 0, motorBlock->maxSpeed );
                break;

      case 3:   motorBlock->maxSpeed = enterNumber( "Enter minimum speed [min speed..4095]: ", motorBlock->maxSpeed, motorBlock->minSpeed, 4095 );
                break;

    }

  }

}

#define MENUCLI 1
#define MENUSAVE 2
#define MENUCONFSLOW 3
#define MENUCONFTRANS 4
#define MENUCONFSPEED 5
#define MENUCONFFREQ 6
#define MENUSETFREQ 7
#define MENUSTOP 8
#define MENUSTART 9

void menu( void ) {

  Menu menu;

  while (1) {
    menu.start("UST Configuration", 20);
    menu.add("Configure slow down motors", "", MENUCONFSLOW );
    menu.add("Configure transport motors", "", MENUCONFTRANS );
    menu.add("Configure speed up motors", "", MENUCONFSPEED );
    menu.add("Max rotary frequency", myConfig.maxFrequency, MENUCONFFREQ );
    menu.add("Set cable speed as max. frequency","", MENUSETFREQ);
    menu.add("command line", "", MENUCLI);
    menu.add("stop motors", "", MENUSTOP);
    menu.add("start motors", "", MENUSTART);
    menu.add("save configuration", "", 2);

    switch( menu.userChoice() ) {
    
      case 0:   // exit
                return;

      case MENUCLI:   
                // firmware/CLI
                firmware();
                break;

      case MENUSAVE:   
                // save
                myConfig.save();
                printf("configuration saved.\n");
                break;

      case MENUCONFSLOW:   
                // slow down motors
                motorMenu( (char *) SLOWDOWN, &myConfig.slowDown );
                break;

      case MENUCONFTRANS:   
                // transport motors
                motorMenu( (char *) TRANSPORT, &myConfig.transport );
                break;

      case MENUCONFSPEED:   
                // speed up motors
                motorMenu( (char *) SPEEDUP, &myConfig.speedUp );
                break;

      case MENUCONFFREQ:   
                myConfig.maxFrequency = enterNumber( "Enter maximum rotary frequency [0..1000]: ", myConfig.maxFrequency, 0, 1000 );
                break;

      case MENUSETFREQ:   
                myConfig.setMaxFrequency();
                break;

      case MENUSTOP:   
                stopControl = true;
                break;

      case MENUSTART:   
                if (stopControl) {
                  stopControl = false;
                  xTaskCreatePinnedToCore( control, "control", 20000, (void *) &myConfig, 1, &controlTask, 1 );
                }
                break;

      default:  break;
    }

  }

} 


void setup() {
  Serial.begin(115200);

  printf("ftSeilbahn UST Control %s\n\n (C) 2024 Stefan Fuss\n\n", SEILBAHNVERSION);
  
  myConfig.load();
  ftSwarm.begin();

  // control((void *) &myConfig);
  xTaskCreatePinnedToCore( control, "control", 20000, (void *) &myConfig, 1, &controlTask, 1 );
  
  menu();

  ESP.restart();

}

void loop () {

    delay(250);

}

#endif

#ifdef CONVENTION24

// # of controllers
#define CONTROLLERS      1

// serial numbers
#define CTRL_WHEEL       1
#define CTRL_BRIDGE      400
#define CTRL_STATION_UL  131  // Station OG links
#define CTRL_STATION_UR  134  // Station OG rechts
#define CTRL_STATION_L   125  // Station unten

const FtSwarmSerialNumber_t CTRL[CONTROLLERS] = { CTRL_STATION_L };

// LIFT
#define LIFT_MOTOR      FTSWARM_M1
#define LIFT_GF         FTSWARM_A1
#define LIFT_1ST        FTSWARM_A6
#define LIFT_2ND        FTSWARM_A5
#define LIFT_UP_SPEED   255
#define LIFT_DOWN_SPEED -255

// Blink
#define BLINK_DELAY     500
#define BLINK_ON        CRGB::DarkBlue
#define BLINK_OFF       CRGB::Black
#define BLINK_NOBLINK   CRGB::Green
#define BLINK_ERROR     CRGB::Red

// Matrix
#define MAXXPIXEL 8
#define MAXYPIXEL 7
#define MATRIX_BACKGROUND CRGB::Red
#define MATRIX_FOREGROUND CRGB::Green
#define MATRIX_TRACEDELAY 100
#define MATRIX_BRIGHTNESS 64

// Bridge
#define BRIDGE_LEFT_MOTOR  FTSWARM_M1
#define BRIDGE_RIGHT_MOTOR FTSWARM_M4
#define BRIDGE_LEFT_SPEED  4000
#define BRIDGE_RIGHT_SPEED 4000

// --------------------------------------------------------------------

class c24Blink {

  public:

    int blink[5];
    FtSwarmPixel *pixel[ CONTROLLERS ][2];

    c24Blink();

    void task(void);
    void start( FtSwarmSerialNumber_t ctrl );
    void stop( FtSwarmSerialNumber_t ctrl );
    void error( FtSwarmSerialNumber_t ctrl, const char *errMsg );
    
};

c24Blink::c24Blink() {

  for ( uint8_t i=0; i<CONTROLLERS; i++ ) {
    blink[i] = 0;
    pixel[i][0] = new FtSwarmPixel( CTRL[i], FTSWARM_LED1 );
    pixel[i][1] = new FtSwarmPixel( CTRL[i], FTSWARM_LED2 );
  }

}

void c24Blink::task(void) {

  bool on = true;

  while (true) {

    for ( uint8_t i=0; i<CONTROLLERS; i++) {

      if (blink[i]>0) {
        // ein/aus
        pixel[i][0]->setColor( ( on ) ? BLINK_ON : BLINK_OFF );
        pixel[i][1]->setColor( ( !on ) ? BLINK_ON : BLINK_OFF );

      } else if (blink[i]<0) {
        // ein/aus
        pixel[i][0]->setColor( BLINK_ERROR );
        pixel[i][1]->setColor( BLINK_ERROR );

      } else if ( pixel[i][0]->getColor() != BLINK_NOBLINK ) {
        // normal
        pixel[i][0]->setColor( BLINK_NOBLINK );
        pixel[i][1]->setColor( BLINK_NOBLINK );
      }
    }

    delay( BLINK_DELAY );
    on = !on;

  }

}

void c24Blink::start(FtSwarmSerialNumber_t ctrl) {

  for (uint8_t i=0; i<CONTROLLERS; i++) {
    if (ctrl = CTRL[i]) blink[i]++;
  }

}

void c24Blink::stop(FtSwarmSerialNumber_t ctrl) {

  for (uint8_t i=0; i<CONTROLLERS; i++) {
    if ( (ctrl = CTRL[i] ) && (blink[i]) ) blink[i]--;
  }

}

void c24Blink::error(FtSwarmSerialNumber_t ctrl, const char *errMsg) {

  // Fehler an PC senden
  printf("ERROR: %s\n", errMsg);

  // Controller auf rot setzen
  for (uint8_t i=0; i<CONTROLLERS; i++) {
    if ( (ctrl = CTRL[i] ) && (blink[i]) ) blink[i]=-1;
  }

  // stehen bleiben
  while (1) delay(1000);

}

c24Blink *blink;

void blinkTask(void *Parameter) {
  blink->task();
}

// --------------------------------------------------------------------

class c24Lift {

  public:

    FtSwarmMotor   *LiftMotor;
    FtSwarmSwitch  *LiftSwitch[3];

    c24Lift();
    void floor( int8_t f ); // fahre ins Stockwerk f

};

c24Lift::c24Lift() {

  LiftMotor     = new FtSwarmMotor( CTRL_STATION_L, LIFT_MOTOR );
  LiftSwitch[0] = new FtSwarmSwitch(  CTRL_STATION_L, LIFT_GF );
  LiftSwitch[1] = new FtSwarmSwitch(  CTRL_STATION_L, LIFT_1ST );
  LiftSwitch[2] = new FtSwarmSwitch(  CTRL_STATION_L, LIFT_2ND );
  
}

void c24Lift::floor( int8_t f) {

  // bin aktiv
  blink->start(CTRL_STATION_L);

  // check, wo bin ich
  int8_t floor = -1;
  int8_t switches = 0;
  for (uint8_t i=0; i<3; i++) {
    if (LiftSwitch[i]->getState()) {
      floor = i;
      switches++;
    }
  }

  // max 1 switch aktiv?
  if ( switches > 1) blink->error(CTRL_STATION_L, "Aufzug - mehrere Schalter aktiv");

  // bin ich nirgens? ins nächste Stockwerk runterfahren
  if (floor < 0) {

    LiftMotor->setSpeed( LIFT_DOWN_SPEED );
  
    while (!( LiftSwitch[0]->getState() || LiftSwitch[1]->getState() || LiftSwitch[2]->getState())) delay(1);

    LiftMotor->setSpeed( 0 );
  
    for (uint8_t i=0; i<3; i++) {
      if (LiftSwitch[i]->getState()) floor = i;
    }

    if (LiftSwitch[2]->getState()) blink->error(CTRL_STATION_L, "Aufzug - Endstop");

    blink->stop( CTRL_STATION_L );
  
  }

  // muss ich fahren?
  if (floor != f ) {

    bool down;

    // welche Richtung?
    if (floor > f ) {
      LiftMotor->setSpeed( LIFT_DOWN_SPEED );
      down = true;

    } else {
      LiftMotor->setSpeed( LIFT_UP_SPEED );
      down = false;
    }

    // Warte, bis das Stockwerk erreicht ist, oder fälschlicherweise oben/unten auslösen
    while (!( ( LiftSwitch[0]->getState() && (f==0 ) ) ||                   // unten angekommen 
              ( LiftSwitch[1]->getState() && (f==1 ) ) ||                   // 1. Stock angekommen
              ( LiftSwitch[2]->getState() && (f==2 ) )                      // 2. Stock angekommen
            ) )
      delay(1);

    LiftMotor->setSpeed( 0 );

    // Fehler?
    if ( ( LiftSwitch[0]->getState() && f != 0) || ( LiftSwitch[2]->getState() && f != 2 ) ) blink->error( CTRL_STATION_L, "Aufzug - Endstop");

  }

  blink->stop( CTRL_STATION_L );

}

c24Lift *lift;

// --------------------------------------------------------------------

class c24Door {
  protected:
    FtSwarmSerialNumber_t _ctrl;
    FtSwarmMotor          *_motor;
    FtSwarmSwitch         *_switch[2];
    int16_t               _speed[2];
  public:
    c24Door( FtSwarmSerialNumber_t ctrl, FtSwarmPort_t motor, FtSwarmPort_t swClose, FtSwarmPort_t swOpen, int16_t speed_close, int16_t speed_open );
    void open(void);
    void close(void);
};

c24Door::c24Door( FtSwarmSerialNumber_t ctrl, FtSwarmPort_t motor, FtSwarmPort_t swClose, FtSwarmPort_t swOpen, int16_t speed_close, int16_t speed_open ) {

  _ctrl = ctrl;
  _speed[0] = speed_close;
  _speed[1] = speed_open;

  _motor = new FtSwarmMotor( _ctrl, motor );
  _switch[0] = new FtSwarmSwitch( ctrl, swClose );
  _switch[1] = new FtSwarmSwitch( ctrl, swOpen );

}

void c24Door::open( void ) {

  blink->start( _ctrl );

  // nur fahren, wenn nicht schon offen
  if ( !_switch[1]->getState() ) {
    _motor->setSpeed( _speed[1] );
    while (!( _switch[1]->getState() ) ) delay(5);
    _motor->setSpeed( 0 );
  }
  
  blink->stop( _ctrl );

}

void c24Door::close( void ) {

  blink->start( _ctrl );

  // nur fahren, wenn nicht schon offen
  if ( !_switch[0]->getState() ) {
    _motor->setSpeed( _speed[0] );
    while (!( _switch[0]->getState() ) ) delay(5);
    _motor->setSpeed( 0 );
  }
  
  blink->stop( _ctrl );

}

c24Door *door[3];

// --------------------------------------------------------------------

class c24Matrix {
  public:
    bool trace = false;
    FtSwarmPixel *pixel[MAXXPIXEL][MAXYPIXEL];
    c24Matrix();
    void color_all( uint32_t color );
    void brightness_all( uint8_t brightness );
    void trace1( void );
    void trace2( void );
};

c24Matrix::c24Matrix() {

  for ( uint8_t x=0; x<MAXXPIXEL; x++) {
    for ( uint8_t y=0; y<MAXYPIXEL; y++) {
      pixel[x][y] = NULL; 
    }
  }

  // Zeile 0
  for ( uint8_t i=0; i<MAXXPIXEL; i++) {
    if (i>4) { 
      pixel[i][0] = NULL; 
    } else {
      pixel[i][0] = new FtSwarmPixel( CTRL_STATION_L, FTSWARM_LED7-i);
    } 
  }

  // Zeile 1
  for ( uint8_t i=0; i<MAXXPIXEL; i++) {
    if (i>4) { 
      pixel[i][1] = NULL; 
    } else {
      pixel[i][1] = new FtSwarmPixel( CTRL_STATION_L, FTSWARM_LED8+i);
    }
  }

  // Zeile 2
  for ( uint8_t i=0; i<MAXXPIXEL; i++) {
    if (i>4) { 
      pixel[i][2] = NULL; 
    } else {
      pixel[i][2] = new FtSwarmPixel( CTRL_STATION_L, FTSWARM_LED17-i);
    }
  }

  // Zeile 3
  for ( uint8_t i=0; i<MAXXPIXEL; i++) {
    if (i<=4) { 
      pixel[i][3] = new FtSwarmPixel( CTRL_STATION_UL, FTSWARM_LED7-i);
    } else if ( i == 5 ) {
      pixel[i][3] = new FtSwarmPixel( CTRL_STATION_UR, FTSWARM_LED3);
    } 
  }

  // Zeile 4
  for ( uint8_t i=0; i<MAXXPIXEL; i++) {
    if (i<=4) { 
      pixel[i][4] = new FtSwarmPixel( CTRL_STATION_UL, FTSWARM_LED8+i);
    } else if ( i == 5 ) {
      pixel[i][4] = new FtSwarmPixel( CTRL_STATION_UR, FTSWARM_LED4);
    } else if ( i == 6 ) {
      pixel[i][4] = new FtSwarmPixel( CTRL_STATION_UR, FTSWARM_LED7);
    } 
  }

  // Zeile 5
  for ( uint8_t i=0; i<MAXXPIXEL; i++) {
    if (i<=4) { 
      pixel[i][5] = new FtSwarmPixel( CTRL_STATION_UL, FTSWARM_LED17-i);
    } else if ( i== 5 ){
      pixel[i][5] = new FtSwarmPixel( CTRL_STATION_UR, FTSWARM_LED5);
    } else if ( i== 6 ){
      pixel[i][5] = new FtSwarmPixel( CTRL_STATION_UR, FTSWARM_LED6);
    } 
  }

  // Zeile 6
  for ( uint8_t i=0; i<6; i++) {
      pixel[i][6] = new FtSwarmPixel( 133, FTSWARM_LED3+i);
  }

}

void c24Matrix::brightness_all( uint8_t brightness ) {

  // da alle LEDs eines Controllers die gleiche Brightness haben, jewels für ein Pixel pro Controller setzen
  if ( pixel[0][0] ) pixel[0][0]->setBrightness( brightness );
  else blink->error( CTRL_STATION_L, "Matrix init error1");

  if ( pixel[0][3] ) pixel[0][3]->setBrightness( brightness );
  else blink->error( CTRL_STATION_UR, "Matrix init error2");

  if ( pixel[5][3] ) pixel[5][3]->setBrightness( brightness );
  else blink->error( CTRL_STATION_UL, "Matrix init error3");

}

void c24Matrix::color_all( uint32_t color ) {

  // Farber für die Matrix setzen
  for (uint8_t x=0; x<MAXXPIXEL; x++) {
    for (uint8_t y=0; y<MAXYPIXEL; y++) {
      if ( pixel[x][y] ) pixel[x][y]->setColor( color );
    }
  }

}

void c24Matrix::trace1( void ) {

  color_all( MATRIX_BACKGROUND );

  for ( int8_t i = 4; i>0; i++ ) {
    // Pixel setzen
    pixel[0][i]->setColor( MATRIX_FOREGROUND );
    pixel[1][i]->setColor( MATRIX_FOREGROUND );
    
    // warten
    delay( MATRIX_TRACEDELAY );

    // Pixel ausschalten
    pixel[0][i]->setColor( MATRIX_BACKGROUND );
    pixel[1][i]->setColor( MATRIX_BACKGROUND );

  }

}

void c24Matrix::trace2( void ) {

  color_all( MATRIX_BACKGROUND );

  for ( int8_t i = 0; i<5; i--) {
    // Pixel setzen
    pixel[0][i]->setColor( MATRIX_FOREGROUND );
    pixel[1][i]->setColor( MATRIX_FOREGROUND );
    
    // warten
    delay( MATRIX_TRACEDELAY );

    // Pixel ausschalten
    pixel[0][i]->setColor( MATRIX_BACKGROUND );
    pixel[1][i]->setColor( MATRIX_BACKGROUND );

  }

}

c24Matrix *matrix;

// --------------------------------------------------------------------

class c24Bridge {
  protected:
    FtSwarmStepperMotor *motor[2];
  public:
    c24Bridge();
    void open(void);
    void close(void);
};

c24Bridge::c24Bridge() {
  motor[0] = new FtSwarmStepperMotor( CTRL_BRIDGE, BRIDGE_LEFT_MOTOR);
  motor[1] = new FtSwarmStepperMotor( CTRL_BRIDGE, BRIDGE_RIGHT_MOTOR);

  motor[0]->setHomingOffset(1900);
  motor[1]->setHomingOffset(500);
}

void c24Bridge::open(void) {

  blink->start( CTRL_BRIDGE );

  motor[0]->setSpeed( BRIDGE_LEFT_SPEED );
  motor[1]->setSpeed( BRIDGE_RIGHT_SPEED );
  motor[0]->setHomingOffset(900);
  motor[1]->setHomingOffset(500);

  motor[0]->homing(  50000 );  //M1
  motor[1]->homing( -50000 );  //M4

  while (motor[0]->isHoming() || motor[1]->isHoming() ) delay(10);

  blink->stop( CTRL_BRIDGE );

}

void c24Bridge::close(void) {

  blink->start( CTRL_BRIDGE );

  motor[0]->setDistance( -34550, true );  // M1
  motor[1]->setDistance(  35550, true );  // M4
  motor[0]->run();
  motor[1]->run();
  
  blink->stop( CTRL_BRIDGE );

}

c24Bridge *bridge;

bool runReset = false;



void controlTask(void *Parameter) {

  int parameter[10];
  
  while (1) {
    if ( ftSwarm.waitOnUserEvent(parameter, 500 / portTICK_PERIOD_MS ) ) {
      switch (parameter[0]) {
        case 1:   if ( ( parameter[1] >= 0) && ( parameter[1] <=2 ) ) {  
                    if ( parameter[2] ) {
                      door[parameter[1]]->open();
                    } else {
                      door[parameter[1]]->close();
                    }
                  } else {
                    printf("Error parameter[1] %d\n", parameter[1]);
                  }
                  break;

        case 2:   if ( ( parameter[1] >= 0) && ( parameter[1] <=2 ) ) {  
                    lift->floor( parameter[1] );
                  } else {
                    printf("Error parameter[1] %d\n", parameter[1]);
                  }
                  break;

        case 3:   matrix->trace = (parameter[1] == 1);
                  break;
        
        case 4:   if (parameter[1] == 1) {
                    bridge->open();
                  } else {
                    bridge->close();
                  };
                  break;

        case 5:   runReset = true;
                  break;

        default:  printf("Cmd %d not found\n", parameter[0]);
                  break;
      }
    } 
  }

}

void brightnessTask(void *Parameter) {

  while (1) {
    for (uint8_t i=16; i<64; i+=2) {
      matrix->brightness_all(i);
      delay(50);
      if (matrix->trace) break;
    }
    for (uint8_t i=63; i>16; i-=2) {
      matrix->brightness_all(i);
      delay(50);
      if (matrix->trace) break;
    }
    if (matrix->trace) {
      matrix->brightness_all(64);
      while (matrix->trace) delay(250);
    }
  }

}


typedef struct {
  uint8_t x1, y1, x2, y2;
} Tracepoint_t;

#define MAXTRACEPOINT 23

const Tracepoint_t TPLIST [MAXTRACEPOINT] = {
  { 4,0, 4,1 },
  { 3,0, 3,1 },
  { 2,0, 2,1 },
  { 1,0, 1,1 },
  { 0,0, 1,1 },

  { 0,1, 1,1 },
  { 0,2, 1,2 },
  { 0,3, 1,3 },
  { 0,4, 1,4 },
  { 0,5, 1,4 },

  { 1,4, 1,5 },
  { 2,4, 2,5 },
  { 2,4, 3,5 },

  { 2,4, 3,4 },
  { 2,3, 3,3 },
  { 2,2, 3,3 },

  { 3,2, 3,3 },
  { 4,2, 4,3 },
  { 4,3, 5,3 },

  { 4,4, 5,4 },
  { 4,5, 5,4 },
  { 5,4, 5,5 },
  { 6,4, 6,5 } 
  
};

void traceTask(void *Parameter) {

  while (1) {
    if (matrix->trace) {
      for (uint8_t i=0; i<MAXTRACEPOINT-1; i++) {
        matrix->pixel[TPLIST[i].x1][TPLIST[i].y1]->setColor(MATRIX_FOREGROUND);
        matrix->pixel[TPLIST[i].x2][TPLIST[i].y2]->setColor(MATRIX_FOREGROUND);
        matrix->pixel[TPLIST[i+1].x1][TPLIST[i+1].y1]->setColor(MATRIX_FOREGROUND);
        matrix->pixel[TPLIST[i+1].x2][TPLIST[i+1].y2]->setColor(MATRIX_FOREGROUND);
        delay(1000);
        matrix->pixel[TPLIST[i].x1][TPLIST[i].y1]->setColor(MATRIX_BACKGROUND);
        matrix->pixel[TPLIST[i].x2][TPLIST[i].y2]->setColor(MATRIX_BACKGROUND);
        matrix->pixel[TPLIST[i+1].x1][TPLIST[i+1].y1]->setColor(MATRIX_BACKGROUND);
        matrix->pixel[TPLIST[i+1].x2][TPLIST[i+1].y2]->setColor(MATRIX_BACKGROUND);
        if (!matrix->trace) break;
      }
    } else { 
      delay(250); 
    }
  }

}

void resetTask(void *Parameter) {

  while (1) {

    if (runReset) {
      runReset = false;

      matrix->trace = false;
      door[0]->close();
      door[1]->close();
      door[2]->close();
      lift->floor(0);
      bridge->open();
      
    }

    delay(250);

  }
}

void setup() {
  Serial.begin(115200);

  printf("ftSüd:con Modell 2024 - Max Schuch, Christain Bergschneider, Stefan Fuss\n\n");
  
  // Schwarm bilden
  ftSwarm.begin();

  delay(1000);

  blink  = new c24Blink;
  lift   = new c24Lift;
  bridge = new c24Bridge;
  
  matrix = new c24Matrix;

  door[0] = new c24Door( CTRL_STATION_UR, FTSWARM_M1, FTSWARM_A2, FTSWARM_A1, 75, -50 );
  door[1] = new c24Door( CTRL_STATION_UL, FTSWARM_M2, FTSWARM_A4, FTSWARM_A5, -200, 200 );
  door[2] = new c24Door( CTRL_STATION_UL, FTSWARM_M1, FTSWARM_A1, FTSWARM_A2, -50, 75 ); 

  // Tasks
  xTaskCreatePinnedToCore( blinkTask, "blinkTask", 10240, NULL, 1, NULL, 1 );
  delay(100);

  xTaskCreatePinnedToCore( traceTask, "traceTask", 10240, NULL, 1, NULL, 1 );
  delay(100);
  xTaskCreatePinnedToCore( controlTask, "ControlTask", 20240, NULL, 1, NULL, 1 );
  
  delay(100);
  matrix->color_all( MATRIX_BACKGROUND );
  delay(100);
  xTaskCreatePinnedToCore( brightnessTask, "brightnessTask", 10240, NULL, 1, NULL, 1 );
  delay(100);
  xTaskCreatePinnedToCore( resetTask, "resetTask", 10240, NULL, 1, NULL, 1 );
  firmware();

}

void loop() {

  delay(500);

}

#endif