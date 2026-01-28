/////// Ver B0.01 ///////


#include    "TypeDef.h"
#include  "sync_io.h"
#include	"sync_ram.h"
#include    "CommDef.h"
#include    "variables.h"
#include    "Timer.h"
#include    "macro.h"
#include    "Interrupt.h"
#include    "PZT.h"
#include    "Lighting.h"
#include    "DioDef.h"



BYTE		*Tri_Con_EMO = (BYTE *)0xb001;
WORD Time_daley=0;

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

	PORTF = 0xfd;
	DDRF = 0x05;        // 0000 0101
	
	FEN_RST=0;

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
        *Tri_Con_EMO=0;
}

void Test_op(void)
{
	WORD i = 0;
	WORD j = 0;
	M_3DLED=0;
	Control_Data[FRAME_PERIOD]=20;
	*Frame_Period_Data=Control_Data[FRAME_PERIOD];
	*Exposure_time=10;
	Sync_Option=0;
	Trigger_Mode=External_Trigger_Enable;
	Tri_Mode=Rising_Edge;
	Shutter_Mode=Global_Shutter;
	Control_Data[FRAME_TRIGGER_OPTION]=Sync_Option;
	*Trigger_Option_Data=Sync_Option;
	
	Control_Data[FRAME_PERIOD_OFFSET]=0;
	Control_Data[LED_2D_ON_DURATION_OFFSET]=0;
	
        *Tri_Con_EMO=0x02;
        FEN_RST=1;

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
	*Tri_Con_EMO=0;
	Trigger_Mode=External_Trigger_Disable;
	Control_Data[FRAME_TRIGGER_OPTION]=Sync_Option;
	*Trigger_Option_Data=Sync_Option;
	Ver_Val=0xB330;
	
}

void Test_op_Rear(void)
{
	WORD i = 0;
	WORD j = 0;
	M_3DLED=1;
	Control_Data[FRAME_PERIOD]=20;
	*Frame_Period_Data=Control_Data[FRAME_PERIOD];
	*Exposure_time=10;
	Sync_Option=0;
	Trigger_Mode=External_Trigger_Enable;
	Tri_Mode=Rising_Edge;
	Shutter_Mode=Global_Shutter;
	Control_Data[FRAME_TRIGGER_OPTION]=Sync_Option;
	*Trigger_Option_Data=Sync_Option;
	
	Control_Data[FRAME_PERIOD_OFFSET]=0;
	Control_Data[LED_2D_ON_DURATION_OFFSET]=0;
	
        *Tri_Con_EMO=0x02;
        FEN_RST=1;

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
	*Tri_Con_EMO=0;
	Trigger_Mode=External_Trigger_Disable;
	Control_Data[FRAME_TRIGGER_OPTION]=Sync_Option;
	*Trigger_Option_Data=Sync_Option;
}

