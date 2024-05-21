/*
LCD.h file
Header file of LCD.c
 */
#ifndef _LCD_H
#define	_LCD_H

#include <xc.h>
#include <stdio.h>
#include "IR.h"
#define _XTAL_FREQ 8000000 //define _XTAL_FREQ so delay routines work

 void E_TOG(void);
 void LCDout(unsigned char number);
 void SendLCD(unsigned char Byte, char type);
 void LCD_Init(void);
 void clear_LCD(void);
 void SetLine(char line);
 void LCD_String(char *string);
 void clear_LCD(void);
 void IR_LCD_display(char *buf_r, char *buf_l, struct IR_struct *ir);
 void LCD_main(void);
#endif	
 
 /* LCD_module_H */