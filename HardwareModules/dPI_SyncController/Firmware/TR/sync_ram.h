
#ifndef		SYNC_RAM_HEADER
#define		SYNC_RAM_HEADER


// LED PORT CONTROL
#define		LEDW0		0x8000
#define		LEDW1		0x8004
#define		LEDW2		0x8008
#define		LEDW3		0x800c

// DIO PORT CONTROL
#define		RCS0		0xa000
#define		RCS1		0xa004

#define		WCS0		0x9000
#define		WCS1		0x9004

#define		WCS0_R		0xa008
#define		WCS1_R		0xa00c

#define		LED_CH_ADD	0x8000
#define		LED_VAL_ADD	0x8004

#define		LED_ADD_L	0x8008
#define		LED_ADD_H	0x800c

#define		PC0_ADD_ADD	0xf000
#define		PC0_DATA_ADD	0xf004
#define		PC1_ADD_ADD	0xf008
#define		PC1_DATA_ADD	0xf00c


//LED와 DIO의 제어를 위해 어드레스 활당하는 함수

//엔코더 측정을 위한 주소
#define         Pulse_Time       0x2000  //엔코더 주기 값 저장 메모리


//#define         Timer0_Value_A		0xc000
//#define         Timer1_Value_A   	0xc004		
//#define         Register2_add	        0xc00c		

#define		FVAL_Add	        0xd000
//#define         Register3_add           0xd004	//읽기 : 홀수 LVAL 주기 값 ,    쓰기 : 데이터1  전송
//#define         Register4_add           0xd00c	//읽기 : 짝수 LVAL 주기 값 , 쓰기 : 데이터3  전송

//#define		Register_Stat_A   	0xe000
//#define		Camera_Trigger_A  	0xe004 
#define		Trigger_RST_A     	0xe004      


// PC Interface
/************************************************
	CPLD와 MCU간의 데이터 통신용 어드레스로 
	PC와 MCU 사이의 중간 매체가 된다
	0xf001~0xf003까지 임의로 설정
**************************************************/
#define		D_READ		0xf001	

enum
{
	ILLUM_COMMAND = 0xa0,   // Illumination Command
	ILL_2D_COUNT,           // 2D Frame Count ( 1 ~ 10 )
	ILL_BUF_COUNT,	    // Buffer Frame Count Between 2D & 3D Frames
	ILL_3D_FG_COUNT,        // 3D Frame Count ( 0 or 3 or 4 )
	ILL_3D_BG_COUNT,        // 3D Frame Count ( 0 or 3 or 4 )
	SELECT_PZT,             // Select PZT						
	PZT_DAC_MODE,           // PZT DAC Mode ( 0 : 10V, 1 : 5V )
	LEVEL_PZT0_H,           // Level_PZT[0]_H					
	LEVEL_PZT0_L,           // Level_PZT[0]_L                   
	LEVEL_PZT1_H,           // Level_PZT[1]_H					
	LEVEL_PZT1_L,           // Level_PZT[1]_L                   
	LEVEL_PZT2_H,           // Level_PZT[2]_H					
	LEVEL_PZT2_L,           // Level_PZT[2]_L                   
	LEVEL_PZT3_H,           // Level_PZT[3]_H					
	LEVEL_PZT3_L,           // Level_PZT[3]_L                   
	LED_2D_ON_DURATION,     // LED 2D On Duration (ms)
	LED_3D_FG_ON_DURATION,  // LED 3D On Duration (ms)
	LED_3D_BG_ON_DURATION,  // LED 3D On Duration (ms)
	CAMERA_CYCLE_TIME,      //Camera Cycle Time
	GRAB_DELAY_TIME,        //Grab Delay Time
	GRAB_SELECT_MODE,        //00: Trigger ,01:Continue (Continue or Trigger Grab Selector)
	LED_ON_FLAG_LOW,        //
	LED_ON_FLAG_HI,         //
	USED_ILLUM_SEL_BIT_L =0xc0,
	USED_ILLUM_SEL_BIT_H,
	Avr_Addr_ToRead      =0xfd 
	
};


enum
{// old Tr
	COMMAND_LED_OFF = 0x1b,    // Turn On 2D Frame - 0
	COMMAND_LED_ON,           // Turn On 2D Frame - 1
	COMMAND_LED_SET,           // Turn On 2D Frame - 2
	COMMAND_2D_3,           // Turn On 2D Frame - 3
	COMMAND_2D_4,           // Turn On 2D Frame - 4
	COMMAND_2D_5,           // Turn On 2D Frame - 5
	COMMAND_2D_6,           // Turn On 2D Frame - 6
	COMMAND_2D_7,           // Turn On 2D Frame - 7
	COMMAND_2D_8,           // Turn On 2D Frame - 8
	COMMAND_2D_9,           // Turn On 2D Frame - 9
	COMMAND_3D_LED,         // Turn On 3D LED
	LASER_ON,
	LASER_OFF,
	MEASURE_START = 0x20   // Start Measurement
};
//---------------------------------------
enum //firmware_mode
{
        TR_FLYINGTRG_STROBE_MODE =0,
        TR_TIMEBASE_ONOFF_MODE =1

};


