#pragma once

#ifdef __DA_BASE_MODULE_EXPORT__
#define _DA_BASE_MODULE_API __declspec(dllexport)
#else
#define _DA_BASE_MODULE_API __declspec(dllimport)
#endif

struct _DA_BASE_MODULE_API VerifyIllumSpec
{
	VerifyIllumSpec();
	VerifyIllumSpec& operator=(const VerifyIllumSpec &Dst) = default;

	void Init();
	void Init(long nCh);//각 채널별 초기 값을 정해진 값에 맞게 세팅

	BYTE byRefChIllum;// 조명 검증 시 각 채널의 조명 설정 값

	float fRefAvrIntensity; // 기준 평균 밝기
	float fAvrIntenRatioSpecMin; // 각각의 조명의 Offset Spec
	float fAvrIntenRatioSpecMax;

	float fIllumUniformityRatioSpec;// 균일도 검증 Spec 
};

