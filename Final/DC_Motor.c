/*Initialise motors, determine functions to turn, go forwards and backwards*/

#include <xc.h>
#include "DC_Motor.h"
#pragma config OSC=IRCIO
#define _XTAL_FREQ 8000000 //i.e. for an 8MHz clock frequency

void initPWM(void) // function to setup PWM
{
    LATB = 0; //Defualt to 0 latch vals
    TRISB = 0; // Set direction for all pins
    PTCON0 = 0b00000000; // free running mode, 1:64 prescaler = 32 us
    PTCON1 = 0b10000000; // enable PWM timer
    PWMCON0 = 0b01111111; // PWM0/1 enabled, all independent mode
    PWMCON1 = 0x00; // special features, all 0 (default)
    PTPERL = 0b11000111; // base PWM period low byte
    PTPERH = 0b00000000; // base PWM period high byte
}

// function to set PWM from the values in the motor structure
void setMotorPWM(struct DC_motor *m)
{
     int PWMduty; //tmp variable to store PWM duty cycle
     if (m->direction)
     {
        // low time increases with power
        PWMduty=m->PWMperiod - ((int)(m->power)*(m->PWMperiod))/100;
     }
     else 
     {
        // high time increases with power
        PWMduty=((int)(m->power)*(m->PWMperiod))/100;
     }
     PWMduty = (PWMduty << 2); // two LSBs are reserved for other things
     *(m->dutyLowByte) = PWMduty & 0xFF; //set low duty cycle byte
     *(m->dutyHighByte) = (PWMduty >> 8) & 0x3F; //set high duty cycle byte

     if (m->direction)
     { // if direction is high,
        LATB=LATB | (1<<(m->dir_pin)); // set dir_pin bit in LATB to high
     } 
     else 
     { // if direction is low,
         LATB=LATB & (~(1<<(m->dir_pin))); // set dir_pin bit in LATB to low
     }
}

void delay_s(char seconds) // function to delay in seconds
{
    for (int i = 0; i < (seconds*1000); i++)
    {
       __delay_ms(1);
        //__delay_ms() is limited to a maximum delay of 89ms with an 8Mhz
        //So need to use this to make longer delays 
    }
}

void turnMotorOff(struct DC_motor *m) 
{
    //Reduce motor power until 0
    for (m->power; (m->power) > 0; (m->power)--) 
    { 
        setMotorPWM(m); 
        __delay_us(50); 
    }
} 

void goForwardsFull(struct DC_motor *m_L, struct DC_motor *m_R)
{
    //Set direction
    m_L->direction = 0;
    m_R->direction = 0;
    //Set both motors to 100 power
    fullSpeedUp(m_L); 
    fullSpeedUp(m_R);
}

void goForwardsHalf(struct DC_motor *m_L, struct DC_motor *m_R)
{
    //Set direction
    m_L->direction = 0;
    m_R->direction = 0;
    //Set both motors to 50 power
    if (m_R->power < 50)
    {
        halfSpeedUp(m_R);
    }
    else if (m_R->power > 50)
    {
        halfSpeedDown(m_R);
    }
    if (m_L->power < 50)
    {
        halfSpeedUp(m_L);
    }
    else if (m_L->power > 50)
    {
        halfSpeedDown(m_L);
    }
}

void fullSpeedUp(struct DC_motor *m) 
{
    for (m->power; (m->power) < 100; (m->power)++) { //increase motor power until 100
        setMotorPWM(m); 
        __delay_us(50);
    } 
}
void halfSpeedUp(struct DC_motor *m) 
{
    for (m->power; (m->power) < 50; (m->power)++) { //increase motor power until 50
        setMotorPWM(m); 
        __delay_us(50);
    } 
}
void halfSpeedDown(struct DC_motor *m) 
{
    for (m->power; (m->power) > 50; (m->power)--) { //increase motor power until 50
        setMotorPWM(m); 
        __delay_us(50);
    } 
}