enum //Illum data16BIT
{
        ILLUM_LED_CH1_VALUE =0x00,//0x00~0x9f,led value
        ILLUM_COMMAND16 = 0xa0,   // Illumination Command
	ACQ_2D_FRAMECOUNT,           // 2D Frame Count ( 1 ~ 10 )
	CAMERA_FRAMEPERIOD =0xb2,      //Camera frameperiod_ms
	SYNC_OP_OPTION =0xb3,        
        CH1_TIME_SCALE     =0xe0, //0xe0~0xef
        USED_ILLUM_SEL_BIT =0xf0, //used led ch sel        
};
// P1,P2_Illumination Command List
enum
{
	LIVE_COMMAND_2D_0 = 0x10,    // Turn On 2D Frame - 0
	LIVE_COMMAND_2D_1,           // Turn On 2D Frame - 1
	LIVE_COMMAND_2D_2,           // Turn On 2D Frame - 2
	LIVE_COMMAND_2D_3,           // Turn On 2D Frame - 3
	LIVE_COMMAND_2D_4,           // Turn On 2D Frame - 4
	LIVE_COMMAND_2D_5,           // Turn On 2D Frame - 5
	LIVE_COMMAND_2D_6,           // Turn On 2D Frame - 6
	LIVE_COMMAND_2D_7,           // Turn On 2D Frame - 7
	LIVE_COMMAND_2D_8,           // Turn On 2D Frame - 8
	LIVE_COMMAND_2D_9,           // Turn On 2D Frame - 9
	LIVE_LED_ON =0x1c,		
	MEASURE_INS = 0xf0,   // Start Measurement
	//Vision S/W는 사용하지 않음, Snyc내부에서 동작 enum
	MEASURE_INS_LED_ON_OFF,
	MEASURE_INS_DELAY,
	MEASURE_INS_END,
	COINCIDENCE_MEASURE =0xfa,
	COINCIDENCE_MEASURE_LED_ON_OFF =0xfb,
	AVR_PARA_ADDR_TOREAD =0xfd,
	//
	COMMAND_CLEAR = 0xff   // Turn Off All Illuminations
};


enum //Grab_State
{
        CHANEL_OPERATION =0,
        FRAME_DONE_CHECK,
        GRAB_COMPLETE
};

enum CHIP_SELECT
{
    _CS0 = 0,
    _CS1,
};

//----------------------------------------

//encoder 측정을 위한 변수

#define         En_Counte_Value_H       0xc3
#define         En_Counte_Value_L       0xc4


#define		BYTE		unsigned char
#define		WORD		unsigned int


typedef struct _def8_bit {

	unsigned char bit0		:1; /**/
	unsigned char bit1		:1; /**/
	unsigned char bit2		:1; /**/
	unsigned char bit3		:1; /**/
	unsigned char bit4		:1; /**/
	unsigned char bit5		:1; /**/
	unsigned char bit6		:1; /**/
	unsigned char bit7		:1; /**/
} DEF8_BIT;		

typedef struct  _def16_bit {
    unsigned int bit0       :1; /**/
    unsigned int bit1       :1; /**/
    unsigned int bit2       :1; /**/
    unsigned int bit3       :1; /**/
    unsigned int bit4       :1; /**/
    unsigned int bit5       :1; /**/
    unsigned int bit6       :1; /**/
    unsigned int bit7       :1; /**/
    unsigned int bit8       :1; /**/
    unsigned int bit9       :1; /**/
    unsigned int bit10      :1; /**/
    unsigned int bit11      :1; /**/
    unsigned int bit12      :1; /**/
    unsigned int bit13      :1; /**/
    unsigned int bit14      :1; /**/
    unsigned int bit15      :1; /**/
} DEF16_BIT;


typedef union   __DEF16 {
    unsigned int        def_word;
    unsigned char       ml[2];
    DEF16_BIT           bit;
} DEF16;


DEF16	mda_value;
#define		mda_temp_value		mda_value.def_word
#define		pzt_value_h		mda_value.ml[1]
#define		pzt_value_l		mda_value.ml[0]
#define		sdi_value		mda_value.bit.bit11
		
