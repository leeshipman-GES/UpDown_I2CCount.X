/*
 * globaldata.h
 *
 *  Created on: Jan 31, 2017
 *      Author: lee.shipman
 */

#ifndef SOURCES_GLOBAL_H_
#define SOURCES_GLOBAL_H_

#include <stdint.h> // need data types
#include "lgs_logevents.h"

// This global data is allocated in global.c


// Right now the currentTime should be the number of 100 usec ticks that have occurred since the last reset.
// The timer object right now is only used for debug purposes.  Should
extern uint32_t currentTime;

// Global Functions, located in global.c
uint32_t getCurrentTime();
void incrementCurrentTime();

#endif /* SOURCES_GLOBAL_H_ */
