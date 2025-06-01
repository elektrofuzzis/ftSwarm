/*
 * SwOSHWI2CSensor.cpp
 *
 * I2C based sensor hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include "SwOSHW/SwOSHWI2CSensor.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWActor.h"
#include <MPU6050_6Axis_MotionApps20.h>
#include <LSM6DSRSensor.h>

/***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

LSM6DSRSensor *lsm = NULL;
MPU6050       *mpu = NULL;

SwOSGyro::SwOSGyro(const char *name, SwOSCtrl *ctrl ) : SwOSIO( name, ctrl, SWOSIO_GYRO ) {

}


/***************************************************
 *
 *   SwOSGyroLSM
 *
 ***************************************************/

SwOSGyroLSM::SwOSGyroLSM(const char *name, SwOSCtrl *ctrl ) : SwOSGyro( name, ctrl ) {

  if (ctrl->isLocal() ) setupLocal();

}

SwOSGyroLSM::~SwOSGyroLSM( ) {

  if ( lsm ) delete lsm;

}

void SwOSGyroLSM::setupLocal() {

  // need an internal I²C interface
  TwoWire internalI2C = TwoWire(1);
  internalI2C.begin( 4, 5 );

  // create gyro object
  lsm = new LSM6DSRSensor(&internalI2C, LSM6DSR_I2C_ADD_H);

  // pull INT1 to low to enable I2C
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << GPIO_NUM_16);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
  gpio_set_level( GPIO_NUM_16, 0 );
  delay(200);

  // start gyro
  lsm->begin();
  lsm->Enable_X();
  lsm->Enable_G();

}

void SwOSGyroLSM::read() {

  // lsm->Get_X_Axes( _accelerometer );
  // lsm->Get_G_Axes( _gyroscope );

}


/***************************************************
 *
 *   SwOSGyroMPU
 *
 ***************************************************/

SwOSGyroMPU::SwOSGyroMPU(const char *name, SwOSCtrl *ctrl ) : SwOSGyro( name, ctrl ) {

  if (ctrl->isLocal() ) setupLocal();

}

SwOSGyroMPU::~SwOSGyroMPU( ) {

  if ( mpu ) delete mpu;

}


void SwOSGyroMPU::setupLocal() {

  uint8_t devStatus;      // Return status after each device operation (0 = success, !0 = error)

  if (!mpu) {
    
    mpu = new MPU6050();

    /*Initialize device*/
    printf("Initializing MPU6050\n");
    mpu->initialize();

    /*Verify connection*/
    if(mpu->testConnection() == false){
      ESP_LOGE(LOGFTSWARM, "Gyro/MPU6050 connection failed.");
      ctrl->setState( ERROR );
      delete mpu;
      mpu = NULL;      
    }

    /* Initializate and configure the DMP*/
    devStatus = mpu->dmpInitialize();

    /* gyro offsets, scaled for min sensitivity */
    mpu->setXGyroOffset(220);
    mpu->setYGyroOffset(76);
    mpu->setZGyroOffset(-85);
    mpu->setXAccelOffset(0);
    mpu->setYAccelOffset(0);
    mpu->setZAccelOffset(1688);

    /* Making sure it worked (returns 0 if so) */ 
    if (devStatus == 0) {
      mpu->CalibrateAccel(6);  // Calibration Time: generate offsets and calibrate our MPU6050
      mpu->CalibrateGyro(6);
      mpu->setDMPEnabled(true);
      packetSize = mpu->dmpGetFIFOPacketSize(); //Get expected DMP packet size for later comparison
    } else {
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      ESP_LOGE(LOGFTSWARM, "Gyro/MPU6050: DMP initialisation error %d.", devStatus);
      ctrl->setState( ERROR );
      delete mpu;
      mpu = NULL;      
    } 

    }

}

uint8_t SwOSGyroMPU::pushState( uint8_t *buffer ) { 
  
  uint8_t *ptr = buffer;
  
  memcpy( ptr, &q.w,  sizeof( q.w ) );  *ptr += sizeof( q.w );
  memcpy( ptr, &q.x,  sizeof( q.x ) );  *ptr += sizeof( q.x );
  memcpy( ptr, &q.y,  sizeof( q.y ) );  *ptr += sizeof( q.y );
  memcpy( ptr, &q.z,  sizeof( q.z ) );  *ptr += sizeof( q.z );
  memcpy( ptr, &aa.x, sizeof( aa.x ) ); *ptr += sizeof( aa.x );
  memcpy( ptr, &aa.y, sizeof( aa.y ) ); *ptr += sizeof( aa.y );
  memcpy( ptr, &aa.z, sizeof( aa.z ) ); *ptr += sizeof( aa.z );

  return ptr - buffer;

};

