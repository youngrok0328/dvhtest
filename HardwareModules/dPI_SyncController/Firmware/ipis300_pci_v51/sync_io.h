#ifndef __SYNC_IO_HEADER_FILE__
#define __SYNC_IO_HEADER_FILE__

#include <iom128.h>
#include <ina90.h>

#define		M_C_RST			PORTB_Bit0

#define		M_FEN_INT		PIND_Bit0	// FEN 신호 인터럽트 
#define		M_CPLD_INT		PIND_Bit1	// CPLD와의 통신을 위한 인터럽트
#define		M_INT2			PIND_Bit2	// 예비 인터럽트 나중을위해 준비
#define		M_INT3			PIND_Bit3	// 예비 인터럽트 나중을위해 준비

#define     M_3DLED         PORTD_Bit4  // 3D LED 용 포트
#define		M_STEREO		PORTD_Bit5	// Stereo trigger용 포트

#define		M_DA_CS			PORTE_Bit7	// D/A 선택 포트;
#define		M_DA_CLK		PORTE_Bit6	//
#define		M_DA_SDI		PORTE_Bit5	//
#define		M_DA_LOAD		PORTE_Bit4	//
#define		M_DA_RST		PORTE_Bit3	// D/A Converter 리셋 포트

#define		M_PZT_AMP		PORTE_Bit2	//


#endif
