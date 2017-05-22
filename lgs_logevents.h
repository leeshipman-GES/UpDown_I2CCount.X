/*
 * lgs_logevents.h
 *
 *  Created on: Feb 16, 2017
 *      Author: lee.shipman
 */

#ifndef SOURCES_LGS_LOGEVENTS_H_
#define SOURCES_LGS_LOGEVENTS_H_



// Debug Support  Simply define LGS_LOG_EVENTS and you can call either of these functions.
#define LGS_LOG_EVENTS

#define LGS_LOG_SIZE (1<<10)
#define LGS_LOG_INDEX_MASK (LGS_LOG_SIZE-1)

#ifdef LGS_LOG_EVENTS
extern unsigned long int savedTimeEvents[LGS_LOG_SIZE];
extern unsigned short int  timeEventIndx;

// Function to capture a 32 bit value.
#define LGS_LOG_EVENT(event) savedTimeEvents[ (timeEventIndx++) & LGS_LOG_INDEX_MASK] = (event)

// Function to get the elapsed time, and then capture a 32 bit value
#define LGS_LOG_EVENT_WITH_TIMESTAMP(event) savedTimeEvents[  (timeEventIndx++) & LGS_LOG_INDEX_MASK]=getCurrentTime(); savedTimeEvents[  (timeEventIndx++) & LGS_LOG_INDEX_MASK] = (event)

// Just log the time value
#define LGS_LOG_TIMESTAMP() savedTimeEvents[  (timeEventIndx++) & LGS_LOG_INDEX_MASK]=getCurrentTime()

#else
// Define NOP versions of functions
#define LGS_LOG_EVENT(event)
#define LGS_LOG_EVENT_WITH_TIMESTAMP(event)
#define LGS_LOG_TIMESTAMP()
#endif



#endif /* SOURCES_LGS_LOGEVENTS_H_ */
