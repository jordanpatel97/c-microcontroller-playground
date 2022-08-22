#include <xc.h>
#include<string.h>
#include<stdio.h>
#include<math.h> //for round function
//compiler directive to set clock to internal oscillator
#pragma config OSC = IRCIO
#define _XTAL_FREQ 8000000 //define _XTAL_FREQ so delay routines work
#define LCD_E LATCbits.LATC0
#define LCD_RS LATAbits.LATA6
#define LCD_DB7 LATDbits.LATD1
#define LCD_DB6 LATDbits.LATD0
#define LCD_DB5 LATCbits.LATC2
#define LCD_DB4 LATCbits.LATC1

//Initialise Global variables
int counter = 0; //Used to count number of button presses

void E_TOG(void){
    LCD_E = 1;
    __delay_us(5); // 5us delay
    LCD_E = 0;
}
 
//function to send four bits to the LCD
void LCDout(unsigned char number){
//set data pins using the four bits from number
//toggle the enable bit to send data
    LCD_DB4 = number & 0b0001;
    LCD_DB5 = (number & 0b0010) >> 1;
    LCD_DB6 = (number & 0b0100) >> 2;
    LCD_DB7 = (number & 0b1000) >> 3;
    E_TOG();
    __delay_us(5); // 5us delay
}
 
//function to send data/commands over a 4bit interface
void SendLCD(unsigned char Byte, char type){
// set RS pin whether it is a Command (0) or Data/Char (1)
// send high bits of Byte using LCDout function
LCD_RS = type;
    LCDout((Byte & 0b11110000) >> 4);
    __delay_us(50);
    LCDout(Byte & 0b00001111);
    __delay_us(50); // 10us delay
// send low bits of Byte using LCDout function
}
void LCD_Init(void){
// set initial LAT output values (they start up in a random state)
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISCbits.RC0 = 0;
    TRISCbits.RC1 = 0;
    TRISCbits.RC2 = 0;
    TRISAbits.RA6 = 0;
      
//set all pins low
// set LCD pins as output (TRIS registers)
    LCD_E = 0;
    LCD_RS = 0;
    LCD_DB7 = 0;
    LCD_DB6 = 0;
    LCD_DB5 = 0;
    LCD_DB4 = 0;
    
 
// Initialisation sequence code - see the data sheet    
    __delay_ms(15); //delay 15mS 
    LCDout(0b0011); //send 0b0011 using LCDout
    __delay_ms(5); //delay 5ms
    LCDout(0b0011); //send 0b0011 using LCDout
    __delay_us(200); //delay 200us
    LCDout(0b0011); //send 0b0011 using LCDout
    __delay_us(50); //delay 50us
    LCDout(0b0010); //send 0b0010 using LCDout set to four bit mode
    __delay_us(50); //delay 50us
    
    // now use SendLCD to send whole bytes ? send function set, clear
    // screen, set entry mode, display on etc to finish initialisation
    SendLCD(0b00001000, 0); //Function Set
    __delay_us(50);
    SendLCD(0b00001000, 0); // display off 
    __delay_us(50);
    SendLCD(0b00000001, 0); //clearLCD
    __delay_ms(5);
    SendLCD(0b00000110, 0); //entry set
    __delay_us(50);
    SendLCD(0b00001110, 0); //turn display on
    __delay_us(50);
}
 
 
//function to put cursor to start of line
void SetLine (char line){
//Send 0x80 to set line to 1 (0x00 ddram address)
//Send 0xC0 to set line to 2 (0x40 ddram address)
    if (line == 1) {
        SendLCD(0x80, 0);
    }
    if (line == 2) {
        SendLCD(0xC0, 0);
    }
    __delay_us(50); // 50us delay
}
 
//Function uses a pointer to output a string of characters to the LCD screen
void LCD_String(char *string){
    while(*string != 0){
    //Send out the current byte pointed to
    // and increment the pointer
    SendLCD(*string++,1);
    __delay_us(50); //to see each character
    //being printed in turn
    }
}
 
void main (void)
{
    char buf[16]; //buffer for characters for LCD   
    OSCCON = 0x72; //8MHz clock
    while(!OSCCONbits.IOFS); //Wait for OSC to become stable

    LCD_Init(); //Initialize the LCD

    SetLine(1); //Set Line 1

    //Display the number of button presses
    //sprintf(buf,"No of presses = %d", counter);
    //strcpy(buf);
    //LCD_String(buf); //output string to LCD 
    char bForward = 1; //sets default direction to forward 
    int iDirection = 0b0000011100; //defaults to shift right
    int i = 0; //postion
    
    //Make car shape
    strcpy(buf,"o-o");
    LCD_String(buf); //output string to LCD 
    while(1)
    {
        if(i > 16 && bForward) //checks if should be going backwards
        {
            bForward = 0;
        }
        else if (i == 0 && !bForward) //checks if should be going forwards
        {
            bForward = 1;
        }
        //Changes direction of shift based on bForward
        if(bForward)
        {
            iDirection = 0b0000011100; 
        }
        else if (!bForward)
        {
            iDirection = 0b0000011000;  
        }

        __delay_ms(200);
        SendLCD(iDirection, 0); //SHIFT display 
        __delay_ms(200);
        if (bForward == 1)
        {
            i++;
        }
        else
        {
            i--;
        }
    }

    //LDR Voltage display (with DP))
//    float lightLevel = 0; //Initialise the light level
//    int int_lightLevel;
//    int int_part;
//    int dec_part;
//    lightLevel = getLightLevel();
////    sprintf(buf, "Light = %.2f", lightLevel);
////    LCD_String(buf); //output string to LCD 
//    int_lightLevel = round(lightLevel);
//    int_part = int_lightLevel / 205 //(1023/5 = 204.6)).
//    dec_part = (int_lightLevel / 2.05) - int_part*100;
//    sprintf(buf,"%d.%02d\n", int_part, dec_part);
       
    while (1) {
            SendLCD(0b00011000, 0); //shift
            __delay_us(50);

        }
}
