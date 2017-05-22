/* 
 * File:   main.c
 * Author: leez
 *
 * Created on April 19, 2017, 1:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
//#include <xc.h>
#include <proc/p32mx460f512l.h>
#include <sys/attribs.h>
#include "lgs_logevents.h"
#include "cerabot.h"
#include "i2c.h"
#include "kypd_drvr.h"

// Clock Configuration - Taken from the other board's notes
// Oscillator Settings
#pragma config FNOSC        = PRIPLL
#pragma config POSCMOD      = EC
#pragma config FPLLIDIV     = DIV_2
#pragma config FPLLMUL      = MUL_20
#pragma config FPLLODIV     = DIV_1
#pragma config FPBDIV       = DIV_8
#pragma config FSOSCEN      = OFF

//  Clock Control
#pragma config IESO         = OFF
#pragma config FCKSM        = CSDCMD
#pragma config OSCIOFNC     = OFF



// Create macros for each reading and writing

#define INITIAL_DELAY (100) // MS
#define DELAY_DELTA (5)
#define DELAY_MAX (3*INITIAL_DELAY)

#define INCREASE (0)
#define DECREASE (!INCREASE)

typedef enum LEDUpdateState
{
    normal,
    alwaysOff,
    alwaysOn,
} LEDUpdateState_t;

int currentDelay = INITIAL_DELAY;

#define INT_SKIP_DELAY 0x0020
int skipDelay = INT_SKIP_DELAY;
int delayCount = 0; // used by ISR to precisely time delays
int loopCount = 0;
int done = 0;  // Set to 0 if both switches are held down.


void adjustDelay(int direction)
{
    switch (direction)
    {
        case INCREASE:
           currentDelay += DELAY_DELTA;
           if (currentDelay > DELAY_MAX)
               currentDelay = DELAY_MAX;
           break;
            
        case DECREASE:
           currentDelay -= DELAY_DELTA/2;
           if (currentDelay < DELAY_DELTA)
               currentDelay = 0;    // Should result in all on.
           break;
    }
}

void setupTimer(void)
{
    T1CONbits.w = 0;
    T1CONbits.TCS = 0;
    T1CONbits.TCKPS = 0b10; // 1:64 
    T1CONbits.ON = 1;
    PR1 = 0xFFFF;
}

void setupPorts(void)
{
    // LED Setup - Port B
    TRISBbits.TRISB10 = 0;
    TRISBbits.TRISB11 = 0;
    TRISBbits.TRISB12 = 0;
    TRISBbits.TRISB13 = 0;

    // Set the corresponding bits.
    PORTBbits.RB10 = 1;
    PORTBbits.RB11 = 1;
    PORTBbits.RB12 = 1;
    PORTBbits.RB13 = 1; 
}

void updateLED(LEDUpdateState_t ledState, int whichLED )
{
    switch (ledState)
    {
        case alwaysOff:
            PORTBCLR = whichLED;
            break;
            
        case alwaysOn:
            PORTBSET = whichLED;
            break;
            
        case normal:
        default:
            PORTBINV = whichLED;
            break;
    } 
    
    // Do the commanded current delay
    //delay(currentDelay);
    delayxMS(currentDelay);
}

// Originally I wanted to use one of the CEREBOT's pins, but they are not 
// wired to a CN pin. At least it did not look like it when I checked.
void setUpInterrupts( void)
{
   asm volatile("DI");
  
    // Will use Timer 2 here   
   T2CONbits.TCKPS = 0b011;  // Prescaler 1:4
   TMR2 = 0;                // Initial Value
   PR2 = 0xFFFF;            // Ending Value
      
   IPC2bits.T2IS = 0x01;    //Sub Priority
   IPC2bits.T2IP = 0x04;    // Priority 4
   
   IFS0bits.T2IF = 0;       // Clear Flag if set
   IEC0bits.T2IE = 1;       // Enable Interrupt
   
   T2CONbits.ON = 1;        // Turn on the timer
    
    asm volatile("EI");
}

int isrCount = 0;
int gotKey = 0;
char key;

// Interrupt stuff in single vector mode.
void isrHandler(void) 
{
    LGS_LOG_EVENT_WITH_TIMESTAMP(0x11110000 | isrCount);
    
    // Check for the button
    if ( BTN1 == 1)
    {    
        adjustDelay(DECREASE);
    }

    if ( BTN2 == 1)
    {
        adjustDelay(INCREASE);
    } 

    if (BTN1 && BTN2)
    {
        IEC0bits.T2IE = 0; // Disable the interrupt
        done = 1;
    }
    
    // do keypad processing
    if (!gotKey)
    {

        kypd_setup();
        kypd_readSwitches();
        key = kypd_getKey();
        if (key != (char) 0xFF)
        {
            gotKey = 1;   // stop processing keys
            //delayxMS(20);
        }
        kypd_teardown();

    }
    
    isrCount++;
           
   IFS0bits.T2IF = 0; // Clear flag
}

// This is documented in sys/attribs.h  basically store the above
// fn in the vector table.
void __ISR_SINGLE__ isrHandler(void);

/*
 * 
 */

