/*
 * SwOSFilter.h
 *
 * Filter to correct analogous values
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 *
 */
 
 #pragma once

 #include <stdint.h>

#define FILTER_INVALID 0x8FFF

class SwOSFilter {

  protected:

    uint8_t    bufSize = 0;
    uint8_t    bufFill = 0;
    int16_t    *buffer = NULL;
    SwOSFilter *nextFilter = NULL;

    bool addBuffer( int16_t newValue );

  public:

    // constructor - allocates a buffer of bufSize int16_t
    SwOSFilter( uint8_t bufSize );

    // destructor
    ~SwOSFilter( );

    // function to run the filter
    virtual int16_t fx( int16_t newValue ) { return FILTER_INVALID; };

    // add a next filter to the list
    virtual void addFilter( SwOSFilter *nextFilter);

    void printBuffer( void );

};

class SwOSMovingAverage:public SwOSFilter {

  public:

    SwOSMovingAverage( uint8_t filterLen ):SwOSFilter( filterLen) {};

    // function to run the filter
    virtual int16_t fx( int16_t newValue );

};

// SwOSSpike - Filter to eliminate one-off small spikes
// maxNoise: max. noise on Signal, so it's not a spike
// maxSpike: if newValue > maxSpike, it's not a spike
// if a spike is detected, the return value is the average of the two last values

class SwOSSpike:public SwOSFilter {

  protected:

    uint8_t maxNoise;
    uint8_t maxSpike;

  public:

    // constructor
    SwOSSpike( uint8_t maxNoise, uint8_t maxSpike );

    // function to run the filter
    virtual int16_t fx( int16_t newValue );

};