/*
 * lgs_logevents.c
 *
 *  Created on: Feb 16, 2017
 *      Author: lee.shipman
 */

#include "lgs_logevents.h"

// Debug stuff, will eventually go away if we need space.
#ifdef LGS_LOG_EVENTS
unsigned long int 	savedTimeEvents[ LGS_LOG_SIZE] = {0};
unsigned short int	timeEventIndx = 0;
#endif
