#include "TypeDef.h"
#include "sync_ram.h"

DEF16	    mda_value;
DEF16	    int_data;
DEF8	    mda_temp;

DEF16       Tem_data16;



BYTE		Control_Data[256];
WORD		Control_Data16[256];
WORD		Control_Data16_Rear[256];

BYTE		*Led_ch_address = (BYTE *)LED_CH_ADD;	    //led 채널 선택을 위한 어드레스
BYTE		*Led_val_address = (BYTE *)LED_VAL_ADD; 	//led 설정값을 보내기위한 어드레스

BYTE		*Led_on_off_address_h = (BYTE *)LED_ADD_H;	//led on/off를 위한 어드레서 상위 8~15까지
BYTE		*Led_on_off_address_l = (BYTE *)LED_ADD_L;	//led on/off를 위한 어드레서 하위 0~7까지

BYTE		*Dio_write_h = (BYTE *)WCS1;
BYTE		*Dio_write_l = (BYTE *)WCS0;

BYTE		*Dio_read_h = (BYTE *)RCS1;
BYTE		*Dio_read_l = (BYTE *)RCS0;

BYTE		*Dio_w_read_h = (BYTE *)WCS1_R;
BYTE		*Dio_w_read_l = (BYTE *)WCS0_R;

BYTE        *PC_Address = (BYTE *)PC_ADD;
BYTE        *PC_Data = (BYTE *)PC_DATA;

BYTE        *Frame_Period_Data=(BYTE *)FRAME_PERIOD_ADD;
BYTE        *Trigger_Option_Data=(BYTE *)FRAME_TRIGGER_OPTION_ADD;
BYTE        *Exposure_time=(BYTE *)EXPOSURE_TIME_ADD;

DEF8		led_stor;

BYTE        FEN_Flag = 0;
BYTE        LIVE_2D_flag = 0;
BYTE        LIVE_3D_flag = 0;

WORD        Timer_Count_ms = 0;
WORD        WatchDog_Count_ms = 0;

WORD        Timer_Elapse_Tick = 0xFC17;

BYTE        LatestCommand = 0;

DEF16       LatestCommand16 ;
DEF16       LatestCommand16_Rear;

BYTE        LED_Bffer_Add=0;
BYTE        Pulse_Width_Flag=0;
BYTE        Int2_Flag=0;
BYTE        Main_Fram_Index=0;
BYTE        LIVE_2D_flag16 = 0;

BYTE        Grab_State=0;
BYTE        Channel_Index=0;


BYTE        LED_Count = 0;
BYTE        BUF_Count = 0;
BYTE        PZT_FG_Count = 0;
BYTE        PZT_BG_Count = 0;
BYTE        LED_On_Time;
BYTE        G_Flag;
	
BYTE        SequenceStep = 0;

DEF8	    _Sync_Option;

