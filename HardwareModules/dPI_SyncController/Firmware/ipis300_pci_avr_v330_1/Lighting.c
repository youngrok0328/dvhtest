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

void LED_2D_255_SET(void )
{
	
    BYTE i;
    for(i = 0 ; i < 16 ; i++)
    {
	LED_2D_TRANSFER(i,255);	
    }
}

void LED_2D_ON_OFF(void)
{
    if(FRAME_PERIOD_SIGNAL==0)
    {
         
         FEN_RST=0;
         LED_2D_ON();
         M_STEREO = 1;
	 Timer_Start();
	 FEN_RST=1;
	 delay(5);	
     }	
    else if(Timer_Count_ms >= LED_On_Time)
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
    if(FRAME_PERIOD_SIGNAL==0)
    {
		
		FEN_RST=0;
		LED_3D_ON();			
		Timer_Start();
		FEN_RST=1;
		delay(5);
    }
    else if(Timer_Count_ms >= Control_Data[LED_3D_FG_ON_DURATION])
    {
        LED_3D_OFF();
        Timer_Reset();
    }	
}

void LED_2D_Only_One(BYTE index)
{
    DEF16       Temp_data ;
    
    Temp_data.def_word=(~(0x0001<<index));
    *Led_on_off_address_h = Temp_data. ml[1];
    *Led_on_off_address_l = Temp_data. ml[0];
}

void AllClearIllumination(void)
{
	FEN_RST=0;
	M_PZT_AMP=0;
	LED_ONOFF_S=0;
	
	Timer_Reset();

	LED_2D_OFF();
	LED_3D_OFF();

	LIVE_2D_flag = 0;
	LIVE_3D_flag = 0;
	LIVE_2D_flag16=0;
	Int2_Flag=0;
	
	*Frame_Period_Data=Control_Data[FRAME_PERIOD];
	*Exposure_time=Control_Data[LED_2D_ON_DURATION];
	Sync_Option=Control_Data[FRAME_TRIGGER_OPTION];
	*Trigger_Option_Data=Sync_Option;
	Pulse_Width_Flag=0;
}


