#ifndef __COMM_DEFINE_HEADER_FILE__
#define __COMM_DEFINE_HEADER_FILE__


enum
{
	SERIAL_ADD_FRONT=0x10,
	SERIAL_DATA_FRONT,
	SERIAL_ADD_REAR=0x20,
	SERIAL_DATA_REAR,
};

enum CHIP_SELECT
{
	_CS0 = 0,
	_CS1,
};

// Control Bytes
enum
{
	ILLUM_COMMAND = 0xa0,   // Illumination Command
	ILL_2D_COUNT,           // 2D Frame Count ( 1 ~ 10 )
	ILL_2D_HIGH_POS_GRAB_COUNT,	  // 2D High Pos Grab Frames      //ILL_BUF_COUNT,	    // Buffer Frame Count Between 2D & 3D Frames
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
	LED_3D_FG_ON_DURATION,    // LED 3D On Duration (ms)
	LED_3D_BG_ON_DURATION,     // LED 3D On Duration (ms)
	MAIN_CAM_FRAMEPERIOD,
	MAIN_CAM_TRIGGER_EDGEMODE,  //0x00 == Rising Edge || GloBal shutter,   0x01 == Falling Edge || Rolling Shutter 
	FRAME_PERIOD_OFFSET,//kircheis_CamTrans
	LED_2D_ON_DURATION_OFFSET,
};

// Illumination Command List
enum
{
	COMMAND_2D_0 = 0x10,    // Turn On 2D Frame - 0
	COMMAND_2D_1,           // Turn On 2D Frame - 1
	COMMAND_2D_2,           // Turn On 2D Frame - 2
	COMMAND_2D_3,           // Turn On 2D Frame - 3
	COMMAND_2D_4,           // Turn On 2D Frame - 4
	COMMAND_2D_5,           // Turn On 2D Frame - 5
	COMMAND_2D_6,           // Turn On 2D Frame - 6
	COMMAND_2D_7,           // Turn On 2D Frame - 7
	COMMAND_2D_8,           // Turn On 2D Frame - 8
	COMMAND_2D_9,           // Turn On 2D Frame - 9
	COMMAND_3D_LED,         // Turn On 3D LED
	MEASURE_START = 0xf0,   // Start Measurement
	MEASURE_START_WITHOUT_OUTPUT_ACQUISITION_DONE,
	COMMAND_OUTPUT_ACQUISITION_DONE,
	MEASURE_SECOND_START,
	COMMAND_CLEAR = 0xff,   // Turn Off All Illuminations
};
enum
{
	CHANNEL_CHECK = 0,    
	FRAME_DONE_CHECK,
};
#define     Rising_Edge       0
#define     Falling_Edge      1
#define     Rolling_Shutter   1
#define     Global_Shutter    0

#define     External_Trigger_Enable  1
#define     External_Trigger_Disable 0

#define     PWM_MODE      1
#define     PLUSE_WIDTH_MODE  0

#endif
