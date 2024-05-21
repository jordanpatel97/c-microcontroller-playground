#include "LCD.h"

#define LCD_E LATCbits.LATC0
#define LCD_RS LATAbits.LATA6
#define LCD_DB7 LATDbits.LATD1
#define LCD_DB6 LATDbits.LATD0
#define LCD_DB5 LATCbits.LATC2
#define LCD_DB4 LATCbits.LATC1

void E_TOG(void){
    LCD_E = 1;
    __delay_us(5);
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
    // using type as the argument
    // send high bits of Byte using LCDout function
    // send low bits of Byte using LCDout function
    LCD_RS = type;
        LCDout((Byte & 0b11110000) >> 4);
        __delay_us(50);
        LCDout(Byte & 0b00001111);
        __delay_us(50);
}
void LCD_Init(void){
    // set initial LAT output values (they start up in a random state)
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISCbits.RC0 = 0;
    TRISCbits.RC1 = 0;
    TRISCbits.RC2 = 0;
    TRISAbits.RA6 = 0;
      
    // set all pins low
    // set LCD pins as output (TRIS registers)
    LCD_E = 0;
    LCD_RS = 0;
    LCD_DB7 = 0;
    LCD_DB6 = 0;
    LCD_DB5 = 0;
    LCD_DB4 = 0;
    
 
    // Initialisation sequence code - see the data sheet    
    __delay_ms(15);
    LCDout(0b0011);
    __delay_ms(5);
    LCDout(0b0011);
    __delay_us(200);
    LCDout(0b0011);
    __delay_us(50);
    LCDout(0b0010);
    __delay_us(50);
    
    // Set, clear screen, set entry mode, display on etc to finish initialisation
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
 
void clear_LCD(void)
{
    SendLCD(0b00000001,0); //send garabge
    __delay_ms(10);
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
    __delay_us(50);
}
 
//Function uses a pointer to output a string of characters to the LCD screen
void LCD_String(char *string)
{
    //While the data pointed to isn't a 0x00 do below
    while(*string != 0)
    {
        //Send out the current byte pointed to
        // and increment the pointer
        SendLCD(*string++,1);
    }
}

 void LCD_main(void)
 {
    TRISC = 0b11000000; //set data direction registers, 8MHz clock
    
    OSCCON = 0x72; //8MHz clock
    while(!OSCCONbits.IOFS); //Wait for OSC to become stable
    
    //setup LCD
    LCD_Init(); //Initialize the LCD
    SetLine(1);
    
    //Set Serial comms
    SPBRG=206; //set baud rate
    SPBRGH=0;
    BAUDCONbits.BRG16=1; //set baud rate scaling to 16 bit mode
    TXSTAbits.BRGH=1; //high baud rate select bit
    RCSTAbits.CREN=1; //continous receive mode
    RCSTAbits.SPEN=1; //enable serial port, other settings default
    TXSTAbits.TXEN=1; //enable transmitter, other settings default
    
 }



