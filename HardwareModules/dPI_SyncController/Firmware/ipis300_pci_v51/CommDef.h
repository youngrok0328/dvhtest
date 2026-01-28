#ifndef __COMM_DEFINE_HEADER_FILE__
#define __COMM_DEFINE_HEADER_FILE__

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
	MEASURE_START_HIGH_POS_GRAB,   // Start Measurement
	MEASURE_START_HIGH_POS_GRAB_WITHOUT_OUTPUT_ACQUISITION_DONE,
	COMMAND_CLEAR = 0xff,   // Turn Off All Illuminations
};

enum
{
	TR_COMMAND_LED_OFF = 0x1b,
	TR_COMMAND_LED_ON,
	TR_COMMAND_LED_SET,
	TR_COMMAND_MEASURE_START = 0x20,

};
// 
// 
// enum _AVR_RESPONS
// {
// 	RESPONSE_CH1_OK = 0x01,
// 	RESPONSE_CH2_OK = 0x02,
// };

enum CHIP_SELECT
{
	_CS0 = 0,
	_CS1,
};

enum CS1_OFFSET//kircheis_TRV
{
	_HAND0_WRITE = 0,
	_HAND1_WRITE,
	_HAND0_READ,
	_HAND1_READ,
};

/// AVR내부에 Chip(Page)마다 4개의 8Bit Register 존재.
enum CHIP_REGISTER
{
	_REG0 = 0,		/// Ch.1의 Address buffer
	_REG1,			/// Ch.1의 Data buffer
	_REG2,			/// Ch.2의 Address buffer
	_REG3,			/// Ch.2의 Data buffer	
	_REG_STATUS = 7,	//// 상태확인 용.
};

// Serial Data Download Sequence
enum
{
	SERIAL_DATA_LOW,
	SERIAL_DATA_HIGH,
	SERIAL_DATA_DATA,
};

// Control Data Definition
enum SideVision
{
	SIDE_ILLUM_FRONT_INTENSITY	= 0x0000,	// 16 Channel, 10 Frames
	SIDE_ILLUM_REAR_INTENSITY	= 0x00A0,		// 16 Channel, 10 Frames
	SIDE_ILLUM_COMMAND			= 0x0140,	// Illumination Command
	SIDE_ILLUM_FRAME_COUNT,						// 2D Frame Count ( 1 ~ 10 )				영훈 : Sync 쪽과 이름이 달라 표기만 해놓는다.
	SIDE_LED_ON_DURATIONCOLOR,					// LED On Duration (ms)					Sync : LED_ON_DURATION1
	SIDE_FRAME_PERIODCOLOR,						// Main Trigger Period						Sync : FRAME_PERIOD1
	SIDE_LED_ON_DURATIONMAIN,					// LED On Duration (ms)					Sync : LED_ON_DURATION2
	SIDE_FRAME_PERIODMAIN,						// Main Trigger Period						Sync : FRAME_PERIOD2
	SIDE_CAMERA_SEL,							        // Camera Selection(0: Stereo, 1:Main)	Sync : First Cam (0), Second Cam (1)


	SIDE_CONTROL_DATA_SIZE,
};

// Illumination Command List
enum
{
	COMMAND_FRONT_0 = 0x10,   // Turn On FRONT Frame - 0
	COMMAND_FRONT_1,          // Turn On FRONT Frame - 1
	COMMAND_FRONT_2,          // Turn On FRONT Frame - 2
	COMMAND_FRONT_3,          // Turn On FRONT Frame - 3
	COMMAND_FRONT_4,          // Turn On FRONT Frame - 4
	COMMAND_FRONT_5,          // Turn On FRONT Frame - 5
	COMMAND_FRONT_6,          // Turn On FRONT Frame - 6
	COMMAND_FRONT_7,          // Turn On FRONT Frame - 7
	COMMAND_FRONT_8,          // Turn On FRONT Frame - 8
	COMMAND_FRONT_9,          // Turn On FRONT Frame - 9
	COMMAND_REAR_0 = 0x20,    // Turn On REAR Frame - 0
	COMMAND_REAR_1,           // Turn On REAR Frame - 1
	COMMAND_REAR_2,           // Turn On REAR Frame - 2
	COMMAND_REAR_3,           // Turn On REAR Frame - 3
	COMMAND_REAR_4,           // Turn On REAR Frame - 4
	COMMAND_REAR_5,           // Turn On REAR Frame - 5
	COMMAND_REAR_6,           // Turn On REAR Frame - 6
	COMMAND_REAR_7,           // Turn On REAR Frame - 7
	COMMAND_REAR_8,           // Turn On REAR Frame - 8
	COMMAND_REAR_9,           // Turn On REAR Frame - 9
	SIDE_MEASURE_START = 0xf0,   // Start Measurement
	SIDE_MEASURE_START_FRONT = 0xf1,   // Turn Off All Illuminations
	SIDE_MEASURE_START_REAR = 0xf2,   // Turn Off All Illuminations
	SIDE_COMMAND_CLEAR = 0xff,   // Turn Off All Illuminations
};

enum DVH_VISION_CAM_TYPE
{
    DVH_DUAL_DEFUSED = 0x01, 
    DVH_DUAL_SPECULAR = 0x02,
    DVH_DUAL_INTEGRATED = 0x03,
};

enum DVH_VISION_COMMAND
{
    DVH_ILLUM_VALUE_CH01 = 0x00,
    DVH_ILLUM_VALUE_CH02,
    DVH_ILLUM_VALUE_CH03,
    DVH_ILLUM_VALUE_CH04,
    DVH_ILLUM_COMMAND = 0xa0, // Illumination Command
    DVH_CAMERA_TYPE = 0xb2,		// Camera Type
    DVH_STEP1_EXPOSE_H = 0xb7, // EXPOSURE
    DVH_STEP1_EXPOSE_L,
    DVH_STEP2_EXPOSE_H,
    DVH_STEP2_EXPOSE_L,
    OFF_2D_TIME = 0xC0,
    CH_ON_OFF_H1,
    CH_ON_OFF_L1,
    CH_ON_OFF_H2 = 0xC7,
    CH_ON_OFF_L2,
    SYNC_PARA_MONITORING = 0xFD,
};

enum DVH_VISION
{
    DVH_COMMAND_LED_OFF = 0x1b,
    DVH_COMMAND_LED_ON,
};

#endif
