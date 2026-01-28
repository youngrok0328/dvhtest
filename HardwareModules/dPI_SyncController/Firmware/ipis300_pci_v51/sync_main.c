#include    "TypeDef.h"
#include	"sync_io.h"
#include	"sync_ram.h"
#include    "CommDef.h"
#include    "variables.h"
#include    "Timer.h"
#include    "macro.h"
#include    "Interrupt.h"
#include    "PZT.h"
#include    "Lighting.h"
#include    "DioDef.h"

void port_ini(void);
void delay(WORD del);
void device_ini(void);
void Wait_FEN(void);
void ProcedureOnMeasure(void);
void ProcedureOnMeasure_WITHOUT_OUTPUT_ACQUISITION_DONE(void);
void Only_OUTPUT_ACQUISITION_DONE(void);

//============================================================================
//== Global Variables



void port_ini(void)
{
	DDRB = 0x01;
	PORTB = 0xC0;

	DDRD = 0x30;	//0011 0000
	PORTD= 0x00;	//0000 0000

	DDRE = 0xfc;	//1111 1100
	PORTE = 0x00;

	PORTF = 0x00;
	DDRF = 0x00;        // Input Port

	M_DA_CS = 0;
	M_DA_LOAD = 1;
	M_DA_CLK = 0;
	M_DA_RST =1;

	M_PZT_AMP = 0;
	M_STEREO = 0;
	M_3DLED = 0;
}


void delay(WORD del)
{
	while(--del)asm("NOP");
}

//LED와 DIO의 제어를 위해 어드레스 활당하는 함수
void device_ini(void)
{
	_CLI();

	port_ini();

	MCUCR |= 0x80;    // Set SRE bit (External Memory Enable)
	XMCRA = 0x40;     // SRL : 100(0x1100 - 0x7FFF, 0x8000 - 0xFFFF)
	XMCRB = 0x00;     // Use Full 60K Memory

	_SEI();

	Init_Interrupt();
	INT0_Enable();
	INT1_Enable();

	Init_Timers();

	M_C_RST = 1;
}

void Test_op(void)
{
	WORD i = 0;
	WORD j = 0;

	for(i = 0 ; i < 256 ; i++)
	{
		Control_Data[i] = 0;
	}

	Control_Data[LED_2D_ON_DURATION] = 5;
	Control_Data[LED_3D_FG_ON_DURATION] = 5;
	Control_Data[LED_3D_BG_ON_DURATION] = 5;

	M_STEREO = 1;

	LED_2D_OFF();

	for(i = 0 ; i < 16 ; i++)
	{
		LED_2D_TRANSFER(i, 0);
	}

	M_STEREO = 0;

	// 2D LED Operation Test
	for(j = 0 ; j < 10 ; j++)
	{
		for(i = 0 ; i < 16 ; i++)
		{
			LED_2D_TRANSFER(i, 255);

			Timer_Start();
			LED_2D_ON();			
			while(Timer_Count_ms < Control_Data[LED_2D_ON_DURATION]);
			Timer_Reset();

			Timer_Start();
			LED_2D_OFF();
			while(Timer_Count_ms < Control_Data[LED_2D_ON_DURATION]);
			Timer_Reset();

			LED_2D_TRANSFER(i, 0);
		}
	}

	// 3D LED Test with FEN
	FEN_Flag = 0;

	for(j = 0 ; j < 100 ; j++)
	{
		while(FEN_Flag==0);
		FEN_Flag = 0;

		Timer_Start();
		LED_3D_ON();			

		while(Timer_Count_ms < Control_Data[LED_3D_FG_ON_DURATION]);

		LED_3D_OFF();
		Timer_Reset();

		while(FEN_Flag==0);
		FEN_Flag = 0;
	}

	// 2D LED Test with FEN
	for(j = 0 ; j < 5 ; j++)
	{
		for(i = 0 ; i < 16 ; i++)
		{
			LED_2D_TRANSFER(i, 255);

			while(FEN_Flag==0);
			FEN_Flag = 0;

			Timer_Start();
			LED_2D_ON();			

			while(Timer_Count_ms < Control_Data[LED_2D_ON_DURATION]);

			LED_2D_OFF();
			Timer_Reset();

			while(FEN_Flag==0);
			FEN_Flag = 0;

			LED_2D_TRANSFER(i, 0);
		}
	}

	PZT_Init();
}

