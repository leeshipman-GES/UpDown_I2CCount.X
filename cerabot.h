/* 
 * File:   cerabot.h
 * Author: lees
 *
 * Created on May 15, 2017, 10:14 AM
 * This file contains some of the CEREBot Specific #defines that my project use.
 */

#ifndef CERABOT_H
#define	CERABOT_H

#ifdef	__cplusplus
extern "C" {
#endif

#define BTN1 (PORTAbits.RA6)        
#define BTN2 (PORTAbits.RA7)
#define LED1 (1<<10)    // RB10
#define LED2 (1<<11)    // RB11
#define LED3 (1<<12)    // RB12
#define LED4 (1<<13)    // RB13



#ifdef	__cplusplus
}
#endif

#endif	/* CERABOT_H */

