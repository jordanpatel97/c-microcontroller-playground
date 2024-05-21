/*
RFID.c file
Written to initiate RFID receivers and to retrieve message from the tag*/

#include <xc.h>
#include <string.h>
#include <stdio.h>
#include "RFID.h"


//initialise RFID module, setup USART and set pins as input 
void initialiseRFID(void) {
    TRISCbits.RC7 = 1;
    SPBRG = 207;
    SPBRGH = 0;
    BAUDCONbits.BRG16 = 1; //set baud rate scaling to 16 bit mode
    TXSTAbits.BRGH = 1; //high baud rate select bit
    RCSTAbits.CREN = 1; //continuous receive mode
    RCSTAbits.SPEN = 1; //enable serial port, other settings default
    TXSTAbits.TXEN = 1; //enable transmitter, other settings default
}

//Return the RFID tag data when the PIR1 interrupt bit is flagged
char getCharSerial(void) 
{
    while (!PIR1bits.RCIF) {
    }; //wait for the data to arrive
    return RCREG; //return byte in RCREG

}

 char* getRFIDOutput(void)
 {
    static char buf[12];
    char i = 0; //counter
    while (1)
    {
        buf[i] = getCharSerial(); 
        
        if (buf[i] == 0x02)
        {
            buf[i] = getCharSerial(); 
            while(buf[i] != 0x03)
            {
                buf[i] = getCharSerial(); 
                i++;
            }
            if (i >= 11) //When 12 bits or more are contained within the buffer return
            {
                return (char*)buf;
                break;
            }
        }   
    }
 }
 
 char checkSum(int* inputString)
 {
    //Initialise variables
    int hex[12]; 
    int check = 0; 
    int pair[6];   
    int afterXor;

    //For loop to convert ascii to hex
    for(char i=0; i < 12; i++)
    {
        if(inputString[i] >= 65 & inputString[i] <= 70) //ascii value is between 65 and 70 (hexa A to F)
        {
            hex[i] = inputString[i] - 55;
        } 
        else if(inputString[i] >= 48 & inputString[i] <= 57) //ascii value is between 48 and 57 (1 to 9)
        { 
            hex[i] = inputString[i] - 48;
        } 
    }    
   
    //Join hex's to make pairs
    for (char j = 0; j < 6; j++)
    {   
        pair[j] = (hex[j*2]<<8 | hex[(j*2)+1]); //1st data byte is upper 4 bits and 2nd data byte is lower 4 bits
    }  

    //XOR 
    afterXor = ((((pair[0]^pair[1])^pair[2])^pair[3])^pair[4]); //5 data pairs XORed together

    //Checks if last pair is equal to the xored value
    if(pair[5]==afterXor){
        check = 1; 
    }
    return check; 
 }