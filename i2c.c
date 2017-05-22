
// The following I2C code was retrieved by googleing for simple i2c driver for pic32
// by Govind Mukundan
// My Implementation is based on his code.

#include <xc.h>
#include "i2c.h"
#include "lgs_logevents.h"  // For Debug purposes only

#define READ_OP (1)
#define WRITE_OP (0)

// Local Functions
static void I2CIdle(void);
static int I2CStart(void);
static void I2CStop(void);
static int I2CSendByte(unsigned char data);

// Init Function.  This only has to happen one time per reboot.
void drvI2CInit(void) {

    unsigned short int temp = 0;

    I2CCON = 0; // reset bits to 0
    I2CCONbits.I2CEN = 0; // disable module
    //I2CBRG = (GetPeripheralClock() / FCL) - (GetPeripheralClock() / 10000000) - 1; //Formula from datasheet

    I2CBRG = 0x0A; // 400 Khz

    //I2CBRG = ((SSMGetCurrentPeripheralClock() / (2 * Fsck)) - 1);

    I2CSTAT = 0;
    I2CCONbits.I2CSIDL = 1; // dont operate in idle mode
    I2CCONbits.I2CEN = 1; // enable module
    temp = I2CRCV; // clear RBF flag
}


// Function which is used pretty much between each transferrance of micro and peripheral via I2C
static void I2CIdle(void) {

    unsigned int t = 255*8; // Was originally just 255, needed to wait longer, not sure why

    LGS_LOG_EVENT_WITH_TIMESTAMP(0x5555);

    /* Wait until I2C Bus is Inactive */
    while (I2CCONbits.SEN || I2CCONbits.PEN || I2CCONbits.RCEN ||
            I2CCONbits.RSEN || I2CCONbits.ACKEN || I2CSTATbits.TRSTAT || t--);
    
    LGS_LOG_EVENT_WITH_TIMESTAMP(0x6666);
}

static int I2CStart(void) {

    I2CIdle();

    // Enable the Start condition
    I2CCONbits.SEN = 1;

    // Check for collisions
    if (I2CSTATbits.BCL) 
    {
        return (FALSE);
    } 
    else 
    {
        I2CIdle();
        return (TRUE);
    }
}

static void I2CStop(void) {

    int x = 0;
		
    // wait for module idle
    I2CIdle();
    
	//initiate stop bit
    I2CCONbits.PEN = 1;
    
    //wait for hardware clear of stop bit
    while (I2CCONbits.PEN) 
    {
        if (x++ > 50) 
            break;
    }
    
    I2CCONbits.RCEN = 0;
    I2CSTATbits.IWCOL = 0;
    I2CSTATbits.BCL = 0;
}

// Currently not used by me.
static int I2CSendByte(unsigned char data) 
{
    // Send the byte
    I2CTRN = data;

    // Check for collisions

    if ((I2CSTATbits.IWCOL == 1)) 
    {
        return (FALSE);

    } 
    else 
    {
        while (I2CSTATbits.TRSTAT); // wait until write cycle is complete

        if ((I2CSTATbits.BCL == 1)) 
        {
            return (FALSE);

        } 
        else 
        {
            // wait for module idle
            I2CIdle();
            return (TRUE);
        }
    }
}

// Tailed to use with the EEPROM on the CEREBOT board.
void readI2CMemoryFromAddress(unsigned short chipAddress, unsigned short int address, unsigned char * buffer, int numBytesToRead)
{
    int i;
	// 7 Bit addresses only
    char icAddr = (char) chipAddress & 0x00FF;

    I2CStart();
    
    I2CTRN = (icAddr << 1) | WRITE_OP;         // Address of EEPROM - Write      
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = address >> 8;;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = address & 0x00FF;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    
    I2CStart();
    
    I2CTRN = (icAddr << 1) | READ_OP;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();	// Might not need
    
    for (i=0; i< numBytesToRead; i++)
    {
        I2CIdle();

        I2CCONbits.RCEN = 1; // enable master read
        while (I2CCONbits.RCEN); // wait for byte to be received !(I2CSTATbits.RBF)
        I2CIdle();

        I2CSTATbits.I2COV = 0;
        I2CIdle();
        
        buffer[i] = I2C1RCV;
        
        if (i+1 == numBytesToRead)
        {
            I2CCONbits.ACKDT = 1; // send nack on last read
            I2CCONbits.ACKEN = 1;            
            I2CStop();
        }
        else
        {
            I2CCONbits.ACKDT = 0; // send ack if more data to get
            I2CCONbits.ACKEN = 1;
        }
    }
    return;    
}