DEF16	int_data;
#define		int_value		int_data.def_word
#define		int_value_h		int_data.ml[1]
#define		int_value_l		int_data.ml[0]

//----------------------added-------------------//
DEF16	data_16Bit;
#define		Value_16Bit     	data_16Bit.def_word
#define		Value_16Bit_h		data_16Bit.ml[1]
#define		Value_16Bit_l		data_16Bit.ml[0]

DEF16	data_16Bit1;
#define		Value_16Bit1     	data_16Bit1.def_word
#define		Value_16Bit1_h		data_16Bit1.ml[1]
#define		Value_16Bit1_l		data_16Bit1.ml[0]

DEF16	data_16Bit2;
#define		Value_16Bit2     	data_16Bit2.def_word
#define		Value_16Bit2_h		data_16Bit2.ml[1]
#define		Value_16Bit2_l		data_16Bit2.ml[0]

typedef union __DEF8 {
	unsigned char		def_byte;
	DEF8_BIT			bit;
} DEF8;



DEF8	mda_temp;
#define		mda_temp_data	mda_temp.def_byte
#define		mda_temp0	mda_temp.bit.bit0
#define		mda_temp1	mda_temp.bit.bit1
#define		mda_temp2	mda_temp.bit.bit2

DEF8	mLaser_temp;

#define		MLaser_flag	  mLaser_temp.bit.bit0	
#define		mLaser_Control	  mLaser_temp.bit.bit1


//BYTE		control_data_probe0[256];
//BYTE		control_data_probe1[256];
WORD            Control_Data16bit1[256];
WORD            Control_Data16bit2[256];
//WORD            P2_Control_Data16[256];
WORD		level_pzt[4];




BYTE		stereo_delay;

BYTE		tdillch0;
BYTE		tdillch1;



// 조명 제어를 위해 추가된 변수-2005.12.27
BYTE		on_off_flag;
BYTE		master_on;

BYTE		Timer_2d;
BYTE		Ch_2d_on_h;
BYTE		Ch_2d_on_l;



DEF8		led_stor;
#define		led_ch_temp	led_stor.def_byte
#define		led_storage	led_stor.bit.bit4	//led control board에 값을 저장 하기 위한 포트	`


BYTE            *pc_add_probe0;
BYTE	        *pc_data_probe0;
	
BYTE            *pc_add_probe1;
BYTE            *pc_data_probe1;

BYTE		*Led_ch_address;	//led 채널 선택을 위한 어드레스
BYTE		*Led_val_address;	//led 설정값을 보내기위한 어드레스

BYTE		*Led_on_off_address_h;	//led on/off를 위한 어드레서 상위 8~15까지
BYTE		*Led_on_off_address_l;	//led on/off를 위한 어드레서 하위 0~7까지

BYTE		*Dio_write_h;
BYTE		*Dio_write_l;

BYTE		*Dio_read_h;
BYTE		*Dio_read_l;

BYTE		*Dio_w_read_h;
BYTE		*Dio_w_read_l;

//------------------------------
DEF16 _P1_Led_Ch_16bit;
#define		P1_Led_Ch_16bit		_P1_Led_Ch_16bit.def_word
#define		P1_Led_Ch_16bit_h	_P1_Led_Ch_16bit.ml[1]
#define		P1_Led_Ch_16bit_l	_P1_Led_Ch_16bit.ml[0]

DEF16 _P2_Led_Ch_16bit;
#define		P2_Led_Ch_16bit		_P2_Led_Ch_16bit.def_word
#define		P2_Led_Ch_16bit_h	_P2_Led_Ch_16bit.ml[1]
#define		P2_Led_Ch_16bit_l	_P2_Led_Ch_16bit.ml[0]

DEF8	Reg_stat_;
#define		Reg_stat	  Reg_stat_.def_byte	
#define		Reg_stat0	  Reg_stat_.bit.bit0
#define		Reg_stat1	  Reg_stat_.bit.bit1
#define		Reg_stat2	  Reg_stat_.bit.bit2
#define		Reg_stat3	  Reg_stat_.bit.bit3

//added by kch(25.05.13)
DEF8 	P1_AvrBitSet_; //Write
#define		P1_AvrBitSet   	  P1_AvrBitSet_.def_byte	
#define		P1_Led_On	  P1_AvrBitSet_.bit.bit0
#define		P1_M_Start	  P1_AvrBitSet_.bit.bit1
#define		P1_Fen_Sig	  P1_AvrBitSet_.bit.bit7

