/*
DC_Motor.h file
Header file of DC_Motor.h
 */

#ifndef _DC_MOTOR_H
#define _DC_MOTOR_H

#define _XTAL_FREQ 8000000

#include <xc.h>

struct DC_motor { //definition of DC_motor structure
    char power;         //motor power, out of 100
    char direction;     //motor direction, forward(1), reverse(0)
    unsigned char *dutyLowByte; //PWM duty low byte address
    unsigned char *dutyHighByte; //PWM duty high byte address
    char dir_pin; // pin that controls direction on PORTB
    int PWMperiod; //base period of PWM cycle
    char powerPrev;     //Previosuly requested power
};

//function prototypes from lab exercises
void initPWM(void); // function to setup PWM
void delay_s(char); //function to delay by 1 second
void setMotorPWM(struct DC_motor *m); //Sets up motor PWM -> starts off at 0 power
void turnMotorOff(struct DC_motor *m); //Turns 1 motor off
void goForwardsFull(struct DC_motor *m_L, struct DC_motor *m_R); //Sets both motors to full power
void goForwardsHalf(struct DC_motor *m_L, struct DC_motor *m_R); //sets both motors to half power
void fullSpeedUp(struct DC_motor *m); //Increases 1 motor to full speed
void halfSpeedUp(struct DC_motor *m); //Increases 1 motor to half speed
void halfSpeedDown(struct DC_motor *m); //Decreases 
void turnHardLeft(struct DC_motor *m_L, struct DC_motor *m_R);
void turnHardRight(struct DC_motor *m_L, struct DC_motor *m_R);
void turnLeft(struct DC_motor *m_L, struct DC_motor *m_R);
void turnRight(struct DC_motor *m_L, struct DC_motor *m_R);
//Backwards moves
void turnLeftBack(struct DC_motor *m_L, struct DC_motor *m_R);
void turnRightBack(struct DC_motor *m_L, struct DC_motor *m_R);
void goBackwardsHalf(struct DC_motor *m_L, struct DC_motor *m_R);

#endif
