#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ VisionInspectionStatisticsValue
{
public:
    VisionInspectionStatisticsValue();
    ~VisionInspectionStatisticsValue();

    void Init();
    void Clear();
    void ResetStatisticsValue();
    long GetObjSize();
    void SetObjSize(long nSize);
    void SetEachValue(CString strObjID, float fEachValue, BYTE bEachResult, BYTE bObjIsInvalid);
    void CalcStatisticsValue(BOOL bUseMin, BOOL bUseMax);
    BOOL GetObjEachData(
        long i_nIdx, CString& o_strObjID, float& o_fObjValue, BYTE& o_bObjResult, BYTE& o_bObjIsInvalid);
    CString GetObjEachObjID(long i_nIdx);

public:
    //특성 정보
    CString strGroupID;
    float fSpecRefValue;
    BOOL bCalcEnd;

    //결과 값
    float fMinValue;
    long nMinObjID;
    float fMaxValue;
    long nMaxObjID;
    float fWorstValue;
    long nWorstObjID;
    float fAvrValue;
    float fSTDEV;
    long nResult;

protected:
    std::vector<CString> vecstrObjID;
    std::vector<float> vecfObjEachValue;
    std::vector<BYTE> vecbObjEachResult;
    std::vector<BYTE> vecbObjIsInvalid;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const VisionInspectionStatisticsValue& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, VisionInspectionStatisticsValue& dst);
};
