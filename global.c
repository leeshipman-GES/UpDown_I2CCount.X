/*
 * global.c
 *
 *  Created on: Jan 31, 2017
 *      Author: lee.shipman
 */

#include <proc/p32mx460f512l.h>
#include "global.h"


// Global Data
uint32_t currentTime;


// Global behavior

// Time Stuff
uint32_t getCurrentTime()
{
	return TMR1;
//    return currentTime;
}

void incrementCurrentTime()
{
	currentTime++;
}

