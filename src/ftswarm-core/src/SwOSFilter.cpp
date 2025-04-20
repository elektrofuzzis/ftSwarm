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
    buffer = (int16_t*) calloc( bufSize, sizeof(int16_t) );

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