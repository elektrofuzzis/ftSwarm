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
 
 /***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

 SwOSGyro::SwOSGyro(const char *name, SwOSCtrl *ctrl, FtSwarmGyroMode_t gyroMode ) : SwOSIO( name, ctrl ) {

  if (ctrl->isLocal() ) {

    if (gyroMode == FTSWARM_GYRO_LSM6 )    _setupLocalLSM();
    if (gyroMode == FTSWARM_GYRO_MPU6050 ) _setupLocalMPU();
  
  }

}

SwOSGyro::~SwOSGyro( ) {

  if ( lsm ) delete lsm;
  if ( mpu ) delete mpu;

}

void SwOSGyro::_setupLocalLSM() {

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

void SwOSGyro::_readLSM() {

  lsm->Get_X_Axes(_accelerometer);
  lsm->Get_G_Axes(_gyroscope);

}

void SwOSGyro::_setupLocalMPU() {

  mpu = new MPU6050();
  mpu->initialize();

  if(mpu->testConnection() == false){
    ESP_LOGE( LOGFTSWARM, "MPU6050 connection failed." );
    _ctrl->setState( ERROR );
    return;
  }

  // Initializate and configure the DMP
  uint8_t devStatus = mpu->dmpInitialize();

  // Supply your gyro offsets here, scaled for min sensitivity
  mpu->setXGyroOffset(0);
  mpu->setYGyroOffset(0);
  mpu->setZGyroOffset(0);
  mpu->setXAccelOffset(0);
  mpu->setYAccelOffset(0);
  mpu->setZAccelOffset(0);

  // Making sure it worked (returns 0 if so)
  if (devStatus != 0) {
    ESP_LOGE( LOGFTSWARM, "MPU6050 DMP initialisation failed." );
    _ctrl->setState( ERROR );
    return;  
  }
  
  mpu->CalibrateAccel(6);  // Calibration Time: generate offsets and calibrate our MPU6050
  mpu->CalibrateGyro(6);
  printf("These are the Active offsets:\n");
  mpu->PrintActiveOffsets();
  printf("Enabling DMP...\n");   //Turning ON DMP
  mpu->setDMPEnabled(true);

}

void SwOSGyro::_readMPU() {

}

void SwOSGyro::read() {

  if (lsm) _readLSM();
  if (mpu) _readMPU();

}

void SwOSGyro::jsonize( JSONize *json, uint8_t id) {
  
}

/***************************************************
 *  
 * SwOSLidarInput
 *
 ***************************************************/

 VL53L0X Lidar;

 SwOSLidarInput::SwOSLidarInput(const char *name, uint8_t port, SwOSCtrl *ctrl ) : SwOSInput( name, port, ctrl, FTSWARM_DIGITAL ) {
   
   // initialize local HW
   if (_ctrl->isLocal()) {
       _setupLocal();
   }
 
 }
 
 void SwOSLidarInput::_setupLocal() {
   // initialize local HW
 
   SwOSInput::_setupLocal( );
 
   Lidar.setTimeout(500);
   Lidar.init();
   // Lidar.setMeasurementTimingBudget(20000);
   Lidar.startContinuous(100);
 }
 
 void SwOSLidarInput::setSensorType( FtSwarmSensor_t sensorType ) {
 
   // due to send normallyOpen to remote controllers, don't call super class
 
   _sensorType   = sensorType;
 
   if (_ctrl->isLocal()) { 
     
     setSensorTypeLocal( sensorType );
 
   } else {
 
     // send SN, SETSENSORTYPE, _port, sensorType
     SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSENSORTYPE );
     cmd.data.sensorCmd.index        = _port;
     cmd.data.sensorCmd.sensorType   = _sensorType;
     cmd.send( );
 
   }
 
 }
 
 void SwOSLidarInput::setSensorTypeLocal( FtSwarmSensor_t sensorType ) {
 
 }
 
 void SwOSLidarInput::read() {
   
   // nothing todo on remote sensors
   if (!_ctrl->isLocal()) return;
 
   uint32_t newValue;
 
   // read new data
   newValue = Lidar.readRangeContinuousMillimeters();
 
   setReading( newValue );
 
 }
 
 void SwOSLidarInput::setReading( int32_t newValue ) {
     
   // store new data
   _lastRawValue = newValue;  
 
   subscription();
 
 }
 
 void SwOSLidarInput::setValue( int32_t value ) {
 
   // nothing ToDo on real local HW
   if ( ( _ctrl->isLocal()) && (!_ctrl->isI2CSwarmCtrl() ) ) return;
   
   _lastRawValue = value;
 
   subscription();
 
 }
 
 void SwOSLidarInput::jsonize( JSONize *json, uint8_t id) {
   json->startObject();
   SwOSIO::jsonize(json, id);
   json->variableUI32("sensorType", _sensorType);
   json->variable("subType",    (char *) SENSORTYPE[_sensorType]);
 
   json->variableI32("value", getValueI32() );
   
   json->endObject();
 }

 /***************************************************
 *
 *   I2C Slave
 *
 ***************************************************/

