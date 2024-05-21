/*
IR.c file
Initialises IR sensors and configures CAP1 and CAP3 registers
*/

#include <xc.h>
#include<string.h>
#include<stdio.h>
//#pragma config OSC = IRCIO, WDTEN = OFF
#include "IR.h"

void initialiseTMR5(void){  //Initialise TMR5 register

  T5CONbits.T5SEN = 0; //Timer5 disabled during Sleep
  T5CONbits.RESEN = 0; //Special Event Reset disabled
  T5CONbits.T5MOD = 0; //Continuous Count mode enabled
  T5CONbits.T5PS = 0b11; //1:8 prescaler forces 250KHz on TMR5
  T5CONbits.T5SYNC = 0;
  T5CONbits.TMR5CS = 0; // use internal clock
  T5CONbits.TMR5ON = 1; //Enable tmr5
}

//Initiate CAP1 and CAP 3 registers
void initialiseIR(void){
    // analogue to digital conversion
    ANSEL0 = 0x00;
    ANSEL1 = 0x00;
    
    //CAP1CON configuration
    CAP1CONbits.CAP1REN = 1; //Enable time reset
    CAP1CONbits.CAP1M = 0b0110; //PWM mode - every falling to rising edge
    
    //CAP2CON configuration
    CAP2CONbits.CAP2REN = 1; //Enable time reset
    CAP2CONbits.CAP2M = 0b0110; //PWM mode - every falling to rising edge
    
    ANSEL0 = 0; //Set pins AN7-AN0 to digital I/O
    ANSEL1 = 0; //Set pin AN8 to digital I/O

    CAP1BUFL = 0;
    CAP1BUFH = 0; //Reset CAP1BUFL & H to 0

    CAP2BUFL = 0;
    CAP2BUFH = 0; //Reset CAP2BUFL & H to 0
}

//Get IR values from CAP1 and CAP2 and write to IR struct
//CAP1 = right
//CAP2 = left
void getIRValues(struct IR_struct *ir)
{  
    char tempLeftIR = CAP1BUFH;
    char tempRightIR = CAP2BUFH;
    if (tempLeftIR > 49) //Set maximum possible value to 50
    {
        tempLeftIR = 49;
    }
    if (tempRightIR > 49) //Set maximum possible value to 50
    {
        tempRightIR = 49;
    }
    ir->irLeft = tempLeftIR;//tempLeftIR; //Creates 8 bit value (only use high 8 bits since low values change too rapidly))
    ir->irRight = tempRightIR;//tempRightIR;
    ir->irDiff = ir->irLeft - ir->irRight;
    ir->irSum = ir->irLeft + ir->irRight;
    //Reset Buffers
    CAP1BUFH = 0;
    CAP2BUFH = 0;
    __delay_ms(100); //Delay to ensure values will have been updated

}