void main(void)
{
	device_ini();

	Test_op();

	while(1)
	{
		if(LatestCommand = Control_Data[ILLUM_COMMAND])
		{
			Control_Data[ILLUM_COMMAND] = 0;

			// 현재 수행하는 게 뭐든지 그냥 꺼버린다.
			AllClearIllumination();

			// 지령에 맞는 프레임을 세팅함
			switch(LatestCommand)
			{
			case COMMAND_2D_0:				// Turn On 2D - Frame 0
			case COMMAND_2D_1:				// Turn On 2D - Frame 1
			case COMMAND_2D_2:				// Turn On 2D - Frame 2
			case COMMAND_2D_3:				// Turn On 2D - Frame 3
			case COMMAND_2D_4:				// Turn On 2D - Frame 4
			case COMMAND_2D_5:				// Turn On 2D - Frame 5
			case COMMAND_2D_6:				// Turn On 2D - Frame 6
			case COMMAND_2D_7:				// Turn On 2D - Frame 7
			case COMMAND_2D_8:				// Turn On 2D - Frame 8
			case COMMAND_2D_9:				// Turn On 2D - Frame 9
				LED_2D_SET(LatestCommand & 0x0f);
				LIVE_2D_flag = 1;
				break;
			case COMMAND_3D_LED:			// Turn On 3D LED
				LIVE_3D_flag = 1;
				break;
			case MEASURE_START:
				ProcedureOnMeasure();
				break;
			case MEASURE_START_WITHOUT_OUTPUT_ACQUISITION_DONE:
				ProcedureOnMeasure_WITHOUT_OUTPUT_ACQUISITION_DONE();
				break;
			case COMMAND_OUTPUT_ACQUISITION_DONE:
				Only_OUTPUT_ACQUISITION_DONE();
				break;
			}
		}
		else if(LIVE_2D_flag)
		{
			LED_2D_ON_OFF();
		}
		else if(LIVE_3D_flag)
		{
			LED_3D_ON_OFF();
		}
	}
}