void pmodTempAndHumnity (unsigned char *readBuffer, int readSize )
{ 
    int i;
    I2CStart();
    
#if 1
    // Set Configuration Register First
    I2CTRN = 0x80;  // Hard coded address (0x40<<1) for this chip  
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = 2; // Hard coded address
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();        
    
    // write config = 0x1000
    I2CTRN = 0x10;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();

    I2CTRN = 0x00;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
#endif
    
    // Now read the data.
    I2CTRN = 0x80;  // Hard coded address (0x40<<1) for this chip  
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = 0; // Hard coded address
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();        

    delayxMS(25); // requires hard delay.
    
    // Added this start / restart to try to fix repeated byte issue.
    I2C1CONbits.RSEN = 1;
    I2CIdle();
    
    I2CTRN = 0x81; // Hard coded address (0x40<<1) | 0x01) with read bit on.
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
     
 
    for (i=0; i< readSize; i++)
    {
        I2CCONbits.RCEN = 1; // enable master read
        while (I2CCONbits.RCEN); // wait for byte to be received !(I2CSTATbits.RBF)
        I2CIdle();

        I2CSTATbits.I2COV = 0;
        I2CIdle();
        
        readBuffer[i] = I2C1RCV;
        
        if (i+1 == readSize)
        {
            I2CCONbits.ACKDT = 1; // send nack on last read
            I2CCONbits.ACKEN = 1;            
            I2CStop();
        }
        else
        {
            I2CCONbits.ACKDT = 0; // send ack if more data to get
            I2CCONbits.ACKEN = 1;
        }
        I2CIdle();

    }
    return;
}

void writeI2CToAddress(unsigned short chipAddress,unsigned short int address, unsigned char * dataToWrite, int numBytesToWrite)
{
    int i;
    
    char icAddr = (char) chipAddress & 0x00FF;    

    // Set the start Bit
    I2CStart();
    
    I2CTRN = (icAddr << 1) | WRITE_OP;         // Address of EEPROM - Write      
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = address >> 8;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = address & 0x00FF;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    
    for (i=0; i<numBytesToWrite; i++)
    {
        I2CIdle();
        I2CTRN = dataToWrite[i];
    }
        
    I2CStop();
    
}

// Not really used right now.
void writeI2CMemory ( int byteToWrite)
{
    // Set the start Bit
    I2CStart();
    
    I2CTRN = 0x0A0;         // Address of EEPROM - Write      
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = 0x00;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = 0x00;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CTRN = (unsigned char) byteToWrite;
    while (I2CSTATbits.TRSTAT); // wait until write cycle is complete
    I2CIdle();
    
    I2CStop();
}

/*  The following is where I got my logic from */

/**

 * @brief Read data from an I2C slave

 *

 * This function can be used to read one or more sequential registers from a slave.

 * To read multiple registers, the slave must support multi-byte reads.

 * 

 * @param reg The register to start reading from (unsigned char)

 * @param rxPtr A pointer to where the read data should be stored (unsigned char*)

 * @param len Number of bytes/registers to read

 * @param slave_adr The 7 bit address of the slave without the R/W bits set

 * @return Boolean indicating if operation completed successfully or not

 */

