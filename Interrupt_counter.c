#include <xc.h>

void initialiseIO ()
{
    LATC=0; //set the output data latch levels to 0 on all pins
    LATD=0;
    TRISC=0b00001000; //set the data direction registers to output on all pins
    TRISD=0;
}

void initialiseInterrupts()
{
    // Enable interrupts
    INTCONbits.GIEH = 1; // Global Interrupt Enable bit
    // Set the button on RC3 to trigger an
    // interrupt. It is always high priority
    INTCONbits.INT0IE = 1; //INT0 External Interrupt Enable bit 
}
/************************************
/ Function delay
/ Used to introduce a delay of length t
************************************/
void delay (int t)
{
 //Skeleton function for delay code
 //put code in here to make a delay
 //of length t
 for (int i = 0; i < t; i++); //Value of t does not represent an actual time value
}
/************************************
/ Function LEDout
/ Used to display number on the LED array
/ in binary
************************************/
void LEDout(int number)
{
 //Skeleton function for displaying a binary number
    
    LATC = (0b11110000 & number<<2) | (LATC & 0b00001111) ;
    LATD = (0b00001100 & number<<2) | (0b00110000 & number>>2) | (LATD & 0b11000011);

}

////////////////////////////////
//GLOBAL VARIABLES
////////////////////////////////
int a = 0;

void main (void)
{
    initialiseIO();
    initialiseInterrupts();


    
    while (1)
    {
        if(a>255)
        {
            a = 0;
        }
        else
        {
            a++;
        }
        LEDout(a);
        delay(200);
    } 
}

// High priority interrupt routine
void interrupt InterruptHandlerHigh ()
{
    if (INTCONbits.INT0IF)
    {
        a--; // increment a counter
        LEDout(a);
        INTCONbits.INT0IF = 0; //clear the interrupt flag
    }
}

