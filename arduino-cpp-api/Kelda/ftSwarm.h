////////////////////////////////////////////////////
//
// ftSwarm.h
//
// common defintions for all classes
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
///////////////////////////////////////////////////

#ifndef FTSWARM_H
#define FTSWARM_H

#define IO0   0
#define IO2   2
#define IO4   4
#define IO5   5
#define IO14 14
#define IO18 18
#define IO19 19
#define IO23 23
#define IO25 25
#define IO26 26
#define IO27 27
#define IO33 33

#define SPI_CLK  14
#define SPI_MISO 12
#define SPI_MOSI 13

enum ftSwarmError { FTSWARM_OK, NO_PORT, UNKNOWN_CMD };

#endif