void PWM_LED_MAIN(void)
{
    if(LatestCommand = Control_Data[ILLUM_COMMAND])
    {
			Control_Data[ILLUM_COMMAND] = 0;
			//Ver_Val=0xb000;
			

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
			        Timer_Reset();
				LED_2D_SET(LatestCommand & 0x0f);
				*Frame_Period_Data=Control_Data[FRAME_PERIOD]+Control_Data[FRAME_PERIOD_OFFSET];
			        *Exposure_time=Control_Data[LED_2D_ON_DURATION]+Control_Data[LED_2D_ON_DURATION_OFFSET];
			        LED_On_Time=Control_Data[LED_2D_ON_DURATION]+Control_Data[LED_2D_ON_DURATION_OFFSET];
				LIVE_2D_flag = 1;
				*Tri_Con_EMO=1;
				break;
			case COMMAND_3D_LED:			// Turn On 3D LED
				LIVE_3D_flag = 1;
				*Frame_Period_Data=Control_Data[FRAME_PERIOD];
				 LED_On_Time=Control_Data[LED_3D_FG_ON_DURATION];
				 if(LED_On_Time<Control_Data[LED_2D_ON_DURATION])
				 {
				     *Exposure_time=Control_Data[LED_2D_ON_DURATION];
				 }
				 else *Exposure_time=Control_Data[LED_3D_FG_ON_DURATION];
			       
			        *Tri_Con_EMO=1;
				break;
			case MEASURE_START:
			        *Tri_Con_EMO=0;
				ProcedureOnMeasure();
				break;
			case MEASURE_START_WITHOUT_OUTPUT_ACQUISITION_DONE:
			        *Tri_Con_EMO=0;
			//	ProcedureOnMeasure_WITHOUT_OUTPUT_ACQUISITION_DONE();
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
WORD  test_data;

void One_Grab(BYTE Frame_Index)
{
    
    M_3DLED=0;
    switch(Grab_State)
    {
        case CHANNEL_CHECK: 
               LED_ONOFF_S=1;
               if(Int2_Flag==0)
              {
                  M_PZT_AMP=0;
                  if(Channel_Index>=16){Grab_State=FRAME_DONE_CHECK;}
                  else 
                  {
                        if(Control_Data16[Frame_Index+Channel_Index])
                        {
                            Int2_Flag=1;
                           
                            test_data=Control_Data16[Frame_Index+Channel_Index];
                            Tem_data16.def_word=Control_Data16[Channel_Index+0xe0];
                            *Exposure_time=Tem_data16.ml[0];
                            
                         //  if(Frame_Index==0) LED_ON_Timer16=test_data*0.8;
                         //   else 
                            LED_ON_Timer16=test_data;
                            test_data=LED_ON_Timer16;
                            if(test_data==LED_ON_Timer16) Int2_Flag=1;
                            LED_2D_Only_One(Channel_Index);
                            M_PZT_AMP=1; 
                          
                        }
               /*         else
                        {
                            if(Frame_Index==0x10 && Control_Data16[Frame_Index+Channel_Index-1]){
                                 Int2_Flag=1;
                                test_data=Control_Data16[Frame_Index+Channel_Index-1];
                                LED_ON_Timer16=test_data;
                                test_data=LED_ON_Timer16;
                               if(test_data==LED_ON_Timer16) Int2_Flag=1;
                                LED_2D_Only_One(Channel_Index-1);
                                M_PZT_AMP=1;  
                             }
                            
                        }    */
                        Channel_Index++;
                  }
                  
              }
              break;
        case FRAME_DONE_CHECK:
        default :
             delay(1000);
             break;
    }
    
}
void ProcedureOnMeasure16(void)
{
    enum
	{
		ProcedureOnMeasure_BEGIN,
		ProcedureOnMeasure_LED_SET,
		ProcedureOnMeasure_LED_ON_OFF,
		ProcedureOnMeasure_DELAY,
		ProcedureOnMeasure_END
	};

	SequenceStep = ProcedureOnMeasure_BEGIN;

	// Watch-Dog Timer 를 가동한다.
	WatchDog_Start();

	while(1)
	{
		// ProcedureOnMeasure 를 1초 이상 실행하고 있는 경우에는 시퀀스를 강제로 종료한다.
		if(WatchDog_Count_ms > 10000)
		{
			SequenceStep = ProcedureOnMeasure_END;
		}

		switch(SequenceStep)
		{
		  case ProcedureOnMeasure_BEGIN:
	                  FEN_RST=0;
	                  *Frame_Period_Data=Control_Data[FRAME_PERIOD];
	                  *Exposure_time=Control_Data[LED_2D_ON_DURATION];
	                  
	                  Channel_Index=0;
	                  Main_Fram_Index=0;
	                  LED_Count=1;
	                  Int2_Flag=0;
	                  Grab_State=CHANNEL_CHECK;
	                  SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
	                  FEN_RST=1;
	                  Time_daley=0;
		       break;
		  case ProcedureOnMeasure_LED_SET: 
      	                  if(FRAME_PERIOD_SIGNAL==0)
	                  {
	                      FEN_RST=0;	                     
	                      Channel_Index=0;
	                      Int2_Flag=0;
	                      Grab_State=CHANNEL_CHECK;
	                      Time_daley=0;
	                      SequenceStep = ProcedureOnMeasure_DELAY;
	                  }
	                  
		       break;

		  case ProcedureOnMeasure_LED_ON_OFF: 
                        if(Grab_State==FRAME_DONE_CHECK && Int2_Flag==0)
                        {
                            
                            LED_ONOFF_S=0;
                            if(LED_Count < Control_Data[ILL_2D_COUNT])
			    {
				Main_Fram_Index=Main_Fram_Index+0x10;
				SequenceStep = ProcedureOnMeasure_LED_SET;
				LED_Count++;		
			    }
			    else 
			    {
	  	                SequenceStep = ProcedureOnMeasure_END;
			    }  
                        }
                        else 
                        {
                            One_Grab(Main_Fram_Index);
                        }
		       break;
		  case ProcedureOnMeasure_DELAY : 
		  
		        if(Time_daley > 130)
		        {
		              FEN_RST=1;
		              SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
		              Time_daley=0;
		        }else Time_daley++;
                        break;
		  case ProcedureOnMeasure_END :     
		  default :
			FEN_RST=0;
			LED_2D_OFF();
			LED_ONOFF_S=0;
			M_PZT_AMP=0;
			Int2_Flag=0;
			WatchDog_Reset();
			Timer_Reset();
			return;
		}	
	}

}
void ProcedureOnMeasure_Second_16(void)
{
    enum
	{
		ProcedureOnMeasure_BEGIN,
		ProcedureOnMeasure_LED_SET,
		ProcedureOnMeasure_LED_ON_OFF,
		ProcedureOnMeasure_DELAY,
		ProcedureOnMeasure_END
	};

	SequenceStep = ProcedureOnMeasure_BEGIN;

	// Watch-Dog Timer 를 가동한다.
	WatchDog_Start();

	while(1)
	{
		// ProcedureOnMeasure 를 1초 이상 실행하고 있는 경우에는 시퀀스를 강제로 종료한다.
		if(WatchDog_Count_ms > 10000)
		{
			SequenceStep = ProcedureOnMeasure_END;
		}

		switch(SequenceStep)
		{
		  case ProcedureOnMeasure_BEGIN:
	                  FEN_RST=0;
	                  *Frame_Period_Data=Control_Data[FRAME_PERIOD];
	                  *Exposure_time=Control_Data[LED_2D_ON_DURATION];
	                  
	                  Channel_Index=0;
	                  Main_Fram_Index=Control_Data[ILL_2D_COUNT];
	                  Main_Fram_Index=Main_Fram_Index<<4;
	                  LED_Count=1;
	                  Int2_Flag=0;
	                  Grab_State=CHANNEL_CHECK;
	                  SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
	                  FEN_RST=1;
	                  Time_daley=0;
		       break;
		  case ProcedureOnMeasure_LED_SET: 
      	                  if(FRAME_PERIOD_SIGNAL==0)
	                  {
	                      FEN_RST=0;	                     
	                      Channel_Index=0;
	                      Int2_Flag=0;
	                      Grab_State=CHANNEL_CHECK;
	                      Time_daley=0;
	                      SequenceStep = ProcedureOnMeasure_DELAY;
	                  }
	                  
		       break;

		  case ProcedureOnMeasure_LED_ON_OFF: 
                        if(Grab_State==FRAME_DONE_CHECK && Int2_Flag==0)
                        {
                            
                            LED_ONOFF_S=0;
                            if(LED_Count < Control_Data[ILL_BUF_COUNT])
			    {
				Main_Fram_Index=Main_Fram_Index+0x10;
				SequenceStep = ProcedureOnMeasure_LED_SET;
				LED_Count++;		
			    }
			    else 
			    {
	  	                SequenceStep = ProcedureOnMeasure_END;
			    }  
                        }
                        else 
                        {
                            One_Grab(Main_Fram_Index);
                        }
		       break;
		  case ProcedureOnMeasure_DELAY : 
		  
		        if(Time_daley > 130)
		        {
		              FEN_RST=1;
		              SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
		              Time_daley=0;
		        }else Time_daley++;
                        break;
		  case ProcedureOnMeasure_END :     
		  default :
			FEN_RST=0;
			LED_2D_OFF();
			LED_ONOFF_S=0;
			M_PZT_AMP=0;
			Int2_Flag=0;
			WatchDog_Reset();
			Timer_Reset();
			return;
		}	
	}

}

void One_Grab_Rear(BYTE Frame_Index)
{
    
    M_3DLED=1;
    switch(Grab_State)
    {
        case CHANNEL_CHECK: 
               LED_ONOFF_S=1;
               if(Int2_Flag==0)
              {
                  M_PZT_AMP=0;
                  if(Channel_Index>=16){Grab_State=FRAME_DONE_CHECK;}
                  else 
                  {
                        if(Control_Data16_Rear[Frame_Index+Channel_Index])
                        {
                            Int2_Flag=1;
                           
                            test_data=Control_Data16_Rear[Frame_Index+Channel_Index];
                            Tem_data16.def_word=Control_Data16_Rear[Channel_Index+0xe0];
                            *Exposure_time=Tem_data16.ml[0];
                            
                         //  if(Frame_Index==0) LED_ON_Timer16=test_data*0.8;
                         //   else 
                            LED_ON_Timer16=test_data;
                            test_data=LED_ON_Timer16;
                            if(test_data==LED_ON_Timer16) Int2_Flag=1;
                            LED_2D_Only_One(Channel_Index);
                            M_PZT_AMP=1; 
                          
                        }
               /*         else
                        {
                            if(Frame_Index==0x10 && Control_Data16[Frame_Index+Channel_Index-1]){
                                 Int2_Flag=1;
                                test_data=Control_Data16[Frame_Index+Channel_Index-1];
                                LED_ON_Timer16=test_data;
                                test_data=LED_ON_Timer16;
                               if(test_data==LED_ON_Timer16) Int2_Flag=1;
                                LED_2D_Only_One(Channel_Index-1);
                                M_PZT_AMP=1;  
                             }
                            
                        }    */
                        Channel_Index++;
                  }
                  
              }
              break;
        case FRAME_DONE_CHECK:
        default :
             delay(1000);
             break;
    }
    
}

void ProcedureOnMeasure16_Rear(void)
{
    enum
	{
		ProcedureOnMeasure_BEGIN,
		ProcedureOnMeasure_LED_SET,
		ProcedureOnMeasure_LED_ON_OFF,
		ProcedureOnMeasure_DELAY,
		ProcedureOnMeasure_END
	};

	SequenceStep = ProcedureOnMeasure_BEGIN;

	// Watch-Dog Timer 를 가동한다.
	WatchDog_Start();

	while(1)
	{
		// ProcedureOnMeasure 를 1초 이상 실행하고 있는 경우에는 시퀀스를 강제로 종료한다.
		if(WatchDog_Count_ms > 10000)
		{
			SequenceStep = ProcedureOnMeasure_END;
		}

		switch(SequenceStep)
		{
		  case ProcedureOnMeasure_BEGIN:
	                  FEN_RST=0;
	                  *Frame_Period_Data=Control_Data[FRAME_PERIOD];
	                  *Exposure_time=Control_Data[LED_2D_ON_DURATION];
	                  
	                  Channel_Index=0;
	                  Main_Fram_Index=0;
	                  LED_Count=1;
	                  Int2_Flag=0;
	                  Grab_State=CHANNEL_CHECK;
	                  SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
	                  FEN_RST=1;
	                  Time_daley=0;
		       break;
		  case ProcedureOnMeasure_LED_SET: 
      	                  if(FRAME_PERIOD_SIGNAL==0)
	                  {
	                      FEN_RST=0;	                     
	                      Channel_Index=0;
	                      Int2_Flag=0;
	                      Grab_State=CHANNEL_CHECK;
	                      Time_daley=0;
	                      SequenceStep = ProcedureOnMeasure_DELAY;
	                  }
	                  
		       break;

		  case ProcedureOnMeasure_LED_ON_OFF: 
                        if(Grab_State==FRAME_DONE_CHECK && Int2_Flag==0)
                        {
                            
                            LED_ONOFF_S=0;
                            if(LED_Count < Control_Data[ILL_2D_COUNT])
			    {
				Main_Fram_Index=Main_Fram_Index+0x10;
				SequenceStep = ProcedureOnMeasure_LED_SET;
				LED_Count++;		
			    }
			    else 
			    {
	  	                SequenceStep = ProcedureOnMeasure_END;
			    }  
                        }
                        else 
                        {
                            One_Grab_Rear(Main_Fram_Index);
                        }
		       break;
		  case ProcedureOnMeasure_DELAY : 
		  
		        if(Time_daley > 130)
		        {
		              FEN_RST=1;
		              SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
		              Time_daley=0;
		        }else Time_daley++;
                        break;
		  case ProcedureOnMeasure_END :     
		  default :
			FEN_RST=0;
			LED_2D_OFF();
			LED_ONOFF_S=0;
			M_PZT_AMP=0;
			Int2_Flag=0;
			WatchDog_Reset();
			Timer_Reset();
			return;
		}	
	}

}
void ProcedureOnMeasure_Second_16_Rear(void)
{
    enum
	{
		ProcedureOnMeasure_BEGIN,
		ProcedureOnMeasure_LED_SET,
		ProcedureOnMeasure_LED_ON_OFF,
		ProcedureOnMeasure_DELAY,
		ProcedureOnMeasure_END
	};

	SequenceStep = ProcedureOnMeasure_BEGIN;

	// Watch-Dog Timer 를 가동한다.
	WatchDog_Start();

	while(1)
	{
		// ProcedureOnMeasure 를 1초 이상 실행하고 있는 경우에는 시퀀스를 강제로 종료한다.
		if(WatchDog_Count_ms > 10000)
		{
			SequenceStep = ProcedureOnMeasure_END;
		}

		switch(SequenceStep)
		{
		  case ProcedureOnMeasure_BEGIN:
	                  FEN_RST=0;
	                  *Frame_Period_Data=Control_Data[FRAME_PERIOD];
	                  *Exposure_time=Control_Data[LED_2D_ON_DURATION];
	                  
	                  Channel_Index=0;
	                  Main_Fram_Index=Control_Data[ILL_2D_COUNT];
	                  Main_Fram_Index=Main_Fram_Index<<4;
	                  LED_Count=1;
	                  Int2_Flag=0;
	                  Grab_State=CHANNEL_CHECK;
	                  SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
	                  FEN_RST=1;
	                  Time_daley=0;
		       break;
		  case ProcedureOnMeasure_LED_SET: 
      	                  if(FRAME_PERIOD_SIGNAL==0)
	                  {
	                      FEN_RST=0;	                     
	                      Channel_Index=0;
	                      Int2_Flag=0;
	                      Grab_State=CHANNEL_CHECK;
	                      Time_daley=0;
	                      SequenceStep = ProcedureOnMeasure_DELAY;
	                  }
	                  
		       break;

		  case ProcedureOnMeasure_LED_ON_OFF: 
                        if(Grab_State==FRAME_DONE_CHECK && Int2_Flag==0)
                        {
                            
                            LED_ONOFF_S=0;
                            if(LED_Count < Control_Data[ILL_BUF_COUNT])
			    {
				Main_Fram_Index=Main_Fram_Index+0x10;
				SequenceStep = ProcedureOnMeasure_LED_SET;
				LED_Count++;		
			    }
			    else 
			    {
	  	                SequenceStep = ProcedureOnMeasure_END;
			    }  
                        }
                        else 
                        {
                            One_Grab_Rear(Main_Fram_Index);
                        }
		       break;
		  case ProcedureOnMeasure_DELAY : 
		  
		        if(Time_daley > 130)
		        {
		              FEN_RST=1;
		              SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
		              Time_daley=0;
		        }else Time_daley++;
                        break;
		  case ProcedureOnMeasure_END :     
		  default :
			FEN_RST=0;
			LED_2D_OFF();
			LED_ONOFF_S=0;
			M_PZT_AMP=0;
			Int2_Flag=0;
			WatchDog_Reset();
			Timer_Reset();
			return;
		}	
	}

}

void PULSE_WIDTH_LED_MAIN(void)
{
    if(LatestCommand16.def_word = Control_Data16[ILLUM_COMMAND])
    {
         if(LatestCommand16.ml[0]==LatestCommand16.ml[1])
         {
              M_3DLED=0;
              Control_Data16[ILLUM_COMMAND]=0;
              Control_Data[ILLUM_COMMAND] = 0;
              AllClearIllumination();
              Trigger_Mode=PLUSE_WIDTH_MODE;
              *Trigger_Option_Data=Sync_Option;
         //     Control_Data[LED_2D_ON_DURATION]=4;
         //     Control_Data[FRAME_PERIOD]=30;
              switch(LatestCommand16.ml[0])
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
			                    LED_Bffer_Add=LatestCommand16.ml[0]&0x0f;
			                    LED_Bffer_Add=LED_Bffer_Add<<4;
			                    LED_2D_255_SET();
			                    LIVE_2D_flag16=1;
			                  
			                    FEN_RST=0;
			                    M_PZT_AMP=0;
	                                    *Frame_Period_Data=Control_Data[FRAME_PERIOD];
	                                    *Exposure_time=Control_Data[LED_2D_ON_DURATION];
	                                    FEN_RST=1;
	                                    Channel_Index=0;
	                                    Grab_State=CHANNEL_CHECK;
			                    break;
			
			case MEASURE_START:
			                   ProcedureOnMeasure16();
			                    break;
			case MEASURE_SECOND_START : 
			                   ProcedureOnMeasure_Second_16();
			                    break;
              }
              
         }
    }
    else if(LIVE_2D_flag16)
    {
	One_Grab(LED_Bffer_Add);
	if(Grab_State==FRAME_DONE_CHECK && Int2_Flag==0)
	{
	    LED_ONOFF_S=0;
	    if(FRAME_PERIOD_SIGNAL==0)
	    {
	      FEN_RST=0;
	      *Frame_Period_Data=Control_Data[FRAME_PERIOD];
	      *Exposure_time=Control_Data[LED_2D_ON_DURATION];
	      FEN_RST=1;
	      Channel_Index=0;
	      Grab_State=CHANNEL_CHECK;
	    }
	}
    }
    
}

void PULSE_WIDTH_LED_REAR(void)
{
    if(LatestCommand16_Rear.def_word = Control_Data16_Rear[ILLUM_COMMAND])
    {
         if(LatestCommand16_Rear.ml[0]==LatestCommand16_Rear.ml[1])
         {
              M_3DLED=1;
              Control_Data16_Rear[ILLUM_COMMAND]=0;
              Control_Data[ILLUM_COMMAND] = 0;
              AllClearIllumination();
              Trigger_Mode=PLUSE_WIDTH_MODE;
              *Trigger_Option_Data=Sync_Option;
         //     Control_Data[LED_2D_ON_DURATION]=4;
         //     Control_Data[FRAME_PERIOD]=30;
              switch(LatestCommand16_Rear.ml[0])
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
			                    LED_Bffer_Add=LatestCommand16.ml[0]&0x0f;
			                    LED_Bffer_Add=LED_Bffer_Add<<4;
			                    LED_2D_255_SET();
			                    LIVE_2D_flag16=1;
			                  
			                    FEN_RST=0;
			                    M_PZT_AMP=0;
	                                    *Frame_Period_Data=Control_Data[FRAME_PERIOD];
	                                    *Exposure_time=Control_Data[LED_2D_ON_DURATION];
	                                    FEN_RST=1;
	                                    Channel_Index=0;
	                                    Grab_State=CHANNEL_CHECK;
			                    break;
			
			case MEASURE_START:
			                   ProcedureOnMeasure16();
			                    break;
			case MEASURE_SECOND_START : 
			                   ProcedureOnMeasure_Second_16();
			                    break;
              }
              
         }
    }
    else if(LIVE_2D_flag16)
    {
	One_Grab_Rear(LED_Bffer_Add);
	if(Grab_State==FRAME_DONE_CHECK && Int2_Flag==0)
	{
	    LED_ONOFF_S=0;
	    if(FRAME_PERIOD_SIGNAL==0)
	    {
	      FEN_RST=0;
	      *Frame_Period_Data=Control_Data[FRAME_PERIOD];
	      *Exposure_time=Control_Data[LED_2D_ON_DURATION];
	      FEN_RST=1;
	      Channel_Index=0;
	      Grab_State=CHANNEL_CHECK;
	    }
	}
    }
    
}
void main(void)
{
	device_ini();

	Test_op();
	Test_op_Rear();
	INT0_Disable();
	INT2_Enable();
	while(1)
	{
		PULSE_WIDTH_LED_MAIN();
		PULSE_WIDTH_LED_REAR();
		
	}
}

