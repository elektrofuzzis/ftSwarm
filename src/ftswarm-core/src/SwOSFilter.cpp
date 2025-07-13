/*
 * SwOSFilter.h
 *
 * Filter to correct analogous values
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SwOSFilter.h"

SwOSFilter::SwOSFilter( uint8_t bufSize ) {

    this->bufSize = bufSize;
    if ( bufSize > 0 ) buffer = (int32_t*) calloc( bufSize, sizeof(int32_t) );

}

SwOSFilter::~SwOSFilter( ) {

    if (buffer)     free( buffer );
    if (nextFilter) delete( nextFilter );

}

void SwOSFilter::addFilter( SwOSFilter *nextFilter) {

    if (this->nextFilter) this->nextFilter->addFilter( nextFilter );
    else this->nextFilter = nextFilter;

}

void SwOSFilter::printBuffer( void ) {
    
    printf("buffer =");
    for (uint8_t i=0; i<bufSize; i++) printf("%t%d", buffer[i]);
    printf("\n");
}

bool SwOSFilter::addBuffer( int32_t newValue ) {

    // shift buffer
    memcpy( &buffer[0], &buffer[1], sizeof(int32_t) * ( bufSize - 1) );

    // enter newest value
    buffer[bufSize-1] = newValue;
    if ( bufFill < bufSize ) bufFill++;

    return bufFill >= bufSize; // false if buffer isn't filled completely

}

int32_t SwOSMovingAverage::fx( int32_t newValue ) {

    // add new value to buffer, return invalid until buffer isn't filled completely
    if (!addBuffer( newValue ) ) return FILTER_INVALID;

    int32_t ma = 0;

    for (uint8_t i=0; i<bufSize; i++) {
        ma += buffer[i];
    }

    ma = ma / bufSize;

    if (nextFilter) return nextFilter->fx( ma );
    
    return ma;

}

SwOSSpike::SwOSSpike( uint8_t maxNoise, uint8_t maxSpike ):SwOSFilter(3){ 
    
    this->maxNoise = maxNoise; 
    this->maxSpike = maxSpike; 

};

int32_t SwOSSpike::fx( int32_t newValue ) {

    // add new value to buffer, return invalid until buffer isn't filled completely
    if (!addBuffer( newValue ) ) return FILTER_INVALID;

    int32_t d1 = abs( buffer[0] - buffer[1] );
    int32_t d2 = abs( buffer[1] - buffer[2] );

    int32_t nv = newValue;

    if ( (d1 <= maxNoise ) && ( d2 > maxNoise ) && ( d2 <= maxSpike ) ) nv = (buffer[0] + buffer[1])/2;

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSLinear::SwOSLinear( float a, int32_t b ):SwOSFilter(0){ 
    
    this->a = a;  
    this->b = b;

};

int32_t SwOSLinear::fx( int32_t newValue ) {

    float   nvf = newValue * a + b; 
    int32_t nv  = nvf;

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSMultiply::SwOSMultiply( float a ):SwOSFilter(0){ 
    
    this->a = a;  

};

int32_t SwOSMultiply::fx( int32_t newValue ) {

    float   nvf = newValue * a; 
    int32_t nv  = nvf;

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSAdd::SwOSAdd( int32_t a ):SwOSFilter(0){ 
    
    this->a = a;  

};

int32_t SwOSAdd::fx( int32_t newValue ) {

    int32_t nv = newValue + a;

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSMinMax::SwOSMinMax( int32_t a, int32_t b ):SwOSFilter(0){ 
    
    this->a = a; 
    this->b = b; 

};

int32_t SwOSMinMax::fx( int32_t newValue ) {

    int32_t nv = newValue;

    if ( nv < a ) nv = a;
    if ( nv > b ) nv = b;
    
    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSFJoystick::SwOSFJoystick( int32_t minValue, int32_t midValue, int32_t maxValue ):SwOSFilter(0){ 
    
  this->minValue = minValue;
  this->midValue = midValue;
  this->maxValue = maxValue;

};


int32_t SwOSFJoystick::fx( int32_t newValue ) {

    int32_t nv;

    float counter, denominator, percentage;

    if ( newValue < midValue ) {
      counter     = newValue - minValue;
      denominator = midValue - minValue;
      percentage  = counter/denominator * 100;
      nv          = -100 + percentage;
      if (nv < -100 ) nv = -100;

    } else {
      counter     = newValue - midValue;
      denominator = maxValue - midValue;
      percentage  = counter/denominator * 100;
      nv          = percentage;
      if (nv > 100 ) nv = 100;

    }

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}