uint8_t SwOSGyroMPU::popState( uint8_t *buffer ) { 

  uint8_t *ptr = buffer;
  
  memcpy( &q.w,  ptr, sizeof( q.w ) );  *ptr += sizeof( q.w );
  memcpy( &q.x,  ptr, sizeof( q.x ) );  *ptr += sizeof( q.x );
  memcpy( &q.y,  ptr, sizeof( q.y ) );  *ptr += sizeof( q.y );
  memcpy( &q.z,  ptr, sizeof( q.z ) );  *ptr += sizeof( q.z );
  memcpy( &aa.x, ptr, sizeof( aa.x ) ); *ptr += sizeof( aa.x );
  memcpy( &aa.y, ptr, sizeof( aa.y ) ); *ptr += sizeof( aa.y );
  memcpy( &aa.z, ptr, sizeof( aa.z ) ); *ptr += sizeof( aa.z );
  
  return ptr - buffer;
  
};

void SwOSGyroMPU::read() {

  uint8_t FIFOBuffer[64]; // FIFO storage buffer

  if (!mpu) return;

  // if FIFO doesn't have data, skip
  if (mpu->getFIFOCount() < packetSize ) return;

  // last packet in FIFO
  if (mpu->dmpGetCurrentFIFOPacket(FIFOBuffer)) {
    mpu->dmpGetQuaternion(&q, FIFOBuffer);
    mpu->dmpGetAccel(&aa, FIFOBuffer);
  }

}

void SwOSGyroMPU::getAcceleration( float *x, float *y, float *z ) {

  if (!mpu) return;
  
  VectorFloat gravity; 
  VectorInt16 aaReal;

  mpu->dmpGetGravity(&gravity, &q);
  mpu->dmpGetLinearAccel(&aaReal, &aa, &gravity);

  *x = aaReal.x;
  *y = aaReal.y;
  *z = aaReal.z;

};

void SwOSGyroMPU::getQuaternion( float *w, float *x, float *y, float *z ) {

  if (!mpu) return;

  *w = q.w;
  *x = q.x;
  *z = q.z;

};

void SwOSGyroMPU::getYawPitchRoll(float *yaw, float *pitch, float *roll, bool radiants ) {

  if (!mpu) return;

  float ypr[3];
  VectorFloat gravity; 

  mpu->dmpGetGravity( &gravity, &q );
  mpu->dmpGetYawPitchRoll( ypr, &q, &gravity );
  
  *yaw   = ypr[0];
  *pitch = ypr[1];
  *roll  = ypr[2];

  if ( !radiants ) {
    *yaw   *= 180/M_PI;
    *pitch *= 180/M_PI;
    *roll  *= 180/M_PI;  
  }

};

void SwOSGyroMPU::getEuler(float *alpha, float *beta, float *gamma, bool radiants ) {

  if (!mpu) return;
  
  float euler[3];
  mpu->dmpGetEuler( euler, &q );
  
  *alpha = euler[0];
  *beta  = euler[1];
  *gamma = euler[2];

  if ( !radiants ) {
    *alpha *= 180/M_PI;
    *beta  *= 180/M_PI;
    *gamma *= 180/M_PI;  
  }

};

void SwOSGyroMPU::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
  
  json->variable4F( "Quaternion", q.w, q.x, q.y, q.z);
  json->variable3I16( "Acceleration", aa.x, aa.y, aa.z );
  
  json->endObject();
}

/***************************************************
 *  
 * SwOSLidarInput
 *
 ***************************************************/

 VL53L0X Lidar;

 SwOSLidarInput::SwOSLidarInput(const char *name, SwOSCtrl *ctrl ) : SwOSInput( name, SWOS_NOPORT, ctrl, SWOSIO_LIDAR ) {
   
   // initialize local HW
   if (ctrl->isLocal()) {
       setupLocal();
   }
 
 }
 
 void SwOSLidarInput::setupLocal() {
   // initialize local HW
 
   SwOSInput::setupLocal( );
 
   Lidar.setTimeout(500);
   Lidar.init();
   // Lidar.setMeasurementTimingBudget(20000);
   Lidar.startContinuous(100);
 }
  
 void SwOSLidarInput::read() {
   
   // no work on remote sensors
   if (!ctrl->isLocal()) return;
 
   uint32_t newValue;
 
   // read new data
   newValue = Lidar.readRangeContinuousMillimeters();
 
   setReading( newValue );
 
 }
 
 void SwOSLidarInput::setReading( int32_t newValue ) {
     
   // store new data
   lastRawValue = newValue;  
 
   subscription();
 
 }
 
 void SwOSLidarInput::setValue( int32_t value ) {
 
   // no work on real local HW
   if ( ( ctrl->isLocal()) && (!ctrl->isI2CSwarmCtrl() ) ) return;
   
   lastRawValue = value;
 
   subscription();
 
 }
 
 void SwOSLidarInput::jsonize( JSONize *json, uint8_t id) {
   
   json->startObject();
   SwOSIO::jsonize(json, id);
   json->variableI32("value", getValueI32() );   
   json->endObject();

 }

 /***************************************************
 *
 *   I2C Slave
 *
 ***************************************************/

