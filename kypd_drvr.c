#include "kypd_drvr.h"


char keyMap[ 4][ 5] =
{
    { 1,2,3,0x0A},
    { 4,5,6,0x0B},
    { 7,8,9,0x0C},
    { 0,0x0F,0x0E,0x0D},   
};


void kypd_setup()
{
    // Columns are driven, Rows are read.  Lows is the state of interest, so
	// any value in the driven low column, which is low, has it button depressed.
    TRISECLR = 0x0F;    // Columns - Set bits 0-3 as outputs
    TRISESET = 0x0F0;   // Rows - Set bits 4-7 as inputs
    
    // Drive all columns high to start.  Will drive each one individually when reading is done.
    COL1 = 1;
    COL2 = 1;
    COL3 = 1;
    COL4 = 1;
    
}

void kypd_teardown()
{
    TRISESET = 0x0FF;   // make everything an input (default)
}

// local data structure used to hold the state of each row
static unsigned short int kypd_row[4];


// Check the state of each switch
void kypd_readSwitches( )
{
    // Drive each column low and read the rows
    COL1 = 0;
    kypd_row[0] = ROW4 << 3 | ROW3 << 2 | ROW2 << 1 | ROW1;
    COL1 = 1;
    
    COL2 = 0;
    kypd_row[1] = ROW4 << 3 | ROW3 << 2 | ROW2 << 1 | ROW1;
    COL2 = 1;

    COL3 = 0;
    kypd_row[2] = ROW4 << 3 | ROW3 << 2 | ROW2 << 1 | ROW1;
    COL3 = 1;
    
    COL4 = 0;
    kypd_row[3] = ROW4 << 3 | ROW3 << 2 | ROW2 << 1 | ROW1;
    COL4 = 1;
}

// Process the saved switch status and determine which button was pushed.
char kypd_getKey()
{
    int col = 0xFF;
    int row;
    
    if (kypd_row[0] != 0x000F)
    {
        col = 0;
    }
    else if (kypd_row[1] != 0x000F)
    {
        col = 1;
    }
    else if (kypd_row[2] != 0x000F)
    {
        col = 2;
    }
    else if (kypd_row[3] != 0x000F)
    {
        col = 3;
    }
    
    switch (col)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            switch ( (~kypd_row[ col] & 0x000F) )
            {
                case 1:
                    row = 0;
                    break;
                    
                case 2:
                    row = 1;
                    break;
                    
                case 4:
                    row = 2;
                    break;
                    
                case 8:
                    row = 3;
            }
            return keyMap[ row][ col];
            
        default:
            return 0xFF;
    }
}