DEF8 	P1_CpldBitSet_; //Read
#define		P1_CpldBitSet  	  P1_CpldBitSet_.def_byte	
#define		P1_Led_Status     P1_CpldBitSet_.bit.bit0
#define		P1_M_End	  P1_CpldBitSet_.bit.bit1
#define		P1_Fen_En	  P1_CpldBitSet_.bit.bit7

DEF8 	P2_AvrBitSet_; //Write
#define		P2_AvrBitSet   	  P2_AvrBitSet_.def_byte	
#define		P2_Led_On	  P2_AvrBitSet_.bit.bit0
#define		P2_M_Start	  P2_AvrBitSet_.bit.bit1
#define		P2_Fen_Sig	  P2_AvrBitSet_.bit.bit7

DEF8 	P2_CpldBitSet_; //Read
#define		P2_CpldBitSet 	  P2_CpldBitSet_.def_byte	
#define		P2_Led_Status	  P2_CpldBitSet_.bit.bit0
#define		P2_M_End	  P2_CpldBitSet_.bit.bit1
#define		P2_Fen_En	  P2_CpldBitSet_.bit.bit7

DEF8 	P1_FirmWare_Mode_Set_; //Read
#define		P1_FirmOpStatus	  P1_FirmWare_Mode_Set_.def_byte	
#define		P1_IllCont_mode	  P1_FirmWare_Mode_Set_.bit.bit0
#define         P1_Measure_Mode_Bit0  P1_FirmWare_Mode_Set_.bit.bit1 //Bit01:Inpoket,Bit10:Oti,Bit11:combination
#define         P1_Measure_Mode_Bit1  P1_FirmWare_Mode_Set_.bit.bit2
DEF8 	P2_FirmWare_Mode_Set_; //Read
#define		P2_FirmOpStatus	  P2_FirmWare_Mode_Set_.def_byte	
#define		P2_IllCont_mode	  P2_FirmWare_Mode_Set_.bit.bit0
#define         P2_Measure_Mode_Bit0  P2_FirmWare_Mode_Set_.bit.bit1 //Bit01:Inpoket,Bit10:Oti,Bit11:combination
#define         P2_Measure_Mode_Bit1  P2_FirmWare_Mode_Set_.bit.bit2
//DEF8 	FirmWare_Mode_Set_; //Read
//#define		FirmOpStatus	  FirmWare_Mode_Set_.def_byte	
//#define		IllCont_mode	  FirmWare_Mode_Set_.bit.bit7
//#define         M_Mode_Bit0       FirmWare_Mode_Set_.bit.bit0 //Bit01:Inpoket,Bit10:Oti,Bit11:combination
//#define         M_Mode_Bit1       FirmWare_Mode_Set_.bit.bit1

DEF8 SyncParaStatus_Write_;
#define		Micom_Para_Status_Set	  SyncParaStatus_Write_.def_byte	//Bit0:control_data1,Bit2:control_data2,Bit4:Illum_data16
#define		ContData1_Valid	          SyncParaStatus_Write_.bit.bit0 //1:Valid,0:invalid
#define     ContData2_Valid           SyncParaStatus_Write_.bit.bit2 //1:Valid,0:invalid
#define     CntData16_1_Valid         SyncParaStatus_Write_.bit.bit4
#define     CntData16_2_Valid         SyncParaStatus_Write_.bit.bit6

DEF8 SyncParaStatus_Read_;
#define		Pc_Para_Status_Set	  SyncParaStatus_Read_.def_byte	//Bit0:control_data1,Bit2:control_data2,Bit4:Illum_data16
#define		Request_ContData1	  SyncParaStatus_Read_.bit.bit0 
#define		Recv_Complete_ContData1	  SyncParaStatus_Read_.bit.bit1 
#define     Request_ContData2         SyncParaStatus_Read_.bit.bit2 //1:Valid,0:invalid
#define		Recv_Complete_ContData2	  SyncParaStatus_Read_.bit.bit3 
#define     Request_CntData16_1       SyncParaStatus_Read_.bit.bit4
#define		Rcv_Cplt_CntData16_1	  SyncParaStatus_Read_.bit.bit5
#define     Request_CntData16_2       SyncParaStatus_Read_.bit.bit6
#define		Rcv_Cplt_CntData16_2	  SyncParaStatus_Read_.bit.bit7  


#define  Total_Led_Ch 16

BYTE		*Timer0_Value_d;
BYTE		*Timer1_Value_d;

BYTE	       *Register_stat_d;
BYTE	       *Camera_Trigger_d;
BYTE	       *Trigger_Rst_d;  

BYTE           int0_flag =0;
BYTE           int1_flag =0;
BYTE           int2_flag =0;

