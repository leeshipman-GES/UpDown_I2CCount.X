/* 
 * File:   LED_Display.c
 * Author: lees
 *
 * Created on May 15, 2017, 10:15 AM
 */

// This file a couple of routines which allow one to blink out numbers
// using the four LEDs on the CEREBOT board.  With not to much effort,
// I think this can be ported to a different platform.

#include <xc.h>
#include "cerabot.h"

#define ZIP_TIME 20 
#define INTER_NIBBLE_DELAY 500
#define INTER_BYTE_DELAY 200
#define NON_ZIP_READ 2000
#define ZIP_READ 900


// Byte - Nibble Display
// MSB                LSB
// LED4, LED3, LED2, LED1
static unsigned short LEDMap[ 16] = //0-F
{
    0,                                  //0
    LED1,                               //1
    LED2,                               //2
    (LED2) | (LED1),                    //3
    LED3,                               //4
    (LED3) | (LED1),                    //5
    (LED3) | (LED2),                    //6
    (LED3) | (LED2) | (LED1),           //7
    (LED4),                             //8
    (LED4) | (LED1),                    //9
    (LED4) | (LED2),                    //A
    (LED4) | (LED2) | (LED1),           //B
    (LED4) | (LED3),                    //C
    (LED4) | (LED3) | (LED1),           //D
    (LED4) | (LED3) | (LED2),           //E
    (LED4) | (LED3) | (LED2) | (LED1),  //F
};

/*
 * Function to delay x number of milliseconds.  Pass in the number of 
 * milliseconds to delay.
 * 
 * First implementation is pretty crude. May want to make more accurate
 * should the need arise in the future.
 */
void delayxMS( int msecToDelay)
{
    int i;
    unsigned int stopTime;
    
    for (i = 0; i < msecToDelay; i++)
    {
        stopTime = (unsigned int) TMR1 + 156; //1ms @1:64, 2170 @ 1:1 
        if (stopTime > 0x0FFFF)
        {
            // Overflow condition
            short int leftOver = stopTime >> 16;
            while ( (signed short int) TMR1 < leftOver)
                ;
        }
        else
        {
            // should be OK
            while ( (unsigned int) TMR1 < stopTime)
                ;
        }
    } 
}

// Zip display zip (zero) since.  Since you really cant see zero on the
// LEDs, zip will quickly sequence from LSB to MSB indicating a zero
// is in its place.  Note that this is also used to display a break in 
// the data.
static void zip()
{
    int i;
    for (i=0;i < 4;i++)
    {
        PORTBSET = 1 << (10 + i);
        delayxMS(ZIP_TIME);
        PORTBCLR = 1 << (10+i);
    }        
    
}

// BLinks out 8 bits, MSB (nibble) followed by the LSB (nibble))
// byte is the byte to blink out
// Delay, if set to non zero will cause a 1 second delay before
// starting.
void blink8Bits (unsigned char byte, int delay)
{
    int bitsToLight;
    int i;
    
 
              // MSB                       LSB
    PORTBCLR = (LED4) | (LED3) | (LED2) | (LED1);
 
    // Delay if this was called all by itself
    if (delay)
        delayxMS(INTER_NIBBLE_DELAY);

    // Light the MSB
    bitsToLight = LEDMap[ (byte & 0xF0) >> 4];
    if (bitsToLight)
    {
        PORTBSET = bitsToLight;        
        delayxMS(NON_ZIP_READ);
    }
    else // for zero
    {
        delayxMS(ZIP_READ);
        zip();
        delayxMS(ZIP_READ);
    }
    
    // clear those bits
    PORTBCLR = (LED4) | (LED3) | (LED2) | (LED1);
    delayxMS(INTER_NIBBLE_DELAY); // small delay
    
    // Light the LSB
    bitsToLight = LEDMap[ byte & 0x0F];
    if (bitsToLight)
    {
        PORTBSET = bitsToLight;        
        delayxMS(NON_ZIP_READ);
    }
    else // for zero
    {
        delayxMS(ZIP_READ);
        zip();
        delayxMS(ZIP_READ);
    }
    
    // Again clear.
    PORTBCLR = (LED4) | (LED3) | (LED2) | (LED1);    
 }

// BLinks out 16 bits, MSB (byte) followed by the LSB (byte))
// byte is the byte to blink out.  Note this uses blink8bits())
// Delay, if set to non zero will cause a 1 second delay before
// starting.
// word, short word to display
// Delay, if set to non zero will cause a 1 second delay before
// starting.
void blink16Bits( unsigned short int word, int delay)
{
    if (delay)
        delayxMS(INTER_NIBBLE_DELAY);

    // blink8Bits MSB byte passing delay parameter is always noDelay =0
    blink8Bits(word >> 8,0);

    // Delineate between each byte
    delayxMS(INTER_NIBBLE_DELAY);
    zip();
    zip();
    zip();
    delayxMS(INTER_NIBBLE_DELAY);

    // blink8Bits LSB byte passing delay parameter is always nonDelay = 0
    blink8Bits(word & 0x00FF,0);
}

// BLinks out 32 bits, MSB (nibble) followed by the LSB (nibble))
// Note this uses blink16bits()
// byte is the long word to blink out.  
// 
void blink32Bits( unsigned int longWord)
{
    // blink16Bits msb word
    delayxMS(INTER_NIBBLE_DELAY);
    blink16Bits( longWord >> 16,0);
    
    // Break up words
    delayxMS(INTER_BYTE_DELAY);
    zip();
    zip();
    zip();
    zip();
    zip();
    zip();
    delayxMS(INTER_BYTE_DELAY);

    // blink16Bits lsb word 
    blink16Bits(longWord & 0x0FFFF,0);
}
