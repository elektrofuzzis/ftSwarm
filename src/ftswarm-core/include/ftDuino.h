/*
 * ftDuino.h
 *
 * wrapper to communicate with ftDuino via I2C
 * 
 * (C) 2023 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOS.h"
#include "ftPwrDrive/i2cBuffer.h"
 
#pragma once

class FtDuino {

    private:
        i2cBuffer i2c;
        uint8_t i2cAddress = 0x20;

        static const uint8_t RESISTANCE = 0, VOLTAGE = 1, SWITCH = 2;
        static const uint8_t OFF = 0, LEFT = 1, RIGHT = 2, BRAKE = 3;
        static const uint8_t I2C_MOTOR_SET = 0, I2C_INPUT_SET_MODE = 1 , I2C_GETSTATE = 2;

    public:
        void setSensorType( uint8_t port, FtSwarmSensor_t sensorType );
        void setMotor( uint8_t port, FtSwarmMotion_t _motionType, int16_t speed);
        void getState( uint16_t input[8] );
        uint8_t getError( void );

};