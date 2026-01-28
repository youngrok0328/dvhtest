#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
#include <map>
#include <vector>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
struct SBallPixelQualityParam
{
    float fRingWidthBeginRatio;
    float fRingWidthEndRatio;
    long nReferenceIntensity;
    long nDarkPixelMinRatio;

    void Init()
    {
        fRingWidthBeginRatio = 0.f;
        fRingWidthEndRatio = 50.f;
        nReferenceIntensity = 200;
        nDarkPixelMinRatio = 5;
    };

    SBallPixelQualityParam()
    {
        Init();
    };

    SBallPixelQualityParam& operator=(const SBallPixelQualityParam Src)
    {
        fRingWidthBeginRatio = Src.fRingWidthBeginRatio;
        fRingWidthEndRatio = Src.fRingWidthEndRatio;
        nReferenceIntensity = Src.nReferenceIntensity;
        nDarkPixelMinRatio = Src.nDarkPixelMinRatio;

        return *this;
    };
};

class VisionInspectionBgaBallPixelQualityPara
{
public:
    VisionInspectionBgaBallPixelQualityPara(VisionProcessing& parent);
    ~VisionInspectionBgaBallPixelQualityPara(void);

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    ImageProcPara m_ImageProcMangePara;

    float m_fBallWidthRatio1st;
    float m_fBallWidthRatio2nd;
    float m_fBallWidthRatio3rd;
    float m_fBallWidthRatio4th;

    // 2nd Inspection
    BOOL m_use2ndInspection; // 2nd 검사 사용 유무 (딥러닝)
    CString m_str2ndInspCode; // 딥러닝 모델과 매칭을 위한 코드

    std::vector<SBallPixelQualityParam> m_vecsPixelQualityParam;
};