int main(int argc, char** argv) 
{    
    int i=0;
    int keyBuffIndex = 0;
    char saveKey = 0;

    unsigned short int numRuns = 0;
    unsigned int deadBeef = 0xDEADBEEF;
    
//    unsigned short int writreData;
    
    setupTimer();
    setupPorts();
    setUpInterrupts();
    drvI2CInit();
    
//    writeI2CToAddress(0x50, 0x00, (unsigned char *)&numRuns , sizeof(numRuns));

    readI2CMemoryFromAddress(0x50, 0x0000, (unsigned char *)&numRuns, sizeof(numRuns));
    readI2CMemoryFromAddress(0x50, 0x0010, (unsigned char *)&saveKey, sizeof(saveKey));
    currentDelay = saveKey * 5;

    
    //blink16Bits(numRuns,1);

    //numRuns = readI2CMemory();
        
    LEDUpdateState_t ledState = normal;
        
    PORTBbits.RB10 = 0;
    PORTBbits.RB11 = 0;
    PORTBbits.RB12 = 0;
    PORTBbits.RB13 = 0;
    
    while (!done)
    {   
//        LGS_LOG_EVENT_WITH_TIMESTAMP(0xDEAD);
//        LGS_LOG_EVENT(loopCount);
//        LGS_LOG_EVENT(currentDelay);
//        
        updateLED(ledState, LED1);
        updateLED(ledState, LED2);
        updateLED(ledState, LED3);
        updateLED(ledState, LED4);

        if (gotKey)
        {
            currentDelay = key * 5;
            //blink8Bits(key);
            saveKey = key;
            gotKey = 0;
            writeI2CToAddress(0x50, 0x0010,(unsigned char *)&saveKey,sizeof(saveKey));
        }
        
        // Update the LED State
        if (currentDelay > 0 && currentDelay < DELAY_MAX)
            ledState = normal;
        else
            if (currentDelay == 0)
                ledState = alwaysOn;
            else
                ledState = alwaysOff;   
        
    }
    
    // Clear LEDs
    PORTBbits.RB10 = 0;
    PORTBbits.RB11 = 0;
    PORTBbits.RB12 = 0;
    PORTBbits.RB13 = 0;
    
    numRuns++;    
    
    writeI2CToAddress(0x50, 0x0000, (unsigned char *)&numRuns , sizeof(numRuns));
    
    writeI2CToAddress(0x50, 0x0010,(unsigned char *)&saveKey,sizeof(saveKey));

    
//    unsigned int readBuffer;
//    pmodTempAndHumnity ( (unsigned char *) &readBuffer,  sizeof(readBuffer ));
 //   blink32Bits(readBuffer);   

   blink16Bits(numRuns);
    
   
    return (EXIT_SUCCESS);
}