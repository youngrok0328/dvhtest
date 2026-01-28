#include "stdafx.h"
#include "VerifyIllumSpec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void VerifyIllumSpec::Init()
{
	byRefChIllum = 130;

	fRefAvrIntensity = 100.f;
	fAvrIntenRatioSpecMin = -100.f;
	fAvrIntenRatioSpecMax = 30.f;

	fIllumUniformityRatioSpec = 80.f;
}

void VerifyIllumSpec::Init(long nCh)//각 채널별 초기 값을 정해진 값에 맞게 세팅
{
	fAvrIntenRatioSpecMin = -100.f;//여기는 공통 파라미터
	fAvrIntenRatioSpecMax = 30.f;
	fIllumUniformityRatioSpec = 80.f;

	if (nCh < 0 || nCh >11) //엉뚱한 채널이 입력되면 그냥 초기화
	{
		Init();
	}
	else
	{
		switch (nCh)
		{
		case 0:
			byRefChIllum = 192;
			fRefAvrIntensity = 50.f;
			break;
		case 1:
			byRefChIllum = 170;
			fRefAvrIntensity = 30.f;
			break;
		case 2:
			byRefChIllum = 171;
			fRefAvrIntensity = 40.f;
			break;
		case 3:
			byRefChIllum = 175;
			fRefAvrIntensity = 80.f;
			break;
		case 4:
			byRefChIllum = 181;
			fRefAvrIntensity = 45.f;
			break;
		case 5:
			byRefChIllum = 184;
			fRefAvrIntensity = 80.f;
			break;
		case 6:
			byRefChIllum = 170;
			fRefAvrIntensity = 110.f;
			break;
		case 7:
			byRefChIllum = 176;
			fRefAvrIntensity = 60.f;
			break;
		case 8:
			byRefChIllum = 163;
			fRefAvrIntensity = 70.f;
			break;
		case 9:
			byRefChIllum = 139;
			fRefAvrIntensity = 110.f;
			break;
		case 10:
			byRefChIllum = 0;
			fRefAvrIntensity = 0.f;
			break;
		case 11:
			byRefChIllum = 111;
			fRefAvrIntensity = 110.f;
			break;
		}
	}
}

VerifyIllumSpec::VerifyIllumSpec()
{
	Init();
}