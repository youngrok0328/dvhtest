#include "iom128.h"
#include "ina90.h"
#include "Variables.h"
#include "Interrupt.h"
#include    "Lighting.h"
#include    "CommDef.h"

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
	EIMSK &= (~0x02);  	// ______0_ : Disable External Interrupt 2
	_SEI();
}    
void INT2_Enable(void)
{
	_CLI();
	//EICRA &= (~0x0C);   // __00____ : Low Level Detection
        EICRA |= 0x30;      // __11____ : Rising Edge Detection
 	EICRB = 0x00;       // extended ext ints
 	EIMSK |= 0x04;      // _____1__ : Enable External Interrupt 2
	_SEI();
}

void INT2_Disable(void)
{
	_CLI();
	EIMSK &= (~0x04);  	// _____0__ : Disable External Interrupt 1
	_SEI();
}  

void Wait_FEN(void)
{
    while(!FEN_Flag);
    
    FEN_Flag = 0;
}

#pragma vector = INT0_vect 
__interrupt void INI0(void)
{
    FEN_Flag = 0xff;
}

BYTE g_wAddress = 0;

#pragma vector=INT1_vect 
__interrupt void  INI1(void)
{
    BYTE ADDR,DATA ; 
    WORD DATA16 ; 
    
    DATA16 = Buffer_Data;
    DATA = *PC_Data;
    ADDR = *PC_Address;
 
    switch(ADDR)
    {
        case SERIAL_ADD_FRONT:
              g_wAddress=*PC_Data;
             break;
	case SERIAL_DATA_FRONT:
	      Control_Data16[g_wAddress]=DATA16;
	      Control_Data[g_wAddress] = DATA;
	     break;
	case SERIAL_ADD_REAR :
	      g_wAddress=*PC_Data; 
	     break;
	case SERIAL_DATA_REAR: 
	      Control_Data16_Rear[g_wAddress]=DATA16;
	     break;    
    
    }
    
}

#pragma vector = INT2_vect 
__interrupt void INI2(void)
{
   LED_2D_OFF();
   Int2_Flag=0;
}

#pragma vector = TIMER1_OVF_vect 
__interrupt void TIMER1(void)
{
    TCNT1 = Timer_Elapse_Tick;
    Timer_Count_ms++;
}

#pragma vector = TIMER3_OVF_vect
__interrupt void TIMER3(void)
{
    TCNT3 = Timer_Elapse_Tick;
    WatchDog_Count_ms++;
}

