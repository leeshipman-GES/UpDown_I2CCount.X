/* 
 * File:   kypd_drvr.h
 * Author: lees
 *
 * Created on May 18, 2017, 12:57 PM
 */

#ifndef KYPD_DRVR_H
#define	KYPD_DRVR_H

#include <xc.h>

// Get easy access to hardware bits corresponding to the
#define COL4 PORTEbits.RE0   
#define COL3 PORTEbits.RE1   
#define COL2 PORTEbits.RE2   
#define COL1 PORTEbits.RE3
#define ROW4 PORTEbits.RE4
#define ROW3 PORTEbits.RE5
#define ROW2 PORTEbits.RE6
#define ROW1 PORTEbits.RE7


#ifdef	__cplusplus
extern "C" {
#endif

void kypd_setup();
void kypd_readSwitches();
void kypd_teardown();
char kypd_getKey();
 

#ifdef	__cplusplus
}
#endif

#endif	/* KYPD_DRVR_H */

