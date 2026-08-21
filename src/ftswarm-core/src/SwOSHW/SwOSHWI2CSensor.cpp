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
#include "SwOSLog.h"

#include <MPU6050_6Axis_MotionApps20.h>
#include <LSM6DSRSensor.h>

/***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

LSM6DSRSensor *lsm = NULL;
MPU6050       *mpu = NULL;

SwOSGyro::SwOSGyro(const char *name, SwOSCtrl *ctrl, uint8_t flags ) : SwOSInput( name, SWOS_NOPORT, ctrl, SWOSIO_GYRO, flags ) {

}

void SwOSGyro::serialize( Serialize *serialize ) {

  serialize->startObject( );
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_YAWPITCHROLL, ypr[0], ypr[1], ypr[2] ); 
  serialize->endObject();
  
}

void SwOSGyro::getYawPitchRoll(float *yaw, float *pitch, float *roll, bool radiants) {

  *yaw   = ypr[0];
  *pitch = ypr[1];
  *roll  = ypr[2];

  if ( !radiants ) {
    *yaw   *= 180/M_PI;
    *pitch *= 180/M_PI;
    *roll  *= 180/M_PI;  
  }

}

uint8_t SwOSGyro::pushState( uint8_t *buffer ) { 
  
  memcpy( buffer, ypr,  sizeof( ypr ) ); 

  return sizeof( ypr );

};

uint8_t SwOSGyro::popState( uint8_t *buffer ) { 

  memcpy( ypr, buffer, sizeof( ypr ) );  
  
  return sizeof( ypr );
  
};

/***************************************************
 *
 *   SwOSGyroLSM
 *
 ***************************************************/

SwOSGyroLSM::SwOSGyroLSM(const char *name, SwOSCtrl *ctrl, uint8_t flags ) : SwOSGyro( name, ctrl, flags ) {

  if (ctrl->isLocal() ) setupLocal();

}

SwOSGyroLSM::~SwOSGyroLSM( ) {

  if ( lsm ) delete lsm;

}

void SwOSGyroLSM::setupLocal() {

  if (nvs.spiGyro) {
    // ftSwarm UC2.1.2 and above have SPI based gyros
    // during initial setup, nvs.spiGyro is set by testing on i2c

    SPIClass *vspi = new SPIClass( HSPI );
    vspi->begin( SCK, MISO, MOSI );
    lsm = new LSM6DSRSensor( vspi, SS );

  } else {
    
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

  } 

  // start gyro
  lsm->begin();
  lsm->Enable_X();
  lsm->Enable_G();
  lsm->Set_X_FS(2);      // ±2g
  lsm->Set_G_FS(250);    // ±250 dps
  lsm->Set_X_ODR(104);
  lsm->Set_G_ODR(104);

}

void SwOSGyroLSM::operate() {

  int32_t accel[3];
  int32_t gyro[3];

  lsm->Get_X_Axes(accel);   // mg
  lsm->Get_G_Axes(gyro);    // mdps

  // Time delta
  unsigned long now = micros();
  float dt = (now - lastMicros) * 1e-6f;
  lastMicros = now;

  // Accelerometer: mg → g
  float ax = accel[0] / 1000.0f;
  float ay = accel[1] / 1000.0f;
  float az = accel[2] / 1000.0f;

  // Gyroscope: mdps → rad/s
  float gx = gyro[0] * 0.001f * DEG_TO_RAD;
  float gy = gyro[1] * 0.001f * DEG_TO_RAD;
  float gz = gyro[2] * 0.001f * DEG_TO_RAD;

  // Accelerometer angles
  float rollAcc  = atan2(ay, az);
  float pitchAcc = atan2(-ax, sqrt(ay * ay + az * az));

  // Gyro integration
  rollGyro  += gx * dt;
  pitchGyro += gy * dt;
  ypr[0]    += gz * dt;

  // Complementary filter
  ypr[2] = alpha * rollGyro  + (1.0f - alpha) * rollAcc;
  ypr[1] = alpha * pitchGyro + (1.0f - alpha) * pitchAcc;

}


/***************************************************
 *
 *   SwOSGyroMPU
 *
 ***************************************************/

SwOSGyroMPU::SwOSGyroMPU(const char *name, SwOSCtrl *ctrl, uint8_t flags ) : SwOSGyro( name, ctrl, flags ) {

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
    mpu->initialize();

    /*Verify connection*/
    if(mpu->testConnection() == false){
      SWARM_LOG_ERROR( TRANSLATE( "Gyro/MPU6050 connection failed.", "Verbindung zum Gyro/MPU6050 fehlgeschlagen." ) );
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
      SWARM_LOG_ERROR( TRANSLATE( "Gyro/MPU6050: DMP initialisation error %d.", "Gyro/MPU6050: DMP Initialisierungsfehler %d." ), devStatus);
      delete mpu;
      mpu = NULL;      
    } 

  }

}

