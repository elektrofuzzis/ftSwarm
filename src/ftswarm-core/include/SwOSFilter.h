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

#define FILTER_INVALID 32767

// Filter types
typedef enum { 
  SWOS_FILTER_BASE, 
  SWOS_FILTER_MOVINGAVERAGE, 
  SWOS_FILTER_SPIKE, 
  SWOS_FILTER_LINEAR, 
  SWOS_FILTER_MULTIPLY, 
  SWOS_FILTER_ADD, 
  SWOS_FILTER_MINMAX,
  SWOS_FILTER_JOYSTICK
} SwOSFilter_t;

class SwOSFilter {

  protected:

    uint8_t    bufSize = 0;
    uint8_t    bufFill = 0;
    int32_t    *buffer = NULL;
    SwOSFilter *nextFilter = NULL;

    bool addBuffer( int32_t newValue );

  public:

    // constructor - allocates a buffer of bufSize int32_t
    SwOSFilter( uint8_t bufSize );

    // destructor
    ~SwOSFilter( );

    // my type
    virtual SwOSFilter_t getType( void ) { return SWOS_FILTER_BASE; };

    // function to run the filter
    virtual int32_t fx( int32_t newValue ) { return FILTER_INVALID; };

    // add a next filter to the list
    virtual void addFilter( SwOSFilter *nextFilter);

    void printBuffer( void );

};

class SwOSMovingAverage:public SwOSFilter {

  public:

    SwOSMovingAverage( uint8_t filterLen ):SwOSFilter( filterLen) {};

    // my type
    virtual SwOSFilter_t getType( void ) { return SWOS_FILTER_MOVINGAVERAGE; };

    // function to run the filter
    virtual int32_t fx( int32_t newValue );

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

    // my type
    virtual SwOSFilter_t getType( void ) { return SWOS_FILTER_SPIKE; };

    // function to run the filter
    virtual int32_t fx( int32_t newValue );

};

// f(x) = a*x+b

class SwOSLinear:public SwOSFilter {

  protected:

    float   a;
    int32_t b;

  public:

    // constructor
    SwOSLinear( float a, int32_t b  );

    // my type
    virtual SwOSFilter_t getType( void ) { return SWOS_FILTER_LINEAR; };

    // function to run the filter
    virtual int32_t fx( int32_t newValue );

};

// f(x) = a*x

class SwOSMultiply:public SwOSFilter {

  protected:

    float a;

  public:

    // constructor
    SwOSMultiply( float a  );

    // my type
    virtual SwOSFilter_t getType( void ) { return SWOS_FILTER_MULTIPLY; };

    // function to run the filter
    virtual int32_t fx( int32_t newValue );

};

// f(x) = x+b

class SwOSAdd:public SwOSFilter {

  protected:

    int32_t a;

  public:

    // constructor
    SwOSAdd( int32_t a );

    // my type
    virtual SwOSFilter_t getType( void ) { return SWOS_FILTER_ADD; };

    int32_t getConstant( void ) { return this->a; };
    void    setConstant( int32_t a ) { this->a = a; };

    // function to run the filter
    virtual int32_t fx( int32_t newValue );

};

// f(x) = max( b , min( a, x) )

class SwOSMinMax:public SwOSFilter {

  protected:

    int32_t a, b;

  public:

    // constructor
    SwOSMinMax( int32_t a, int32_t b );

    // my type
    virtual SwOSFilter_t getType( void ) { return SWOS_FILTER_MINMAX; };

    // function to run the filter
    virtual int32_t fx( int32_t newValue );

};

// f(x) = max( b , min( a, x) )

class SwOSFJoystick:public SwOSFilter {

  protected:

    int32_t minValue, midValue, maxValue; 

  public:

    // constructor
    SwOSFJoystick( int32_t minValue, int32_t midValue, int32_t maxValue );

    // my type
    virtual SwOSFilter_t getType( void ) { return SWOS_FILTER_JOYSTICK; };

    // function to run the filter
    virtual int32_t fx( int32_t newValue );

};