#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class _DA_BASE_MODULE_API BlobAttribute
{
public:
    BlobAttribute();
    ~BlobAttribute();

    void Clear();

    Ipvm::Point32r2 m_massCenter;

    //-----------------------------------------------------------
    // 2D 항목
    //-----------------------------------------------------------

    float m_localAreaPercent_All;
    float m_localAreaPercent;
    float m_keyContrast; // Diff. From Average
    float m_averageContrast;
    float m_localContrast;
    float m_defectROIRatio;
    float m_auxLocalContrast;
    float m_deviation;
    float m_auxDeviation;
    float m_width; // 블럽 Axis 에 대한 단축 방향 길이
    float m_length; // 블럽 Axis 에 대한 장축 방향 길이
    float m_lengthX; // 블럽 Axis 에 대한 장축 방향 길이
    float m_lengthY; // 블럽 Axis 에 대한 장축 방향 길이
    float m_totalLength; // 영훈 : 블럽의 모든 길이를 더한 값

    float m_thickness; // 블럽의 두께
    float m_locus; // 블럽의 궤적

    float m_edgeEnergy;

    float m_fMergedCount; // 블럽의 Merged Count
    float m_fBlobGV_AVR; // 블럽의 평균 GV 값
    float m_fBlobGV_Min; // 블럽의 Min GV 값
    float m_fBlobGV_Max; // 블럽의 Max GV 값

    //-----------------------------------------------------------
    // 3D 항목
    //-----------------------------------------------------------

    float m_averageDeltaHeight;
    float m_keyDeltaHeight;

    //-----------------------------------------------------------
    // 공통 항목
    //-----------------------------------------------------------

    enum enBrightDarkType
    {
        DARK,
        BRIGHT,
    };
    enBrightDarkType m_blobType; // DARK, BRIGHT

    __forceinline static void ResetArray(BlobAttribute attributes[], long nCount)
    {
        for (long n = 0; n < nCount; n++)
        {
            attributes[n].Clear();
        }
    };
};
