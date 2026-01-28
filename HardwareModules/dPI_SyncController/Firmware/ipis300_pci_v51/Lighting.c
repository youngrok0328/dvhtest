#include "TypeDef.h"
#include "Variables.h"
#include "Macro.h"
#include "Timer.h"
#include "CommDef.h"
#include "sync_io.h"
#include "DioDef.h"
#include "Lighting.h"
#include "Interrupt.h"

void delay(WORD del);   // From Main.

#define		STORAGE_TIME	110
void LED_2D_TRANSFER(BYTE led_ch,BYTE led_value)
{
	led_ch_temp=led_ch;		//led_storage를 위한 비트 컨트롤 때문에 행함
	*Led_val_address=led_value;
	delay(10);
	led_storage=1;
	*Led_ch_address=led_ch_temp;
	led_storage=0;
	delay(STORAGE_TIME);
	*Led_ch_address=led_ch_temp;
	led_storage=1;
	delay(STORAGE_TIME);
	*Led_ch_address=led_ch_temp;
	delay(STORAGE_TIME);
}

void LED_2D_ON(void)
{
	*Led_on_off_address_h = 0x00;
	*Led_on_off_address_l = 0x00;
}

void LED_2D_OFF(void)
{
	*Led_on_off_address_h = 0xff;
	*Led_on_off_address_l = 0xff;
}

void LED_2D_SET(BYTE frame)
{
	BYTE  nChannel;
	const BYTE nChannelCount = 16;
    const BYTE nDataOffset = frame * nChannelCount;
	
	for(nChannel = 0 ; nChannel < nChannelCount ; nChannel++)
	{
		LED_2D_TRANSFER(nChannel, Control_Data[nDataOffset + nChannel]);	
    }
}

void LED_2D_ON_OFF(void)
{
	if(FEN_Flag)
	{
		FEN_Flag = 0;

		LED_2D_ON();			
		Timer_Start();
		M_STEREO = 1;
	}
    else if(Timer_Count_ms >= Control_Data[LED_2D_ON_DURATION])
    {
        M_STEREO = 0;
        LED_2D_OFF();
        Timer_Reset();
    }	
}

void LED_3D_ON(void)
{
    M_3DLED = 1;
}

void LED_3D_OFF(void)
{
    M_3DLED = 0;
}

void LED_3D_ON_OFF(void)
{
	if(FEN_Flag)
	{
		FEN_Flag = 0;

		LED_3D_ON();			
		Timer_Start();
	}
	else if(Timer_Count_ms >= Control_Data[LED_3D_FG_ON_DURATION])
    {
        LED_3D_OFF();
        Timer_Reset();
    }	
}

void AllClearIllumination(void)
{
	Timer_Reset();

	LED_2D_OFF();
	LED_3D_OFF();

	LIVE_2D_flag = 0;
	LIVE_3D_flag = 0;
}


