#ifndef		SYNC_RAM_HEADER
#define		SYNC_RAM_HEADER

// LED PORT CONTROL
#define		LEDW0		0x8000
#define		LEDW1		0x8004
#define		LEDW2		0x8008
#define		LEDW3		0x800c

#define		LED_CH_ADD	0x8000
#define		LED_VAL_ADD	0x8004
#define		LED_ADD_L	0x8008
#define		LED_ADD_H	0x800c

// DIO PORT CONTROL
#define		RCS0		0xa000
#define		RCS1		0xa004

#define		WCS0		0x9000
#define		WCS1		0x9004
#define		WCS0_R		0xa008
#define		WCS1_R		0xa00c

// PC Interface
#define     PC_ADD      0xf000
#define     PC_DATA     0xf004

// PC Interface
/************************************************
	CPLD와 MCU간의 데이터 통신용 어드레스로 
	PC와 MCU 사이의 중간 매체가 된다
	0xf001~0xf003까지 임의로 설정
**************************************************/
#define		D_READ		0xf001	


#endif