/*
#define I2C_DATA_LENGTH 512                         -- Data buffer length of test buffer
#define I2C_SLAVE_TX_BUF_LEN (2 * I2C_DATA_LENGTH)  -- I2C slave tx buffer size
#define I2C_SLAVE_RX_BUF_LEN (2 * I2C_DATA_LENGTH)  -- I2C slave rx buffer size
#define I2C_SLAVE_NUM I2C_NUM_1

void SwOSI2C::read(  ) {
 
  int bytes_read = 0;
  uint8_t data[4];
  
  bytes_read = i2c_slave_read_buffer(I2C_SLAVE_NUM, data, 4, portMAX_DELAY);

  if ( bytes_read == 1 ) {
    // read a register
    uint8_t result = 0;
    if (data[0]<MAXI2CREGISTERS) result = myRegister[data[0]];
    i2c_reset_tx_fifo(I2C_SLAVE_NUM);
    i2c_slave_write_buffer(I2C_SLAVE_NUM, &result, 1, portMAX_DELAY);
      
  } else if ( bytes_read == 2 ) {
    // write a register
    if (data[0]<MAXI2CREGISTERS) {
      myRegister[data[0]] = data[1];
    }
  }

  if ( bytes_read > 0 ) {
    trigger( FTSWARM_TRIGGERI2CREAD, 0 );
  }

}

void SwOSI2C::_setupLocal(uint8_t I2CAddress) {

  int i2c_slave_port = I2C_SLAVE_NUM;
  i2c_config_t conf_slave;

  conf_slave.sda_pullup_en       = GPIO_PULLUP_ENABLE;
  conf_slave.scl_pullup_en       = GPIO_PULLUP_ENABLE;
  conf_slave.mode                = I2C_MODE_SLAVE;
  conf_slave.slave.addr_10bit_en = 0;
  conf_slave.slave.slave_addr    = I2CAddress;        
  conf_slave.slave.maximum_speed = 400000;
  conf_slave.clk_flags           = 0;

  switch ( _ctrl->getCPU() ) {
    case FTSWARMRS_2V1:
      // ftSwarmRS final
      conf_slave.sda_io_num = GPIO_NUM_8;
      conf_slave.scl_io_num = GPIO_NUM_9;
      break;
    case FTSWARMRS_2V0:
      // ftSwarmRS 
      conf_slave.sda_io_num = GPIO_NUM_4;
      conf_slave.scl_io_num = GPIO_NUM_5;
      break;
    default:
      // ftSwarm & ftSwarmControl 
      conf_slave.sda_io_num = GPIO_NUM_21;
      conf_slave.scl_io_num = xGPIO_NUM_22;
      break;
    }
    

    i2c_param_config(i2c_slave_port, &conf_slave);
    i2c_driver_install(i2c_slave_port,  I2C_MODE_SLAVE, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);

}


SwOSI2C::SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t I2CAddress):SwOSIO( name, ctrl ) {

  memset(myRegister, 0, sizeof(myRegister));
  
  if (ctrl->isLocal()) _setupLocal(I2CAddress);

}

void SwOSI2C::setRegister( uint8_t reg, uint8_t value ) {

  // check on boundaries
  if (reg>=MAXI2CREGISTERS) return;
  
  myRegister[reg] = value;

  if (_ctrl->isLocal()) _setLocal( reg, value );
  else                  _setRemote( reg, value );

}

void SwOSI2C::_setRemote( uint8_t reg, uint8_t value ) {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_I2CREGISTER );
  cmd.data.I2CRegisterCmd.reg   = reg;
  cmd.data.I2CRegisterCmd.value = value;
  cmd.send( );
}

void SwOSI2C::_setLocal( uint8_t reg, uint8_t value ) {
  myRegister[reg] = value;
  trigger( FTSWARM_TRIGGERI2CWRITE, 0 );
}

uint8_t SwOSI2C::getRegister( uint8_t reg ) {

  if (reg>=MAXI2CREGISTERS) return 0;
  else return myRegister[reg];

} */

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
    _ctrl->actor[nvs.interruptLine-1]->setSpeed(nvs.interruptOnOff[0]);
    _ctrl->actor[nvs.interruptLine-1]->apply();
  }

}

