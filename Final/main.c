#include <xc.h>
#pragma config OSC = IRCIO //compiler directive to set clock to internal oscillator
#include<string.h>
#include<stdio.h>
#define _XTAL_FREQ 8000000 //define _XTAL_FREQ so delay routines work


//Call Headers
#include "LCD.h"
#include "RFID.h"
#include "DC_Motor.h"
#include "IR.h"


//Global Variables
//RFID variables
char g_rfidValToSend[16]; //stores rfid code to send to lcd 
char g_rfidCharCounter = 0; //counter for RFID interrupt (global to avoid multiple writes))
char g_bGotRFIDValue = 0;

/////////////////////


void initialiseInterrupts()
{
    // Enable interrupts
    PIE1bits.RCIE = 1; //PIR1 External Recieve Interrupt Enable bit 
    INTCONbits.GIEL = 1; //enable peripheral interrupts
    INTCONbits.GIEH = 1; //enable global interrupts
    INTCON3bits.INT2IE = 1; //enable external interrupts
}

void main(void) 
{
    //IR Struct Setup
    struct IR_struct IR_vals;

    IR_vals.irLeft = 0;
    IR_vals.irRight = 0;
    IR_vals.irDiff = 0;
    IR_vals.irSum = 0;
    
    initPWM();
    //Initialise motor structs
    //Motor Left Initial Setup
    struct DC_motor motorL, motorR; //declare two DC_motor structures
    motorL.power = 0; //zero power to start
    motorL.direction = 0; //set default motor direction
    motorL.dutyLowByte = (unsigned char *) (&PDC0L); //store address of PWM duty low byte
    motorL.dutyHighByte = (unsigned char *) (&PDC0H); //store address of PWM duty high byte
    motorL.dir_pin = 0; //pin RB0/PWM0 controls direction
    motorL.PWMperiod = 199; //store PWMperiod for motor

    //same for motorR but different PWM registers and direction pin 
    motorR.power = 0; 
    motorR.direction = 0; 
    motorR.dutyLowByte = (unsigned char *) (&PDC1L); 
    motorR.dutyHighByte = (unsigned char *) (&PDC1H);
    motorR.dir_pin = 2;
    motorR.PWMperiod = 199;
    
    //Initialise stuff
    initialiseInterrupts();
    LCD_main();
    initialiseRFID();
    initialiseTMR5();
    initialiseIR(); 
    
    //RFID Variables////
    char bDisplayedRFIDValue = 0; //'boolean' to check if RFID value has been displayed
    
    //IR Variables/////
    char foundIRStatus = 0; //0 = no direction found, 1 = general direction found (move forwards left /right), 2 = go forwards slowly (target acquired)
    char phaseRotateSumThreshold = 2; //Maximum Threshold for sum of left and right IR to conclude that bot should rotate
    char phaseRotateDiffThreshold = 2; //Maximum Threshold for difference of left and right IR to conclude that bot should rotate
    char phaseForwardsLeftRightSumThreshold = 5; //Minimum Threshold for IR sum to enable left / right forwards movement 
    char phaseForwardsLeftRightDiffThreshold = 5; //Minimum Threshold for IR difference to enable left / right forwards movement 
    char phaseForwardsSumThreshold = 97; //Minimum Threshold for IR sum to enable a move forwards. Seen that at >2.5m both IR vals were 49
    char phaseForwardsDiffThreshold = 1; //Maximum Threshold for IR difference to enable a move forwards
    
    //Retrace variables
    char retraceArray[100] = {0}; //1 = slow right, 2 = slow left, 3 = fast forwards, 4 = slow forwards
    char nextArrayPos = 0;  //holds next available return array position
    char bHome = 0; //Determines whether the bot is at the start position - default to 0

    OSCCON = 0x72; //internal oscillator, 8MHz
    while (!OSCCONbits.IOFS); //Wait for OSC to become stable
    
    //Main while 1 loop
    while(1)
    {   
        //Checks if the bot is docked
        if (!bHome)
        {
            
            /////////////////////////////////////////////////////
            //RFID - checks if RFID value has been stored and displays it if so display it
            /////////////////////////////////////////////////////
            if (g_bGotRFIDValue && !bDisplayedRFIDValue)
            {
                for(char i = 0; i < 16; i++) //print all values in array (to stop strange || values affecting output))
                {  
                    SendLCD(g_rfidValToSend[i],1); //display read code on LCD
                }
                bDisplayedRFIDValue = 1;
                continue; 
            }

            //Main statements to Locate and Move to beacon
            if (!g_bGotRFIDValue)
            {
                ////////////////////////////////////////////////////
                //IR
                ////////////////////////////////////////////////////////
                SetLine(1); //Set Line 1
                
                /////////////////////////////////////////////////////
                //Initial motor movement to determine position
                //Rotate right super slowly until a signal is found
                /////////////////////////////////////////////////////
                if (foundIRStatus == 0)
                {
                    //Checks that foundIRStatus is correct (once again)
                    if (IR_vals.irSum < phaseRotateSumThreshold && abs(IR_vals.irDiff) < phaseRotateDiffThreshold)
                    {
                        LCD_String("Rotating..."); 
                        //Turn quickly right
                        turnHardRight(&motorL, &motorR);
                        getIRValues(&IR_vals); 
                        retraceArray[nextArrayPos] = 3;
                        nextArrayPos++;
                    }

                    ///////////////////////////////////////////
                    //Updates foundIRStatus
                    ///////////////////////////////////////////
                    if (IR_vals.irSum > phaseForwardsSumThreshold && abs(IR_vals.irDiff) < phaseForwardsDiffThreshold) //~Equal signal found for both IR's
                    {
                        foundIRStatus = 2;
                        continue;

                    }
                    else if (IR_vals.irSum > phaseForwardsLeftRightDiffThreshold && abs(IR_vals.irDiff) > phaseForwardsLeftRightSumThreshold) //Large enough difference found between left and right
                    {
                        foundIRStatus = 1;
                        continue;

                    }
                    else if (IR_vals.irSum < phaseRotateSumThreshold && abs(IR_vals.irDiff) < phaseRotateDiffThreshold) //No strong signal found. Rotate on the spot
                    {
                        foundIRStatus = 0;
                        continue;

                    }


                }

                //////////////////////////////////////////////////////
                //Move left and right whilst still moving forwards
                //////////////////////////////////////////////////////
                if (foundIRStatus == 1)
                {
                    LCD_String("Slow r and l");
                    if (IR_vals.irSum > phaseForwardsLeftRightDiffThreshold && abs(IR_vals.irDiff) > phaseForwardsLeftRightSumThreshold)
                    {
                       if (IR_vals.irDiff < 0) //When right signal is stronger
                       {
                           //Turn slowly right
                           turnRight(&motorL, &motorR); 
                           __delay_ms(200);
                           getIRValues(&IR_vals); 
                           retraceArray[nextArrayPos] = 0;
                           nextArrayPos++;
                       }
                       else if (IR_vals.irDiff > 0) //When left signal is stronger
                       {
                           //Turn slowly left
                           turnLeft(&motorL, &motorR);
                           __delay_ms(200);
                           getIRValues(&IR_vals);
                           retraceArray[nextArrayPos] = 1;
                           nextArrayPos++;
                       }  
                    }

                    if (IR_vals.irSum > phaseForwardsSumThreshold && abs(IR_vals.irDiff) < phaseForwardsDiffThreshold) //~Equal signal found for both IR's
                    {
                        foundIRStatus = 2;
                        continue;

                    }
                    else if (IR_vals.irSum > phaseForwardsLeftRightDiffThreshold && abs(IR_vals.irDiff) > phaseForwardsLeftRightSumThreshold) //Large enough difference found between left and right
                    {
                        foundIRStatus = 1;
                        continue;

                    }
                    else if (IR_vals.irSum < phaseRotateSumThreshold && abs(IR_vals.irDiff) < phaseRotateDiffThreshold) //No strong signal found. Rotate on the spot
                    {
                        foundIRStatus = 0;
                        continue;
                    }
                }

                //////////////////////////////////////////////////////
                //Go forwards slowly
                //////////////////////////////////////////////////////
                if (foundIRStatus == 2)
                {
                    LCD_String("forwards slowly"); 
                    if (IR_vals.irSum > phaseForwardsSumThreshold && abs(IR_vals.irDiff) < phaseForwardsDiffThreshold)
                    {
                        //Go forwards for 0.5 second
                        goForwardsHalf(&motorL, &motorR);
                        __delay_ms(500);
                        getIRValues(&IR_vals);
                        retraceArray[nextArrayPos] = 2;
                        nextArrayPos++;
                    }
                    if (IR_vals.irSum > phaseForwardsSumThreshold && abs(IR_vals.irDiff) < phaseForwardsDiffThreshold) //~Equal signal found for both IR's
                    {
                        foundIRStatus = 2;
                        continue;
                    }
                    else if (IR_vals.irSum > phaseForwardsLeftRightDiffThreshold && abs(IR_vals.irDiff) > phaseForwardsLeftRightSumThreshold) //Large enough difference found between left and right
                    {
                        foundIRStatus = 1;
                        continue;
                    }
                    else if (IR_vals.irSum < phaseRotateSumThreshold && abs(IR_vals.irDiff) < phaseRotateDiffThreshold) //No strong signal found. Rotate on the spot
                    {
                        foundIRStatus = 0;
                        continue;
                    }
                }

                getIRValues(&IR_vals);
            }

            //Go Back HOME!!!
            else
            {
               
                //Stop motors
                turnMotorOff(&motorL);
                turnMotorOff(&motorR);
                __delay_ms(200); 

                SetLine(2); //Set Line
                LCD_String("CS Correct!!");

                char i;
                //Cycle through retraceArray from end to start
                for (i = nextArrayPos-1; i>0; i--)
                {
                    //If value = 0 -> go backwards right
                    if (retraceArray[i] == 0)
                    {
                       //Turn slowly right
                       turnRightBack(&motorL, &motorR); 
                       __delay_ms(200);
                       continue;
                    }
                    if (retraceArray[i] == 1)
                    {
                       //Turn slowly right
                       turnLeftBack(&motorL, &motorR);
                       __delay_ms(200);
                       continue;
                    }
                    if (retraceArray[i] == 2)
                    {
                        //Go back slowly
                        goBackwardsHalf(&motorL, &motorR);
                        __delay_ms(500);
                        continue;
                    }
                    //If value = 3 -> Rotate in the opposite direction (this ideally shouldn't be required)
                    if (retraceArray[i] == 3)
                    {
                        //Turn hard left (opposite to forwards movement)
                        turnHardLeft(&motorL, &motorR);
                        continue;
                    }
                }
                turnMotorOff(&motorL);
                turnMotorOff(&motorR);
                __delay_ms(200); 
                bHome = 1;
            }
        }
        else
        {
            turnMotorOff(&motorL);
            turnMotorOff(&motorR);
        }
    }
}

// High priority interrupt routine
//void interrupt InterruptHandlerHigh ()
void __interrupt(high_priority) my_hp_isr(void)
{
    int rfid[16]; //unchecked rfid values
    if(PIR1bits.RCIF){
        rfid[g_rfidCharCounter] = RCREG; //setting rfid and rfidsend equal to the received byte
        g_rfidValToSend[g_rfidCharCounter]=RCREG;
        g_rfidCharCounter = g_rfidCharCounter+1;
    }  
    if(RCREG == 0x03)
    {
        if(checkSum(rfid)==1) //is checksum satsified
        { 
            g_bGotRFIDValue = 1;//return 1 if passes checksum
        }
    }
}
  
  

   
