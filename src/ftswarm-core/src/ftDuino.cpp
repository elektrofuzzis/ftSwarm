#include <Wire.h>
#include <string.h>

#include "ftPwrDrive/i2cBuffer.h"
#include "ftDuino.h"
#include "SwOSCom.h"

void FtDuino::setSensorType( uint8_t port, FtSwarmSensor_t sensorType ) {

    uint8_t mode;

    switch (sensorType) {
        case FTSWARM_ANALOG:
        case FTSWARM_VOLTMETER:     mode = VOLTAGE;
                                    break;
                                    
        case FTSWARM_LDR:
        case FTSWARM_OHMMETER:
        case FTSWARM_THERMOMETER:   mode = RESISTANCE;
                                    break;

        default:                    mode = SWITCH;
                                    break;
    }

    i2c.sendData( i2cAddress, I2C_INPUT_SET_MODE, port, mode );
}


void FtDuino::setMotor( uint8_t port, FtSwarmMotion_t _motionType, int16_t speed) {

    // cast speed to pwm
    uint8_t pwm = ( abs( speed) > 255 ) ? 255 : abs( speed );
    
    // cast _motionType to mode
    uint8_t mode;
    switch ( _motionType ) {
        case FTSWARM_COAST: mode = OFF; break;
        case FTSWARM_BRAKE: mode = BRAKE; break;
        case FTSWARM_ON:    mode = ( speed > 0 ) ? LEFT : RIGHT; break;
    }

    i2c.sendData( i2cAddress, I2C_MOTOR_SET, port, mode, pwm );
}

void FtDuino::getState( uint16_t input[8] ) {

    i2c.sendData( i2cAddress, I2C_GETSTATE );
    i2c.receiveBuffer( i2cAddress, 16 );
    memcpy( input, i2c.data, 16 );

}

uint8_t FtDuino::getError( void ) {
    return i2c.error;
}