void SwOSI2C::_setupLocal(uint8_t I2CAddress) {

  Wire.begin(I2CAddress);
  Wire.onReceive(I2CReceiveEvent);
  Wire.onRequest(I2CRequestEvent);
  
  if ( nvs.interruptLine ) { 
    I2CSlave_read=false; 
    _ctrl->actor[nvs.interruptLine-1]->setSpeed( nvs.interruptOnOff[0] );
    _ctrl->actor[nvs.interruptLine-1]->apply();
  }

}

SwOSI2C::SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t I2CAddress):SwOSIO( name, ctrl ) {

  memset(myRegister, 0, sizeof(myRegister));
  
  if (ctrl->isLocal()) _setupLocal(I2CAddress);

}

void SwOSI2C::setRegister( uint8_t reg, uint8_t value ) {

  // check on boundaries
  if (reg>=MAXI2CREGISTERS) return;
  
  myRegister[reg] = value;

  if (_ctrl->isLocal()) _setLocal( reg, value );
  else                  _setRemote( reg, value );

}

void SwOSI2C::_setRemote( uint8_t reg, uint8_t value ) {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_I2CREGISTER );
  cmd.data.I2CRegisterCmd.reg   = reg;
  cmd.data.I2CRegisterCmd.value = value;
  cmd.send( );
}

void SwOSI2C::_setLocal( uint8_t reg, uint8_t value ) {

  I2CSlave_value[reg] = value;

  if ( nvs.interruptLine ) { 
    // Use M1/M2 as interrupt line
    
    // reset read semaphore
    I2CSlave_read = false;

    // if the remote controller didn't ack the last interrupt, so I need to reset the interupt line first 
    if ( _ctrl->actor[nvs.interruptLine-1]->getSpeed() != nvs.interruptOnOff[0] ) {
      _ctrl->actor[nvs.interruptLine-1]->setSpeed( nvs.interruptOnOff[0] );
      _ctrl->actor[nvs.interruptLine-1]->apply();
      delay(1);
    }

    // set interrupt
    _ctrl->actor[nvs.interruptLine-1]->setSpeed( nvs.interruptOnOff[1] );
    _ctrl->actor[nvs.interruptLine-1]->apply();

  }

}

uint8_t SwOSI2C::getRegister( uint8_t reg ) {

  if (reg>=MAXI2CREGISTERS) return 0;
  else return myRegister[reg];

}