#ifndef __GLOBAL_VARIABLES_HEADER_FILE__
#define __GLOBAL_VARIABLES_HEADER_FILE__

#include "TypeDef.h"

extern  DEF16	    mda_value;
extern  DEF16	    int_data;
extern  DEF8	    mda_temp;

extern  BYTE		Control_Data[256];

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

extern  DEF8		led_stor;

extern  BYTE        FEN_Flag;
extern  BYTE        LIVE_2D_flag;
extern  BYTE        LIVE_3D_flag;

extern  WORD        Timer_Count_ms;
extern  WORD        WatchDog_Count_ms;

extern  WORD        Timer_Elapse_Tick;

extern  BYTE        LatestCommand;

extern  BYTE        LED_Count;
extern  BYTE        BUF_Count;
extern  BYTE        PZT_FG_Count;
extern  BYTE        PZT_BG_Count;
	
extern  BYTE        SequenceStep;

#endif
