//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionStatisticsValue.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <math.h>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionStatisticsValue::VisionInspectionStatisticsValue()
{
    Init();
}

VisionInspectionStatisticsValue::~VisionInspectionStatisticsValue()
{
    Init();
}

void VisionInspectionStatisticsValue::Init()
{
    strGroupID.Empty();
    fSpecRefValue = 0.f;
    ResetStatisticsValue();
    Clear();
    bCalcEnd = FALSE;
}

void VisionInspectionStatisticsValue::Clear()
{
    vecstrObjID.clear();
    vecfObjEachValue.clear();
    vecbObjEachResult.clear();
    vecbObjIsInvalid.clear();
}

void VisionInspectionStatisticsValue::ResetStatisticsValue()
{
    fMinValue = fMaxValue = fAvrValue = fWorstValue = fSTDEV = 0.f;
    nResult = 0;
    bCalcEnd = FALSE;
}

long VisionInspectionStatisticsValue::GetObjSize()
{
    return (long)vecfObjEachValue.size();
}

void VisionInspectionStatisticsValue::SetObjSize(long nSize)
{
    Clear();
    vecstrObjID.resize(nSize);
    vecfObjEachValue.resize(nSize);
    vecbObjEachResult.resize(nSize);
    vecbObjIsInvalid.resize(nSize);
}

void VisionInspectionStatisticsValue::SetEachValue(
    CString strObjID, float fEachValue, BYTE bEachResult, BYTE bObjIsInvalid)
{
    vecstrObjID.push_back(strObjID);
    vecfObjEachValue.push_back(fEachValue);
    vecbObjEachResult.push_back(bEachResult);
    vecbObjIsInvalid.push_back(bObjIsInvalid);
    bCalcEnd = FALSE;
}

void VisionInspectionStatisticsValue::CalcStatisticsValue(BOOL bUseMin, BOOL bUseMax)
{
    const long nDataNum = (long)vecfObjEachValue.size();
    ResetStatisticsValue();
    if (nDataNum <= 0)
        return;

    fMinValue = 9999999.f;
    fMaxValue = -fMinValue;

    BOOL bUseMinMax = bUseMin && bUseMax;

    double dSum = 0.;
    float fDEV;
    float fWorstBuf(0.f), fAbsErrorValue(0.f);
    long nIdx(0), nWorstID(0);
    long nValidDataNum = 0;
    nResult = 0;
    for (auto fData : vecfObjEachValue)
    {
        nResult = (long)max(nResult, (long)vecbObjEachResult[nIdx]);
        if (vecbObjIsInvalid[nIdx] == 1)
        {
            nIdx++;
            continue;
        }
        if (fMinValue >= fData)
        {
            fMinValue = fData;
            nMinObjID = nIdx;
        }
        if (fMaxValue <= fData)
        {
            fMaxValue = fData;
            nMaxObjID = nIdx;
        }

        dSum += (double)fData;
        fAbsErrorValue = (float)fabs(fData - fSpecRefValue);
        if (fAbsErrorValue >= fWorstBuf)
        {
            fWorstBuf = fAbsErrorValue;
            nWorstID = nIdx;
        }
        nIdx++;
        nValidDataNum++;
    }

    if (bUseMinMax)
    {
        nWorstObjID = nWorstID;
        fWorstValue = vecfObjEachValue[nWorstID];
    }
    else if (bUseMin)
    {
        nWorstObjID = nMinObjID;
        fWorstValue = fMinValue;
    }
    else if (bUseMax)
    {
        nWorstObjID = nMaxObjID;
        fWorstValue = fMaxValue;
    }

    if (nValidDataNum < 1)
        return;
    fAvrValue = (float)(dSum / (double)nValidDataNum);
    bCalcEnd = TRUE;

    if (nValidDataNum <= 1)
        return;

    dSum = 0.;

    nIdx = -1;
    for (auto fData : vecfObjEachValue)
    {
        nIdx++;
        if (vecbObjIsInvalid[nIdx] == 1)
            continue;
        fDEV = fData - fAvrValue;
        dSum += (double)(fDEV * fDEV);
    }

    fSTDEV = (float)sqrt(dSum / (double)nValidDataNum);
    //fSTDEV = (float)sqrt(dSum / (double)(nValidDataNum-1.));//kircheis_VSV_Option // 위의 Code는 STDEVP(), 주석 처리된 Code는 STDEV()
}

BOOL VisionInspectionStatisticsValue::GetObjEachData(
    long i_nIdx, CString& o_strObjID, float& o_fObjValue, BYTE& o_bObjResult, BYTE& o_bObjIsInvalid)
{
    o_strObjID.Empty();
    o_fObjValue = 0.f;
    o_bObjResult = 0;
    if (i_nIdx < 0 || i_nIdx >= vecbObjEachResult.size())
        return FALSE;

    o_strObjID = vecstrObjID[i_nIdx];
    o_fObjValue = vecfObjEachValue[i_nIdx];
    o_bObjResult = vecbObjEachResult[i_nIdx];
    o_bObjIsInvalid = vecbObjIsInvalid[i_nIdx];

    return TRUE;
}

CArchive& operator<<(CArchive& ar, const VisionInspectionStatisticsValue& src)
{
    long nDataNum = (long)src.vecfObjEachValue.size();

    ar << src.strGroupID;
    ar << src.nResult;
    ar << src.fMinValue;
    ar << src.nMinObjID;
    ar << src.fMaxValue;
    ar << src.nMaxObjID;
    ar << src.fWorstValue;
    ar << src.nWorstObjID;
    ar << src.fAvrValue;
    ar << src.fSTDEV;

    ar << nDataNum;
    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
    {
        ar << src.vecstrObjID[nIdx] << src.vecfObjEachValue[nIdx] << src.vecbObjEachResult[nIdx]
           << src.vecbObjIsInvalid[nIdx];
    }

    return ar;
}

CArchive& operator>>(CArchive& ar, VisionInspectionStatisticsValue& dst)
{
    long nDataNum = 0;

    ar >> dst.strGroupID;
    ar >> dst.nResult;
    ar >> dst.fMinValue;
    ar >> dst.nMinObjID;
    ar >> dst.fMaxValue;
    ar >> dst.nMaxObjID;
    ar >> dst.fWorstValue;
    ar >> dst.nWorstObjID;
    ar >> dst.fAvrValue;
    ar >> dst.fSTDEV;

    ar >> nDataNum;

    dst.SetObjSize(nDataNum);
    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
    {
        ar >> dst.vecstrObjID[nIdx] >> dst.vecfObjEachValue[nIdx] >> dst.vecbObjEachResult[nIdx]
            >> dst.vecbObjIsInvalid[nIdx];
    }

    return ar;
}

CString VisionInspectionStatisticsValue::GetObjEachObjID(long i_nIdx)
{
    CString strObjID;
    strObjID.Empty();

    if (i_nIdx >= 0 && i_nIdx < vecstrObjID.size())
        strObjID = vecstrObjID[i_nIdx];

    return strObjID;
}
