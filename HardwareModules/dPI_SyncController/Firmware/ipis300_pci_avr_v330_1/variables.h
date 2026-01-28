#ifndef __GLOBAL_VARIABLES_HEADER_FILE__
#define __GLOBAL_VARIABLES_HEADER_FILE__

#include "TypeDef.h"


__no_init WORD   Ver_Val   @0xb00e;
__no_init WORD   Buffer_Data @0xb008;
__no_init WORD   LED_ON_Timer16 @0xb00a;

extern  DEF16	    mda_value;
extern  DEF16	    int_data;
extern  DEF8	    mda_temp;
extern  DEF16       Tem_data16;


extern  BYTE		Control_Data[256];
extern  WORD		Control_Data16[256];
extern  WORD		Control_Data16_Rear[256];

extern  BYTE		*Led_ch_address;
extern  BYTE		*Led_val_address;
extern  BYTE		*Led_on_off_address_h;
extern  BYTE		*Led_on_off_address_l;

extern  BYTE		*Dio_write_h;
extern  BYTE		*Dio_write_l;

extern  BYTE		*Dio_read_h;
extern  BYTE		*Dio_read_l;

extern  BYTE		*Dio_w_read_h;
extern  BYTE		*Dio_w_read_l;

extern  BYTE        *PC_Address;
extern  BYTE        *PC_Data;

extern BYTE        *Frame_Period_Data;
extern BYTE        *Trigger_Option_Data;
extern BYTE        *Exposure_time;

extern  DEF8		led_stor;

extern  BYTE        FEN_Flag;
extern  BYTE        LIVE_2D_flag;
extern  BYTE        LIVE_3D_flag;

extern  WORD        Timer_Count_ms;
extern  WORD        WatchDog_Count_ms;

extern  WORD        Timer_Elapse_Tick;

extern  BYTE        LatestCommand;
extern  DEF16        LatestCommand16;
extern  DEF16        LatestCommand16_Rear;
extern  BYTE        LED_Bffer_Add;
extern  BYTE        Pulse_Width_Flag;
extern  BYTE        Int2_Flag;
extern  BYTE        Main_Fram_Index;
extern  BYTE        LIVE_2D_flag16;
extern  BYTE        Grab_State;
extern  BYTE        Channel_Index;

extern  BYTE        LED_Count;
extern  BYTE        BUF_Count;
extern  BYTE        PZT_FG_Count;
extern  BYTE        PZT_BG_Count;
extern  BYTE        LED_On_Time;

extern  BYTE        G_Flag;
extern  BYTE        SequenceStep;
extern  DEF8	    _Sync_Option;
#endif
