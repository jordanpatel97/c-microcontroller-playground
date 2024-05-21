/*
IR.h file
Header file of IR.h
 */

#ifndef _IR_H
#define	_IR_H

#include <xc.h>
#define _XTAL_FREQ 8000000 //define _XTAL_FREQ so delay routines work

//ints required as 16 bits returned from register
struct IR_struct {
    char irLeft; 
    char irRight;
    signed int irDiff;
    char irSum;
};


//Call functions created in IR.c file
void initialiseTMR5(void);
void initialiseIR(void);
void getIRValues(struct IR_struct *ir);

#endif	/* IR_H */

