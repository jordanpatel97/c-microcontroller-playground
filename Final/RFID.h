/*
RFID.h file
Header file of RFID.h
 */

#ifndef _RFID_H
#define	_RFID_H

#include <xc.h>
#define _XTAL_FREQ 8000000 //define _XTAL_FREQ so delay routines work

void initialiseRFID(void);
char* getRFIDOutput(void);
char getCharSerial(void);
char checkSum(int* inputString);

#endif	/* RFID_H */