#if 0
int drvI2CReadRegisters(unsigned short int adr, unsigned char* rxPtr, unsigned char len, unsigned char slave_adr) 
{
    unsigned char i, flag, ret, j;
    unsigned char addrMSB, addrLSB;

    flag = 0;
    ret = FALSE;

#if(I2C_DEBUG == 1)

    UART2PrintString("\r\n-----Start read reg:-----\r\n");

    UART2PutHex(reg);

    UART2PrintString("\r\n");

#endif

    for (i = 0; i < 100; i++) // wait for ACK for some time
    {

        //1. i2c start
        I2CStart();
        //2. Set Slave in W Mode
        I2CSendByte((slave_adr << 1) | 0);

        //3. Check ACK

        I2CIdle();

        if (I2CSTATbits.ACKSTAT == 0) // Did we receive an ACK ?

        {

            flag = 1;

            break;

        }

#if(I2C_DEBUG == 1)

        UART2PutChar('.');

#endif

    }




    if (!flag) return FALSE; // Exit if there was a problem
    
    I2CSendByte(addrMSB);
    
    I2CIdle();

    if (I2CSTATbits.ACKSTAT == 0) // Did we receive an ACK ?

    {

        flag = 1;

    }


    // 4.if write cmd was successful, put the regno on the bus
    if (!flag) return FALSE; // Exit if there was a problem

    I2CSendByte(addrLSB);
    I2CIdle();

    if (I2CSTATbits.ACKSTAT != 0) // Did we receive an ACK ?

    {

#if(I2C_DEBUG == 1)

        UART2PrintString("Error NACK Rxed\n");

#endif

        return FALSE; // Exit if there was a problem

    }

    // Now that the register addres is setup, we can ask the slave to enter read mode.

    for (j = 0; j < 100; j++) {

        //5.Issue a repeated start = a start cond without a prior stop

        I2CStart();

        //6. Set Slave in R mode

        I2CSendByte((slave_adr << 1) | 1);

        //7. Check ACK

        if (I2CSTATbits.ACKSTAT == 0) // Did we receive an ACK ?

        {

            flag = 1;

            break;

        }

#if(I2C_DEBUG == 1)

        UART2PutChar('.');

#endif

    }



    if (!flag) return FALSE; // Exit if there was a problem


    for (i = 0; i < len; i++) //read all the bytes

    {

        I2CIdle();

        // got the ack, processor is in read mode

        //8. read the register

        I2CCONbits.RCEN = 1; // enable master read

        while (I2CCONbits.RCEN); // wait for byte to be received !(I2CSTATbits.RBF)




        I2CIdle();

        I2CSTATbits.I2COV = 0;

        *(rxPtr + i) = I2CRCV;




        if ((i + 1) == len) {

            //9. Generate a NACK on last byte

            I2CCONbits.ACKDT = 1; // send nack

            I2CCONbits.ACKEN = 1;

            //10. generate a stop

            I2CStop();

        } else {

            I2CCONbits.ACKDT = 0; // send ACK for sequential reads

            I2CCONbits.ACKEN = 1;

        }




        ret = TRUE;

    }

#if(I2C_DEBUG == 1)

    for (i = 0; i < len; i++) {

        UART2PutChar('$');

        UART2PutHex(*(rxPtr + i));

    }

#endif




    return ret;

}
#endif



/**

 * @brief Write data into an I2C slave

 *

 * This function can be used to write one or more sequential registers from a slave.

 * To write multiple registers, the slave must support multi-byte writes.

 *

 * @param reg The register to start writing to (unsigned char)

 * @param rxPtr A pointer to where the data should be fetched from (unsigned char*)

 * @param len Number of bytes/registers to write

 * @param slave_adr The 7 bit address of the slave without the R/W bits set

 * 

 * @return Boolean indicating if operation completed successfully or not

 */

#if 0
int drvI2CWriteRegisters(unsigned short int adr, unsigned char* data, unsigned char len, unsigned char slave_adr) {

    unsigned  i, flag, j;
    unsigned  addrMSB, addrLSB;

    flag = 0;

    for (i = 0; i < 100; i++) {

        //1. i2c start

        I2CStart();

        //2. Set  in W Mode

        I2CSendByte(0x50 << 1);

        //3. Check ACK

        I2CIdle();

        if (I2CSTATbits.ACKSTAT == 0) // Did we receive an ACK ?

        {

            flag = 1;

            break;

        }

#if(I2C_DEBUG == 1)

        UART2PutChar('.');

#endif

    }

    if (!flag) return (FALSE); // Exit if there was a problem
    
    // Write out both address bytes
    I2CSendByte(0x00);
    
    I2CIdle();

    if (I2CSTATbits.ACKSTAT == 0) // Did we receive an ACK ?

    {

        flag = 1;

    }

    if (!flag) return (FALSE); // Exit if there was a problem

    // 4.if write cmd was successful, put the adress on the bus

    I2CSendByte(0x00);

    I2CIdle();

    for (j = 0; j < len; j++) {

        if (I2CSTATbits.ACKSTAT == 0) // Did we receive an ACK ?

        {

            I2CSendByte(0x55);
//            I2CSendByte(*(data + j));

        } else {

#if(I2C_DEBUG == 1)

            UART2PrintString("Error NACK Rxed\n");

#endif

            return FALSE;

        }

    }

    I2CStop();




    return TRUE;




}
#endif


/**

 * @brief A wrapper around drvI2CWriteRegisters() to write only a byte of data

 *

 * @param reg The register to start reading from (unsigned char)

 * @param byte The byte to write

 * @param slave_adr The 7 bit address of the slave without the R/W bits set

 * @return Boolean indicating if operation completed successfully or not

 */

/*
int drvI2CWriteByte(unsigned short int reg, unsigned char byte, unsigned char slave_adr) {

    return ( drvI2CWriteRegisters(reg, &byte, 1, slave_adr));

}
 */

