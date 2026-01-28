#include "TypeDef.h"
#include "sync_io.h"
#include "variables.h"
#include "macro.h"
#include "CommDef.h"
#include "PZT.h"

void Wait_FEN(void);        // From Main.


void PZT_ON(BYTE nStep)
{
	switch(nStep)
	{
	case 0:
		int_value_h = Control_Data[LEVEL_PZT0_H];		
		int_value_l = Control_Data[LEVEL_PZT0_L];
		break;
	case 1:
		int_value_h = Control_Data[LEVEL_PZT1_H];		
		int_value_l = Control_Data[LEVEL_PZT1_L];
		break;
	case 2:
		int_value_h = Control_Data[LEVEL_PZT2_H];		
		int_value_l = Control_Data[LEVEL_PZT2_L];
		break;
	case 3:
		int_value_h = Control_Data[LEVEL_PZT3_H];		
		int_value_l = Control_Data[LEVEL_PZT3_L];
		break;
	}	

	DA_Conversion(Control_Data[SELECT_PZT], int_value_h, int_value_l);
}

void PZT_OFF(void)
{	
	DA_Conversion(Control_Data[SELECT_PZT], 0, 0);
}

#define		MDA_ADDRESS_SPACE	2
#define		MDA_DATA_SPACE		12

void DA_Conversion(BYTE reg, BYTE nValue_h, BYTE nValue_l)		
{
	BYTE	nCount;
	mda_temp_data = reg;
	
	pzt_value_h = nValue_h;
	pzt_value_l = nValue_l;
	
	M_DA_CS = 1;
	M_DA_LOAD = 0;
	
	//두 비트는 어드레스 비트이고 나중에 비트는 아무 상관없는 비트이다
	for(nCount = 0 ; nCount < MDA_ADDRESS_SPACE ; nCount++)
	{
		M_DA_CLK = 1;
		M_DA_SDI = ~mda_temp1;
		M_DA_CLK = 0;
	
		M_DA_CLK = 1;
		M_DA_SDI = ~mda_temp0;
		M_DA_CLK = 0;
	}
	
	// ad설정값 셋팅
	for(nCount = 0 ; nCount < MDA_DATA_SPACE ; nCount++)//0101 0101 0101 0111
	{
		M_DA_CLK = 1;
		M_DA_SDI = ~sdi_value;
		mda_temp_value <<= 1;
		M_DA_CLK = 0;
	}
	
	M_DA_CS = 0;
	M_DA_LOAD = 1;
	M_DA_LOAD = 0;
}

#define		DA_COUNT	5
void PZT_Init(void)
{
    const WORD da_volt_level[] = {0,816,1632,2448,4095};// 816 : 2v, 1632 : 4v, 2448 : 6v, 4095 : 10v
	BYTE i = 0;

	M_PZT_AMP = 0;
		   
    for(i = 0 ; i < DA_COUNT ; i++)
	{
		int_value = da_volt_level[i];	
		Wait_FEN();
		DA_Conversion(0, int_value_h, int_value_l);
	}
	
	int_value = da_volt_level[0];
	Wait_FEN();
	DA_Conversion(0,int_value_h,int_value_l);
	
	for(i = 0 ; i < DA_COUNT ; i++)
	{
		int_value = da_volt_level[i];
		Wait_FEN();
		DA_Conversion(1, int_value_h, int_value_l);
	}
	
	int_value = da_volt_level[0];	
	Wait_FEN();
	DA_Conversion(1, int_value_h, int_value_l);
}

