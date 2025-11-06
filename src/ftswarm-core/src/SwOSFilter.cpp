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
    if ( bufSize > 0 ) buffer = (int16_t*) calloc( bufSize, sizeof(int16_t) );

}

SwOSFilter::~SwOSFilter( ) {

    if (buffer)     free( buffer );
    if (nextFilter) delete( nextFilter );

}

void SwOSFilter::addFilter( SwOSFilter *nextFilter) {

    if (this->nextFilter) this->nextFilter->addFilter( nextFilter );
    else this->nextFilter = nextFilter;

}

void SwOSFilter::deleteFilter( SwOSFilter_t filterType) {

  while ( nextFilter ) {

    if ( nextFilter->getType() == filterType ) {
      SwOSFilter *obsolete = nextFilter;
      nextFilter = nextFilter->nextFilter;
      obsolete->nextFilter = NULL;
      delete obsolete;

    } else {
      nextFilter = nextFilter->nextFilter;
    }

  }

}

void SwOSFilter::printBuffer( void ) {
    
    printf("buffer =");
    for (uint8_t i=0; i<bufSize; i++) printf("%t%d", buffer[i]);
    printf("\n");
}

bool SwOSFilter::addBuffer( int16_t newValue ) {

    // shift buffer
    memcpy( &buffer[0], &buffer[1], sizeof(int16_t) * ( bufSize - 1) );

    // enter newest value
    buffer[bufSize-1] = newValue;
    if ( bufFill < bufSize ) bufFill++;

    return bufFill >= bufSize; // false if buffer isn't filled completely

}

int16_t SwOSMovingAverage::fx( int16_t newValue ) {

    // add new value to buffer, return invalid until buffer isn't filled completely
    if (!addBuffer( newValue ) ) return FILTER_INVALID;

    int16_t ma = 0;

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

int16_t SwOSSpike::fx( int16_t newValue ) {

    // add new value to buffer, return invalid until buffer isn't filled completely
    if (!addBuffer( newValue ) ) return FILTER_INVALID;

    int16_t d1 = abs( buffer[0] - buffer[1] );
    int16_t d2 = abs( buffer[1] - buffer[2] );

    int16_t nv = newValue;

    if ( (d1 <= maxNoise ) && ( d2 > maxNoise ) && ( d2 <= maxSpike ) ) nv = (buffer[0] + buffer[1])/2;

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSLinear::SwOSLinear( float a, int16_t b ):SwOSFilter(0){ 
    
    this->a = a;  
    this->b = b;

};

int16_t SwOSLinear::fx( int16_t newValue ) {

    if ( newValue == FILTER_INVALID ) return FILTER_INVALID;

    float   nvf = newValue * a + b; 
    int16_t nv  = nvf;

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSMultiply::SwOSMultiply( float a ):SwOSFilter(0){ 
    
    this->a = a;  

};

int16_t SwOSMultiply::fx( int16_t newValue ) {

    if ( newValue == FILTER_INVALID ) return FILTER_INVALID;

    float   nvf = newValue * a; 
    int16_t nv  = nvf;

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSAdd::SwOSAdd( int16_t a ):SwOSFilter(0){ 
    
    this->a = a;  

};

int16_t SwOSAdd::fx( int16_t newValue ) {

    if ( newValue == FILTER_INVALID ) return FILTER_INVALID;

    int16_t nv = newValue + a;

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSMinMax::SwOSMinMax( int16_t a, int16_t b ):SwOSFilter(0){ 
    
    this->a = a; 
    this->b = b; 

};

int16_t SwOSMinMax::fx( int16_t newValue ) {

    if ( newValue == FILTER_INVALID ) return FILTER_INVALID;

    int16_t nv = newValue;

    if ( nv < a ) nv = a;
    if ( nv > b ) nv = b;
    
    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}

SwOSFJoystick::SwOSFJoystick( int16_t minValue, int16_t midValue, int16_t maxValue ):SwOSFilter(0){ 
    
  this->minValue = minValue;
  this->midValue = midValue;
  this->maxValue = maxValue;

};


int16_t SwOSFJoystick::fx( int16_t newValue ) {

    if ( newValue == FILTER_INVALID ) return FILTER_INVALID;

    int16_t nv;
    int16_t offset = 40; 

    float counter, denominator, percentage;

    if ( abs( newValue - midValue ) < offset ) {
      nv = 0;

    } else if ( newValue < ( midValue - offset ) ) {
      counter     = newValue - minValue;
      denominator = ( midValue - offset) - minValue;
      percentage  = counter/denominator * 100;
      nv          = -100 + percentage;
      if (nv < -100 ) nv = -100;

    } else  {
      counter     = newValue - (midValue + offset);
      denominator = maxValue - (midValue + offset);
      percentage  = counter/denominator * 100;
      nv          = percentage;
      if (nv > 100 ) nv = 100;

    }

    if (nextFilter) return nextFilter->fx( nv );
    
    return nv;
    
}