#define  Delay_Time 10
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
			if(FRAME_PERIOD_SIGNAL==0)
			{
			  //*Dio_write_l = (*Dio_w_read_l & (~_OUTPUT_ACQUISITION_DONE));
			  M_PZT_AMP = Control_Data[PZT_DAC_MODE] ? 1 : 0;
			  LED_Count = 0;
			  BUF_Count = 0;
			  PZT_FG_Count = 0;
			  PZT_BG_Count = 0;
			  LED_2D_SET(LED_Count++);
			  PZT_ON(PZT_FG_Count++);
			  SequenceStep = ProcedureOnMeasure_LED_SET;
			  *Tri_Con_EMO=1;
			 }
			break;
		case ProcedureOnMeasure_LED_SET: 
			if(FRAME_PERIOD_SIGNAL==0)
			{
			  FEN_RST=0;
			  SequenceStep = ProcedureOnMeasure_LED_ON_OFF;
			  FEN_Flag = 0;	// 첫 프레임 밝기 변하는 문제 때문에 위치 변경함
			 
			  LED_2D_ON();
			  LED_ONOFF_S=1;
			 
			  if(LED_Count==Control_Data[ILL_2D_COUNT])
			  {
			    *Frame_Period_Data=Control_Data[FRAME_PERIOD]+Control_Data[FRAME_PERIOD_OFFSET];
			    LED_On_Time=Control_Data[LED_2D_ON_DURATION]+Control_Data[LED_2D_ON_DURATION_OFFSET];
			    *Exposure_time= LED_On_Time;
			  }
			  else 
			  {			    
			    *Frame_Period_Data=Control_Data[FRAME_PERIOD];
			    *Exposure_time=Control_Data[LED_2D_ON_DURATION];
			    LED_On_Time=Control_Data[LED_2D_ON_DURATION];
			  }
			  
			  Timer_Start();
			  M_STEREO = 1;
			  FEN_RST=1;
			 }
			break;

		case ProcedureOnMeasure_LED_ON_OFF: 
			if(Timer_Count_ms >= LED_On_Time)
			{
				LED_2D_OFF();
				Timer_Reset();
				LED_ONOFF_S=0;
				
				if(LED_Count < Control_Data[ILL_2D_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_LED_SET;
					LED_2D_SET(LED_Count++);		
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
				if(FRAME_PERIOD_SIGNAL==0)
				{
				 
				 FEN_RST=0;
				  LED_2D_OFF();
				  
				  *Frame_Period_Data=Control_Data[FRAME_PERIOD];
			          *Exposure_time=Control_Data[LED_2D_ON_DURATION];
				  Timer_Start();
				  BUF_Count++;
				 
				  SequenceStep = ProcedureOnMeasure_BUF_ON_OFF;
				  FEN_RST=1;
				  LED_ONOFF_S=1;
				}
				
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_PZT_FG_SET;
			}
			break;

		case ProcedureOnMeasure_BUF_ON_OFF: 
		
			if(Timer_Count_ms >= Control_Data[LED_2D_ON_DURATION])
			{
				Timer_Reset();
				LED_ONOFF_S=0;

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
				if(FRAME_PERIOD_SIGNAL==0)
				{
				  
				  FEN_RST=0;
				  LED_3D_ON();
				  *Frame_Period_Data=Control_Data[FRAME_PERIOD];
				  LED_On_Time=Control_Data[LED_3D_FG_ON_DURATION];
				  
				  if(LED_On_Time < Control_Data[LED_2D_ON_DURATION])
				  {
				      LED_On_Time=Control_Data[LED_2D_ON_DURATION];
				      *Exposure_time=LED_On_Time;
				  }
				  else *Exposure_time=LED_On_Time;
				  Timer_Start();
				  SequenceStep = ProcedureOnMeasure_PZT_FG_LED_ON_OFF;
				  FEN_RST=1;
				  LED_ONOFF_S=1;
				}							
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_PZT_BG_SET;
			}
			break;

		case ProcedureOnMeasure_PZT_FG_LED_ON_OFF:
		
			if(Timer_Count_ms >= Control_Data[LED_3D_FG_ON_DURATION])
			{
				LED_3D_OFF();
				Timer_Reset();
				LED_ONOFF_S=0;

				if(PZT_FG_Count < Control_Data[ILL_3D_FG_COUNT])
				{
					SequenceStep = ProcedureOnMeasure_PZT_FG_SET;
					PZT_ON(PZT_FG_Count++);		
				}
				else 
				{
					PZT_OFF();
					SequenceStep = ProcedureOnMeasure_PZT_BG_SET;
					PZT_ON(PZT_BG_Count++);
				}
			}	
			break;

		case ProcedureOnMeasure_PZT_BG_SET:
			if(Control_Data[ILL_3D_BG_COUNT] > 0)
			{
				if(FRAME_PERIOD_SIGNAL==0)
				{
                                  
				  FEN_RST=0;
				  LED_3D_ON();
				  *Frame_Period_Data=Control_Data[FRAME_PERIOD];
			          LED_On_Time=Control_Data[LED_3D_BG_ON_DURATION];
				  if(LED_On_Time < Control_Data[LED_2D_ON_DURATION])
				  {
				      LED_On_Time=Control_Data[LED_2D_ON_DURATION];
				      *Exposure_time=LED_On_Time;
				  }
				  else *Exposure_time=LED_On_Time;
				   Timer_Start();
				  SequenceStep = ProcedureOnMeasure_PZT_BG_LED_ON_OFF;
				   FEN_RST=1;
				   LED_ONOFF_S=1;	
				}						
			}
			else
			{
				SequenceStep = ProcedureOnMeasure_END;
			}
			break;

		case ProcedureOnMeasure_PZT_BG_LED_ON_OFF:
		
			if(Timer_Count_ms >= Control_Data[LED_3D_BG_ON_DURATION])
			{
				LED_3D_OFF();
				Timer_Reset();
				LED_ONOFF_S=0;

				if(PZT_BG_Count < Control_Data[ILL_3D_BG_COUNT])
				{
					PZT_ON(PZT_BG_Count++);
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
			//*Dio_write_l = (*Dio_w_read_l | _OUTPUT_ACQUISITION_DONE);
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
