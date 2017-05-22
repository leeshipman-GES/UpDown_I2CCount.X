/* 
 * File:   LED_Display.h
 * Author: lees
 *
 * Created on May 15, 2017, 10:15 AM
 */

#ifndef LED_DISPLAY_H
#define	LED_DISPLAY_H

#ifdef	__cplusplus
extern "C" {
#endif

    void blink8Bits(unsigned char byte);
    void blink16Bits(unsigned short int word);
    void blink8Bits(unsigned int long word);
    void delayxMS( int msecToDelay);

#ifdef	__cplusplus
}
#endif

#endif	/* LED_DISPLAY_H */