BYTE           Led_off_time0;
BYTE           Led_off_time1;

BYTE           Probe0_LED;
BYTE           Probe1_LED;

BYTE           Probe0_LED_On_Flag;
BYTE           Probe1_LED_On_Flag;
///////---------------/////////////
BYTE            Cur_Led_Ch_h;
BYTE            Cur_Led_Ch_l;
BYTE            Grab_State;
BYTE            Channel_Index1;
BYTE            Channel_Index2;
WORD            Timer_Count_ms;
WORD            WatchDog_Count_ms;
WORD            P1_Led_Sel_Bit;
WORD            P2_Led_Sel_Bit;
BYTE            Led_Ch_Add_Offset1;
BYTE            Led_Ch_Add_Offset2;
BYTE            gCommand1;
BYTE            gCommand2;
BYTE            FrameIndex1;
BYTE            FrameIndex2;
BYTE            Grab_State1;
BYTE            Grab_State2;
BYTE            MultiCam_Grab_State;
BYTE            MultiCam_FrameIndex;
BYTE            MultiCam_Channel_Index;
BYTE            P1_SeqStep;
BYTE            P2_SeqStep;
BYTE            P1_FrameNum;
BYTE            P2_FrameNum;
BYTE            MultiCam_FrameNum; 
BYTE            Time_Delay1;
BYTE            Time_Delay2;
BYTE            P1_Img_Acq_Num;
BYTE            P2_Img_Acq_Num;
BYTE            Pre_Command1;
BYTE            Pre_Command2;
//__no_init BYTE   P1_CAM_SIG_SET   @0xb000;
//__no_init BYTE   P1_FRAMEPERIOD   @0xb002;
//__no_init BYTE   P1_TIME_SCALE    @0xb003;
//__no_init BYTE   P2_CAM_SIG_SET   @0xb004;
//__no_init BYTE   P2_FRAMEPERIOD   @0xb005;
//__no_init BYTE   P2_TIME_SCALE    @0xb006;
//__no_init BYTE   FIRM_OP_SET1     @0xb008;
//__no_init BYTE   FIRM_OP_SET2     @0xb009;
//__no_init WORD   P1_LEDON16       @0xb00a; //Pc Para 16bit data
//__no_init WORD   P2_LEDON16       @0xb00c; //Pc Para 16bit data
//__no_init WORD   Firmwarre_Ver    @0xb00e;
//
//__no_init BYTE   INT_REG_STATUS    @0xe000;
//__no_init BYTE   P1_CTRLDATA16_ADDR @0xf000;
//__no_init WORD   PC_DATA16BIT1    @0xf001; //Pc Para 16bit data
//__no_init BYTE   P2_CTRLDATA16_ADDR @0xf005;
//__no_init WORD   PC_DATA16BIT2    @0xf006; //Pc Para 16bit data
//__no_init BYTE   SYNC_ALARM_CODE  @0xf00a;
//__no_init BYTE   PARA_DATA_STATUS @0xf00b;
//__no_init BYTE   ADDR_TO_READ     @0xf00d; 
//__no_init BYTE   DATAL_TO_READ    @0xf00e;
//__no_init BYTE   DATAH_TO_READ    @0xf00f;





/////////////////////////////////////////////////////////////////////	

void port_ini(void);
void delay(WORD del);
void int0_enable(void);
void int0_disable(void);
void int1_enable(void);
void int1_disable(void);
void device_ini(void);
void DA_Conversion(BYTE reg,BYTE nValue_h,BYTE nValue_l);
void Out_da_value(void);
void ILL_Control(BYTE led_ch,BYTE led_value);
void ILL_2D_ON(void);
void ILL_2D_OFF(void);
void Set_2D_Level(BYTE number);
void Pzt_ini(void);
void Measure_start(void);
void LED_2D_255_SET(void);
void P1_LED_2D_Each_Ch_On(BYTE index);
void P2_LED_2D_Each_Ch_On(BYTE index);
void P1_LED_2D_Off(WORD ch_sel_bit);
void P2_LED_2D_Off(WORD ch_sel_bit);
void P1_TimeBase_Status_Ini(void);
void P2_TimeBase_Status_Ini(void);
void P1_FrameUnit_Grab(BYTE Frame_Offset1);
void P2_FrameUnit_Grab(BYTE Frame_Offset2);
void MultiCam_FrameUnit_Grab(BYTE Frame_Offset);
void Time_Base_P1_Main(void);
void Time_Base_P2_Main(void);
void Time_Base_MultiCam_Measure(void);
void Sync_Para_Setting(void);
unsigned char PCI_Control(unsigned char ucOffset);
#endif