void turnHardRight(struct DC_motor *m_L, struct DC_motor *m_R) 
{
    if (!(m_L->power == 0)) //Only turn motor off if motor is on
    {
        turnMotorOff(m_L);
    }
    if (!(m_R->power == 0)) //Only turn motor off if motor is on
    {
        turnMotorOff(m_R);
    }
    m_R->direction = 1;
    m_L->direction = 0;
    halfSpeedUp(m_L);
    halfSpeedUp(m_R);
    //Do for 100 ms
    __delay_ms(100);
    //Stop all movement to enable time for IR vals to refresh
    turnMotorOff(m_L);
    turnMotorOff(m_R);
    __delay_ms(300);
}

// turn robot base Left whilst moving
void turnLeft(struct DC_motor *m_L, struct DC_motor *m_R)
{
    m_R->direction = 0;
    m_L->direction = 0;
    //Stop the left motor and power the right
    if (m_R->power < 100) //Only turn motor off if motor is on
    {
        fullSpeedUp(m_R);
    }
    //Turn right motor on to half speed if not already
    if (m_L->power < 50)
    {
        halfSpeedUp(m_L);
    }
    else if (m_R->power > 50)
    {
        halfSpeedDown(m_L);
    } 
}

// turn robot base right whilst moving
void turnRight(struct DC_motor *m_L, struct DC_motor *m_R)
{
    m_R->direction = 0;
    m_L->direction = 0;
    //Stop the left motor and power the right
    if (m_L->power < 100) //Only turn motor off if motor is on
    {
        fullSpeedUp(m_L);
    }
    //Turn right motor on to half speed if not already
    if (m_R->power < 50)
    {
        halfSpeedUp(m_R);
    }
    else if (m_R->power > 50)
    {
        halfSpeedDown(m_R);
    } 
}


///////////////////////////////////////////////////////////////
//Backward moves
///////////////////////////////////////////////////////////////
void turnHardLeft(struct DC_motor *m_L, struct DC_motor *m_R) 
{
    if (!(m_L->power == 0)) //Only turn motor off if motor is on
    {
        turnMotorOff(m_L);
    }
    if (!(m_R->power == 0)) //Only turn motor off if motor is on
    {
        turnMotorOff(m_R);
    }
    m_R->direction = 0;
    m_L->direction = 1;
    halfSpeedUp(m_L);
    halfSpeedUp(m_R);
    //Do for 150 ms
    __delay_ms(150);
    //Stop all movement to enable time for IR vals to refresh
    turnMotorOff(m_L);
    turnMotorOff(m_R);
    __delay_ms(300);
}

// turn robot base right whilst moving backwards 50/100 power split
void turnRightBack(struct DC_motor *m_L, struct DC_motor *m_R)
{
    m_R->direction = 1;
    m_L->direction = 1;
    //Stop the left motor and power the right
    if (m_L->power < 100) //Only turn motor off if motor is on
    {
        fullSpeedUp(m_L);
    }
    __delay_ms(89);
    //Turn right motor on to half speed if not already
    if (m_R->power < 50)
    {
        halfSpeedUp(m_R);
    }
    else if (m_R->power > 50)
    {
        halfSpeedDown(m_R);
    } 
    __delay_ms(89);
}

// turn robot base Left whilst moving
void turnLeftBack(struct DC_motor *m_L, struct DC_motor *m_R)
{
    m_R->direction = 1;
    m_L->direction = 1;
    //Stop the left motor and power the right
    if (m_R->power < 100) //Only turn motor off if motor is on
    {
        fullSpeedUp(m_R);
    }
    __delay_ms(89);
    //Turn right motor on to half speed if not already
    if (m_L->power < 50)
    {
        halfSpeedUp(m_L);
    }
    else if (m_R->power > 50)
    {
        halfSpeedDown(m_L);
    } 
    __delay_ms(89);
}

void goBackwardsHalf(struct DC_motor *m_L, struct DC_motor *m_R)
{
    //Set direction
    m_L->direction = 1;
    m_R->direction = 1;
    //Set both motors to 50 power
    if (m_R->power < 50)
    {
        halfSpeedUp(m_R);
    }
    else if (m_R->power > 50)
    {
        halfSpeedDown(m_R);
    }
    if (m_L->power < 50)
    {
        halfSpeedUp(m_L);
    }
    else if (m_L->power > 50)
    {
        halfSpeedDown(m_L);
    }
    __delay_ms(50);
    __delay_ms(50);
}
