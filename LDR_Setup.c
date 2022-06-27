
#include <xc.h>
#pragma config OSC = IRCIO, WDTEN = OFF //internal oscillator, WDT off

//GLOBAL VARIABLES
int timer = 0; //Set to current time
int counter = 0;

void initialiseInterrupts() {
    // Enable high priority interrupts
    INTCONbits.GIEH = 1; // Global Interrupt Enable bit

    //Low Interrupt Priority setup
    INTCONbits.GIEL = 1; //Enable low priority interrupts

}

//timer setup

void initialiseTimer() {
    T0CONbits.TMR0ON = 1; //turn on timer0
    T0CONbits.T016BIT = 0; // 16bit mode
    T0CONbits.T0CS = 0; // use internal clock (Fosc/4)
    T0CONbits.PSA = 0; // enable prescaler
    T0CONbits.T0PS = 0b100; // set prescaler value divides by 32
    //calculate the offset value)

    TMR0H = 0b1011;
    TMR0L = 0b11011100; //Sets count so starts from 3036 to 65536 (2^16))
    // Generate an interrupt on timer overflow
    INTCONbits.TMR0IE = 1; //enable TMR0 overflow interrupt
    INTCON2bits.TMR0IP = 0; // TMR0 Low priority (0 for low, 1 for high))
}

void initialiseIO() {
    LATC = 0; //set the output data latch levels to 0 on all pins
    LATD = 0;
    TRISC = 0b00001000; //set the data direction registers to output on all pins
    TRISD = 0;


    //Used to initialise LDR light meter
    TRISA = 0b0001000; //pin RA3
    ANSEL0 = 0b00001000; //pin RA3
    ANSEL1 = 0;
    ADCON0 = 0b00001101;
    ADCON1 = 0b00000000;
    ADCON2 = 0b10101011;
}

void delay(int t) 
{
    //function for delay code
    //of length t
    for (int i = 0; i < t; i++); //Value of t does not represent an actual time value
}

//Function to get the light level from the LDR. Returns the binary value (10 bit).
int getLightLevel() {
    int ADResult = 0; //Result from analogue input
    ADCON0bits.GO = 1; // Start conversion
    while (ADCON0bits.GO); // Wait until conversion done
    ADResult = ADRESL; // Get the 8 bit LSB result
    ADResult += ((unsigned int) ADRESH << 8); //Get 2 MSB   
    ADResult = 1023 - ADResult;
    return ADResult;
}

void LEDout(int number) {
    //function for displaying a binary number
    //on the LED array
    LATC = (0b11110000 & number << 2) | (LATC & 0b00001111);
    LATD = (0b00001100 & number << 2) | (0b00110000 & number >> 2) | (LATD & 0b11000011);
    //    LATC = ((number&0b000111100)<<2) | (LATC & 0b00001111) ;
    //    LATD = ((number&0b111000000)>>2) | ((number&0b0000000011)<<2) | (LATD & 0b10000011);
}

void main(void) {
    //INITIALISE STUFF/////////
    OSCCON = 0x72; //8MHz clock ? why 0x72? Check the datasheet!
    RCONbits.IPEN = 1; //enable interrupt priority
    PIR1 = 0; //clear the peripheral interrupt flags
    while (!OSCCONbits.IOFS); //Wait for OSC to become stable 
    initialiseIO();
    initialiseInterrupts();
    initialiseTimer();
    ///////////////////////////

    int iLightLevel;
    int iLowerLightLimit = 400;
    int iUpperLightLimit = 520;
    int night;
    int MSB;
    int DST;

    while (1) //While to keep execution continuing
    {
        iLightLevel = getLightLevel(); //Get light level
        ///////////////////////////
        //If light is off, turn on
        if ((iLightLevel < iLowerLightLimit) && !night) 
        { //)) {
            night = 1;
            counter = 0;
            //dispVal = toggleMSB(counter);
        }            ///////////////////////////
            //If light is on, turn off
        else if ((iLightLevel > iUpperLightLimit) && night) //Turns off light if upper limit is exceeded 
        {
            timer = counter/2; //resets the timer to corrected time.
            night = 0;
            if(counter > 8)
            {
                DST = 1;
                timer++;
            }
            else
            {
                DST = 0;
            }
        }

        ///////////////////////////
        // if time is between 1 and 5am turn off
        if (night) 
        {
            if (((timer < 7 && timer > 1) && (DST)) || ((timer < 6 && timer > 0) && (!DST))) 
            {
                MSB = 0;
            } 
            else
            {
                MSB = 0b1 << 7;
            }
        }
        else
        {
            MSB = 0;
        }
        ////////////////////////////
        //        //Changes daylight saving time
        //        if ((iLightLevel > iUpperLightLimit) & timer > 19)
        //        {
        //            timer = timer + 1;
        //        }
        //        else if ((iLightLevel < iLowerLightLimit) & timer < 7)
        //        {
        //            timer = timer - 1;
        //        }
        ///////////////////////////

        //LEDout(iLightLevel);

        //Counts to 24 seconds then resets back to 0
        if (timer > 23) {

            timer = 0;
        }
        LEDout(timer | MSB);
                //LEDout(dispVal); //Display the counter and MSB (to tell if the street light will be on/off))
    }
}

// Low priority interrupt routine
//void interrupt InterruptHandlerLow ()

void __interrupt(low_priority) my_lp_isr(void) {
    if (INTCONbits.TMR0IF) {
        timer++; // increment a counter
                counter++;
                INTCONbits.TMR0IF = 0; //clear the interrupt flag
    }
}


