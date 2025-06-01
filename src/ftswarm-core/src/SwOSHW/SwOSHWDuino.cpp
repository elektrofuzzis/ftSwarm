/*
 * SwOSHWDuino.cpp
 *
 * ftDuino wrapper class
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <Wire.h>
#include <string.h>

#include "ftPwrDrive/i2cBuffer.h"
#include "SwOSHW/SWOSHWDuino.h"
#include "SwOSCom.h"

void SwOSDuino::setIOType( uint8_t port, SwOSIOType_t ioType ) {

    uint8_t mode;

    switch ( ioType ) {
        case SWOSIO_ANALOG:
        case SWOSIO_VOLTMETER:      mode = VOLTAGE;
                                    break;
                                    
        case SWOSIO_LDR:
        case SWOSIO_OHMMETER:
        case SWOSIO_THERMOMETER:    mode = RESISTANCE;
                                    break;

        default:                    mode = SWITCH;
                                    break;
    }

    i2c.sendData( i2cAddress, I2C_INPUT_SET_MODE, port, mode );
}


void SwOSDuino::setMotor( uint8_t port, FtSwarmMotion_t _motionType, int16_t speed) {

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

void SwOSDuino::read( void ) {

    i2c.sendData( i2cAddress, I2C_GETSTATE );
    i2c.receiveBuffer( i2cAddress, 16 );
    memcpy( input, i2c.data, 16 );

    if ( i2c.error ) error++;

}

uint8_t SwOSDuino::getError( void ) {
    return error;
}