#include "iom128.h"
#include "ina90.h"
#include "Variables.h"
#include "Interrupt.h"


void Init_Interrupt(void)
{
 	_CLI();
    EICRA = 0x00;
    EICRB = 0x00;
    EIMSK = 0x00;
	_SEI();
}

void INT0_Enable(void)
{
	_CLI();
	EICRA |= 0x03;      // ______11 : Rising Edge Detection
 	EICRB = 0x00;       // extended ext ints
 	EIMSK |= 0x01;      // _______1 : Enable External Interrupt 0
	_SEI();
}

void INT0_Disable(void)
{
	_CLI();
	EIMSK &= (~0x01);   // _______0 : Disable External Interrupt 0
	_SEI();
}

void INT1_Enable(void)
{
	_CLI();
	EICRA &= (~0x0C);   // ____00__ : Low Level Detection
//  EICRA |= 0x08;      // ____10__ : Falling Edge Detection
 	EICRB = 0x00;       // extended ext ints
 	EIMSK |= 0x02;      // ______1_ : Enable External Interrupt 1
	_SEI();
}

void INT1_Disable(void)
{
	_CLI();
	EIMSK &= (~0x02);  	// ______0_ : Disable External Interrupt 1
	_SEI();
}    

void Wait_FEN(void)
{
    while(!FEN_Flag);
    
    FEN_Flag = 0;
}

#pragma std::vector = INT0_vect 
__interrupt void INI0(void)
{
    FEN_Flag = 0xff;
}

#pragma std::vector=INT1_vect 
__interrupt void  INI1(void)
{
    BYTE ADDR = *PC_Address;
    BYTE DATA = *PC_Data;
    
	Control_Data[ADDR] = DATA;
}

#pragma std::vector = TIMER1_OVF_vect 
__interrupt void TIMER1(void)
{
    TCNT1 = Timer_Elapse_Tick;
    Timer_Count_ms++;
}

#pragma std::vector = TIMER3_OVF_vect
__interrupt void TIMER3(void)
{
    TCNT3 = Timer_Elapse_Tick;
    WatchDog_Count_ms++;
}