void ProcedureOnMeasure(void)
{
	enum
	{
		ProcedureOnMeasure_BEGIN,
		ProcedureOnMeasure_LED_SET,
		ProcedureOnMeasure_LED_ON_OFF,
		ProcedureOnMeasure_BUF_SET,
		ProcedureOnMeasure_BUF_ON_OFF,
		ProcedureOnMeasure_PZT_FG_SET,
		ProcedureOnMeasure_PZT_FG_LED_ON_OFF,
		ProcedureOnMeasure_PZT_BG_SET,
		ProcedureOnMeasure_PZT_BG_LED_ON_OFF,
		ProcedureOnMeasure_END
	};

	SequenceStep = ProcedureOnMeasure_BEGIN;

	// Watch-Dog Timer 를 가동한다.
	WatchDog_Start();

	while(1)
	{
		// ProcedureOnMeasure 를 1초 이상 실행하고 있는 경우에는 시퀀스를 강제로 종료한다.
		if(WatchDog_Count_ms > 1000)
		{
			SequenceStep = ProcedureOnMeasure_END;
		}

		switch(SequenceStep)
		{
		case ProcedureOnMeasure_BEGIN:
			*Dio_write_l = (*Dio_w_read_l & (~_OUTPUT_ACQUISITION_DONE));
			M_PZT_AMP = Control_Data[PZT_DAC_MODE] ? 1 : 0;
			LED_Count = 0;
			BUF_Count = 0;
			PZT_FG_Count = 0;
			PZT_BG_Count = 0;
			SequenceStep = ProcedureOnMeasure_LED_SET;
			break;

		case ProcedureOnMeasure_LED_SET:
			LED_2D_SET(LED_Count++);
			SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
			FEN_Flag = 0;	// 첫 프레임 밝기 변하는 문제 때문에 위치 변경함
			break;

		case ProcedureOnMeasure_LED_ON_OFF: 
			if(FEN_Flag)
			{
				FEN_Flag = 0;

				LED_2D_ON();
				Timer_Start();
				M_STEREO = 1;
			}
			else if(Timer_Count_ms >= Control_Data[LED_2D_ON_DURATION])
			{
				LED_2D_OFF();
				Timer_Reset();

				if(LED_Count < Control_Data[ILL_2D_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_LED_SET;		
				}
				else
				{
					SequenceStep = ProcedureOnMeasure_BUF_SET;
				}
			}	
			break;

		case ProcedureOnMeasure_BUF_SET:
			if(Control_Data[ILL_BUF_COUNT] > 0)
			{
				BUF_Count++;
				SequenceStep = ProcedureOnMeasure_BUF_ON_OFF;
				FEN_Flag = 0;	// 첫 프레임 밝기 변하는 문제 때문에 위치 변경함
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_PZT_FG_SET;
			}
			break;

		case ProcedureOnMeasure_BUF_ON_OFF: 
			if(FEN_Flag)
			{
				FEN_Flag = 0;

				Timer_Start();
			}
			else if(Timer_Count_ms >= Control_Data[LED_2D_ON_DURATION])
			{
				Timer_Reset();

				if(BUF_Count < Control_Data[ILL_BUF_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_BUF_SET;		
				}
				else
				{
					SequenceStep = ProcedureOnMeasure_PZT_FG_SET;
				}
			}	
			break;

		case ProcedureOnMeasure_PZT_FG_SET:
			if(Control_Data[ILL_3D_FG_COUNT] > 0)
			{
				PZT_ON(PZT_FG_Count++);

				SequenceStep = ProcedureOnMeasure_PZT_FG_LED_ON_OFF;							
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_PZT_BG_SET;
			}
			break;

		case ProcedureOnMeasure_PZT_FG_LED_ON_OFF:
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

				if(PZT_FG_Count < Control_Data[ILL_3D_FG_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_PZT_FG_SET;		
				}
				else 
				{
					PZT_OFF();
					SequenceStep = ProcedureOnMeasure_PZT_BG_SET;
				}
			}	
			break;

		case ProcedureOnMeasure_PZT_BG_SET:
			if(Control_Data[ILL_3D_BG_COUNT] > 0)
			{
				PZT_ON(PZT_BG_Count++);

				SequenceStep = ProcedureOnMeasure_PZT_BG_LED_ON_OFF;							
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_END;
			}
			break;

		case ProcedureOnMeasure_PZT_BG_LED_ON_OFF:
			if(FEN_Flag)
			{
				FEN_Flag = 0;

				LED_3D_ON();
				Timer_Start();
			}
			else if(Timer_Count_ms >= Control_Data[LED_3D_BG_ON_DURATION])
			{
				LED_3D_OFF();
				Timer_Reset();

				if(PZT_BG_Count < Control_Data[ILL_3D_BG_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_PZT_BG_SET;		
				}
				else 
				{
					PZT_OFF();
					SequenceStep = ProcedureOnMeasure_END;
				}
			}	
			break;

		case ProcedureOnMeasure_END :
		default :
			*Dio_write_l = (*Dio_w_read_l | _OUTPUT_ACQUISITION_DONE);
			M_STEREO = 0;
			LED_2D_OFF();
			LED_3D_OFF();
			WatchDog_Reset();
			Timer_Reset();
			return;
		}	
	}
}

void ProcedureOnMeasure_WITHOUT_OUTPUT_ACQUISITION_DONE(void)
{
	enum
	{
		ProcedureOnMeasure_BEGIN,
		ProcedureOnMeasure_LED_SET,
		ProcedureOnMeasure_LED_ON_OFF,
		ProcedureOnMeasure_BUF_SET,
		ProcedureOnMeasure_BUF_ON_OFF,
		ProcedureOnMeasure_PZT_FG_SET,
		ProcedureOnMeasure_PZT_FG_LED_ON_OFF,
		ProcedureOnMeasure_PZT_BG_SET,
		ProcedureOnMeasure_PZT_BG_LED_ON_OFF,
		ProcedureOnMeasure_END
	};

	SequenceStep = ProcedureOnMeasure_BEGIN;

	// Watch-Dog Timer 를 가동한다.
	WatchDog_Start();

	while(1)
	{
		// ProcedureOnMeasure 를 1초 이상 실행하고 있는 경우에는 시퀀스를 강제로 종료한다.
		if(WatchDog_Count_ms > 1000)
		{
			SequenceStep = ProcedureOnMeasure_END;
		}

		switch(SequenceStep)
		{
		case ProcedureOnMeasure_BEGIN:
			*Dio_write_l = (*Dio_w_read_l & (~_OUTPUT_ACQUISITION_DONE));
			M_PZT_AMP = Control_Data[PZT_DAC_MODE] ? 1 : 0;
			LED_Count = 0;
			BUF_Count = 0;
			PZT_FG_Count = 0;
			PZT_BG_Count = 0;
			SequenceStep = ProcedureOnMeasure_LED_SET;
			break;

		case ProcedureOnMeasure_LED_SET:
			LED_2D_SET(LED_Count++);
			SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
			FEN_Flag = 0;	// 첫 프레임 밝기 변하는 문제 때문에 위치 변경함
			break;

		case ProcedureOnMeasure_LED_ON_OFF: 
			if(FEN_Flag)
			{
				FEN_Flag = 0;

				LED_2D_ON();
				Timer_Start();
				M_STEREO = 1;
			}
			else if(Timer_Count_ms >= Control_Data[LED_2D_ON_DURATION])
			{
				LED_2D_OFF();
				Timer_Reset();

				if(LED_Count < Control_Data[ILL_2D_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_LED_SET;		
				}
				else
				{
					SequenceStep = ProcedureOnMeasure_BUF_SET;
				}
			}	
			break;

		case ProcedureOnMeasure_BUF_SET:
			if(Control_Data[ILL_BUF_COUNT] > 0)
			{
				BUF_Count++;
				SequenceStep = ProcedureOnMeasure_BUF_ON_OFF;
				FEN_Flag = 0;	// 첫 프레임 밝기 변하는 문제 때문에 위치 변경함
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_PZT_FG_SET;
			}
			break;

		case ProcedureOnMeasure_BUF_ON_OFF: 
			if(FEN_Flag)
			{
				FEN_Flag = 0;

				Timer_Start();
			}
			else if(Timer_Count_ms >= Control_Data[LED_2D_ON_DURATION])
			{
				Timer_Reset();

				if(BUF_Count < Control_Data[ILL_BUF_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_BUF_SET;		
				}
				else
				{
					SequenceStep = ProcedureOnMeasure_PZT_FG_SET;
				}
			}	
			break;

		case ProcedureOnMeasure_PZT_FG_SET:
			if(Control_Data[ILL_3D_FG_COUNT] > 0)
			{
				PZT_ON(PZT_FG_Count++);

				SequenceStep = ProcedureOnMeasure_PZT_FG_LED_ON_OFF;							
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_PZT_BG_SET;
			}
			break;

		case ProcedureOnMeasure_PZT_FG_LED_ON_OFF:
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

				if(PZT_FG_Count < Control_Data[ILL_3D_FG_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_PZT_FG_SET;		
				}
				else 
				{
					PZT_OFF();
					SequenceStep = ProcedureOnMeasure_PZT_BG_SET;
				}
			}	
			break;

		case ProcedureOnMeasure_PZT_BG_SET:
			if(Control_Data[ILL_3D_BG_COUNT] > 0)
			{
				PZT_ON(PZT_BG_Count++);

				SequenceStep = ProcedureOnMeasure_PZT_BG_LED_ON_OFF;							
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_END;
			}
			break;

		case ProcedureOnMeasure_PZT_BG_LED_ON_OFF:
			if(FEN_Flag)
			{
				FEN_Flag = 0;

				LED_3D_ON();
				Timer_Start();
			}
			else if(Timer_Count_ms >= Control_Data[LED_3D_BG_ON_DURATION])
			{
				LED_3D_OFF();
				Timer_Reset();

				if(PZT_BG_Count < Control_Data[ILL_3D_BG_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_PZT_BG_SET;		
				}
				else 
				{
					PZT_OFF();
					SequenceStep = ProcedureOnMeasure_END;
				}
			}	
			break;

		case ProcedureOnMeasure_END :
		default :
			*Dio_write_l = (*Dio_w_read_l);
			M_STEREO = 0;
			LED_2D_OFF();
			LED_3D_OFF();
			WatchDog_Reset();
			Timer_Reset();
			return;
		}	
	}
}

void Only_OUTPUT_ACQUISITION_DONE(void)
{
	*Dio_write_l = (*Dio_w_read_l | _OUTPUT_ACQUISITION_DONE);
}