// internal copy of the registers
uint8_t I2CSlave_register = 0;
uint8_t I2CSlave_value[MAXI2CREGISTERS];
bool    I2CSlave_read = false;

void I2CReceiveEvent(int bytesReceived){
  // is called when I2C data is received

  uint8_t value;
  
  // 0 bytes shouldn't happen at all
  // 1 byte  - it's a register read, need to send the requested data
  // 2 bytes - it's a register write, neeed to inform Kelda
  // > bytes - ignore

  switch (bytesReceived) {

/* ftSeilbahn Fix
    case 1:   // Read register
              I2CSlave_register = Wire.read();
              break;
*/

    case 2:   // Write register
              I2CSlave_register = Wire.read();
              value = Wire.read();
              if (I2CSlave_register<MAXI2CREGISTERS) I2CSlave_value[ I2CSlave_register ] = value;
              break;

    default:  for (uint8_t i=0; i<bytesReceived; i++) Wire.read();
              break;

  }

}

void I2CRequestEvent() {
  // i2C-Interrupt to send data to master

  for (uint8_t i=0; i<nvs.I2CRegisters; i++) Wire.write( I2CSlave_value[i] );
  I2CSlave_read = true;
  
}

void SwOSI2C::read( ) {
  
  for( uint8_t i=0; i<MAXI2CREGISTERS; i++) myRegister[i] = I2CSlave_value[i];

  if ( (I2CSlave_read) && ( nvs.interruptLine ) ) { 
    I2CSlave_read=false; 
    if (intIO) {
      intIO->setSpeed(nvs.interruptOnOff[0]);
      intIO->apply();
    }
  }

}

void SwOSI2C::setupLocal(uint8_t I2CAddress) {

  Wire.begin(I2CAddress);
  Wire.onReceive(I2CReceiveEvent);
  Wire.onRequest(I2CRequestEvent);
  
  if ( nvs.interruptLine ) { 
    I2CSlave_read=false; 
    if (intIO) {
      intIO->setSpeed(nvs.interruptOnOff[0]);
      intIO->apply();      
    }
  }

}

SwOSI2C::SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t I2CAddress):SwOSIO( name, ctrl, SWOSIO_I2C ) {

  memset(myRegister, 0, sizeof(myRegister));
  
  if (ctrl->isLocal()) setupLocal(I2CAddress);

}

void SwOSI2C::setRegister( uint8_t reg, uint8_t value ) {

  // check on boundaries
  if (reg>=MAXI2CREGISTERS) return;
  
  myRegister[reg] = value;

  if (ctrl->isLocal()) setLocal( reg, value );
  else                 setRemote( reg, value );

}

void SwOSI2C::setRemote( uint8_t reg, uint8_t value ) {
  
  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_I2CREGISTER );
  cmd.data.I2CRegisterCmd.index = ctrl->getIndex( this );
  cmd.data.I2CRegisterCmd.reg   = reg;
  cmd.data.I2CRegisterCmd.value = value;
  cmd.send( );

}

void SwOSI2C::setLocal( uint8_t reg, uint8_t value ) {

  I2CSlave_value[reg] = value;

  if ( nvs.interruptLine ) { 
    // Use M1/M2 as interrupt line
    
    // reset read semaphore
    I2CSlave_read = false;

    // get MotorIO
    intIO = (SwOSMotor*) ctrl->getIO( SWOSIO_MOTOR, nvs.interruptLine - 1 + FTSWARM_M1 );

    if (intIO) {

      // if the remote controller didn't ack the last interrupt, so I need to reset the interupt line first 
      if ( intIO->getSpeed() != nvs.interruptOnOff[0] ) {
        intIO->setSpeed(nvs.interruptOnOff[0]);
        intIO->apply();
        delay(1);
      }

      // set interrupt
      intIO->setSpeed(nvs.interruptOnOff[1]);
      intIO->apply();

    }

  }

}

uint8_t SwOSI2C::getRegister( uint8_t reg ) {

  if (reg>=MAXI2CREGISTERS) return 0;
  else return myRegister[reg];

}

uint8_t SwOSI2C::pushState( uint8_t *buffer ) {

  memcpy( buffer, myRegister, MAXI2CREGISTERS );
  return MAXI2CREGISTERS;

}

uint8_t SwOSI2C::popState( uint8_t *buffer ) {

  memcpy( myRegister, buffer, MAXI2CREGISTERS );
  return MAXI2CREGISTERS;

}