void SwOSGyroMPU::operate() {

  uint8_t FIFOBuffer[64]; // FIFO storage buffer

  if (!mpu) return;

  // if FIFO doesn't have data, skip
  if (mpu->getFIFOCount() < packetSize ) return;

  // last packet in FIFO
  if (mpu->dmpGetCurrentFIFOPacket(FIFOBuffer)) {

    Quaternion  q;
    mpu->dmpGetQuaternion(&q, FIFOBuffer);

    VectorInt16 aa;
    mpu->dmpGetAccel(&aa, FIFOBuffer);

    VectorFloat gravity; 
    mpu->dmpGetGravity( &gravity, &q );
    mpu->dmpGetYawPitchRoll( ypr, &q, &gravity );

  }

}

/* unused...

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

}; */

/***************************************************
 *  
 * SwOSLidarInput
 *
 ***************************************************/

VL53L0X Lidar;

SwOSLidarInput::SwOSLidarInput(const char *name, SwOSCtrl *ctrl, uint8_t flags ) : SwOSInput( name, SWOS_NOPORT, ctrl, SWOSIO_LIDAR, flags ) {
   
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
  
void SwOSLidarInput::operate() {
   
  // no work on remote sensors
  if (!ctrl->isLocal()) return;

  uint32_t newValue;

  // read new data
  newValue = Lidar.readRangeContinuousMillimeters();
 
  setReading( newValue, FTSWARM_NOTRIGGER );
 
}
 
void SwOSLidarInput::serialize( Serialize *serialize ) {
   
  serialize->startObject( );
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_VALUE, getValueI32() );   
  serialize->endObject();

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

  for (uint8_t i=0; i<nvs.extensionPort.I2CRegisters; i++) Wire.write( I2CSlave_value[i] );
  I2CSlave_read = true;
  
}

void SwOSI2C::operate( ) {
  
  for( uint8_t i=0; i<MAXI2CREGISTERS; i++) myRegister[i] = I2CSlave_value[i];

  if ( (I2CSlave_read) && ( nvs.extensionPort.interruptLine ) ) { 
    I2CSlave_read=false; 
    if (intIO) {
      intIO->setSpeed(nvs.extensionPort.interruptOnOff[0]);
      intIO->apply();
    }
  }

}

void SwOSI2C::setupLocal(uint8_t I2CAddress) {

  Wire.begin(I2CAddress);
  Wire.onReceive(I2CReceiveEvent);
  Wire.onRequest(I2CRequestEvent);
  
  if ( nvs.extensionPort.interruptLine ) { 
    I2CSlave_read=false; 
    if (intIO) {
      intIO->setSpeed(nvs.extensionPort.interruptOnOff[0]);
      intIO->apply();      
    }
  }

}

SwOSI2C::SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t flags, uint8_t I2CAddress):SwOSIO( name, ctrl, SWOSIO_I2C, flags ) {

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

  if ( nvs.extensionPort.interruptLine ) { 
    // Use M1/M2 as interrupt line
    
    // reset read semaphore
    I2CSlave_read = false;

    // get MotorIO
    intIO = (SwOSMotor*) ctrl->getIO( SWOSIO_MOTOR, nvs.extensionPort.interruptLine - 1 + FTSWARM_M1 );

    if (intIO) {

      // if the remote controller didn't ack the last interrupt, so I need to reset the interupt line first 
      if ( intIO->getSpeed() != nvs.extensionPort.interruptOnOff[0] ) {
        intIO->setSpeed(nvs.extensionPort.interruptOnOff[0]);
        intIO->apply();
        delay(1);
      }

      // set interrupt
      intIO->setSpeed(nvs.extensionPort.interruptOnOff[1]);
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

/***************************************************
 *
 *   TWAI
 ***************************************************/

 /*
void SwOSTWAI::operate( ) {
  
}

void SwOSTWAI::setupLocal( void ) {

  Wire.begin(I2CAddress);
  Wire.onReceive(I2CReceiveEvent);
  Wire.onRequest(I2CRequestEvent);
  
  if ( nvs.extensionPort.interruptLine ) { 
    I2CSlave_read=false; 
    if (intIO) {
      intIO->setSpeed(nvs.extensionPort.interruptOnOff[0]);
      intIO->apply();      
    }
  }

}

SwOSI2C::SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t flags, uint8_t I2CAddress):SwOSIO( name, ctrl, SWOSIO_I2C, flags ) {

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

  if ( nvs.extensionPort.interruptLine ) { 
    // Use M1/M2 as interrupt line
    
    // reset read semaphore
    I2CSlave_read = false;

    // get MotorIO
    intIO = (SwOSMotor*) ctrl->getIO( SWOSIO_MOTOR, nvs.extensionPort.interruptLine - 1 + FTSWARM_M1 );

    if (intIO) {

      // if the remote controller didn't ack the last interrupt, so I need to reset the interupt line first 
      if ( intIO->getSpeed() != nvs.extensionPort.interruptOnOff[0] ) {
        intIO->setSpeed(nvs.extensionPort.interruptOnOff[0]);
        intIO->apply();
        delay(1);
      }

      // set interrupt
      intIO->setSpeed(nvs.extensionPort.interruptOnOff[1]);
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
  */