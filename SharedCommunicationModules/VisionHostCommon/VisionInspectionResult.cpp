//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionResult.h"

//CPP_2_________________________________ This project's headers
#include "DBObject.h"
#include "VisionHostBaseDef.h"
#include "VisionInspectionSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedComponent/Serialization/SerializeVector.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ArchiveAllType& operator>>(ArchiveAllType& ar, VisionInspectionResult& dst)
{
    ar >> dst.m_moduleGuid;
    ar >> dst.m_specGuid;
    ar >> dst.m_inspName;
    ar >> dst.m_resultName;
    ar >> dst.m_fullName;
    ar >> dst.m_sortName; // 영훈 20131128 : sortname을 정의한다.
    ar >> dst.m_itemUnit;
    ar >> dst.m_reviewImageFileName;
    ar >> *(long*)&dst.m_resultType;
    ar >> *(long*)&dst.m_hostReportCategory;

    ar >> dst.m_targetName;
    ar >> dst.m_totalResult;
    ar >> dst.m_minObjectIndex;
    ar >> dst.m_maxObjectIndex;
    ar >> dst.m_worstObjectIndex;

    ar >> dst.m_objectResults;
    ar >> dst.m_objectRects;
    ar >> dst.m_objectErrorValues;
    ar >> dst.m_objectNames;
    ar >> dst.m_objectGroupID; //kircheis_MED2.5
    ar >> dst.m_objectValues;
    ar >> dst.m_nominalValues;
    ar >> dst.vecrtRejectROI;
    ar >> dst.vecrtMarginalROI;

    ar >> dst.nDefectAreaSize; // 영훈 20150908 : Surface에서 불량이 위치한 영역의 ID를 받는다.
    ar >> dst.vecnDefectAreaID;

    ar >> dst.m_objectPositionX;
    ar >> dst.m_objectPositionY;

    dst.m_vecSurfaceCriteriaResult.clear();
    long nSize = 0;
    ar >> nSize;
    dst.m_vecSurfaceCriteriaResult.resize(nSize); //kircheis_Criteria
    for (long idx = 0; idx < nSize; idx++)
        ar >> dst.m_vecSurfaceCriteriaResult[idx];

    ar >> dst.m_objectCompTypes; //mc_CompType(DMS Name)

    //{{ //kircheis_VSV
    dst.m_vecInspStatisticsValue.clear();
    nSize = 0;
    ar >> nSize;
    dst.m_vecInspStatisticsValue.resize(nSize);
    for (long idx = 0; idx < nSize; idx++)
        ar >> dst.m_vecInspStatisticsValue[idx];
    //}}

    return ar;
}

ArchiveAllType& operator<<(ArchiveAllType& ar, VisionInspectionResult& src)
{
    ar << src.m_moduleGuid;
    ar << src.m_specGuid;
    ar << src.m_inspName;
    ar << src.m_resultName;
    ar << _T("[") + src.m_inspName + _T("]") + src.m_resultName;
    ar << src.m_sortName; // 영훈 20131128 : sortname을 정의한다.
    ar << src.m_itemUnit;
    ar << src.m_reviewImageFileName;
    ar << *(long*)&src.m_resultType;
    ar << *(long*)&src.m_hostReportCategory;

    ar << src.m_targetName;
    ar << src.m_totalResult;
    ar << src.m_minObjectIndex;
    ar << src.m_maxObjectIndex;
    ar << src.m_worstObjectIndex;

    ar << src.m_objectResults;
    ar << src.m_objectRects;
    ar << src.m_objectErrorValues;
    ar << src.m_objectNames;
    ar << src.m_objectGroupID; //kircheis_MED2.5
    ar << src.m_objectValues;
    ar << src.m_nominalValues;
    ar << src.vecrtRejectROI;
    ar << src.vecrtMarginalROI;

    ar << src.nDefectAreaSize; // 영훈 20150908 : Surface에서 불량이 위치한 영역의 ID를 받는다.
    ar << src.vecnDefectAreaID;

    ar << src.m_objectPositionX;
    ar << src.m_objectPositionY;

    long nSize = (long)src.m_vecSurfaceCriteriaResult.size(); //kircheis_Criteria
    ar << nSize;
    for (long idx = 0; idx < nSize; idx++)
        ar << src.m_vecSurfaceCriteriaResult[idx]; //kircheis_Criteria

    ar << src.m_objectCompTypes; //mc_CompType(DMS Name)

    //{{ //kircheis_VSV
    nSize = (long)src.m_vecInspStatisticsValue.size();
    ar << nSize;
    for (long idx = 0; idx < nSize; idx++)
        ar << src.m_vecInspStatisticsValue[idx];
    //}}

    return ar;
}

#if defined(USE_BY_HANDLER)
VisionInspectionResult::VisionInspectionResult()
{
    m_inspName = _T("Unknown Insp");
    m_resultName = _T("Unknown Result");
    m_targetName = _T("Unknown Target");
    m_totalResult = NOT_MEASURED;
    m_minObjectIndex = 0;
    m_maxObjectIndex = 0;
    m_worstObjectIndex = 0;
    m_itemUnit = _T("");
    m_resultType = II_RESULT_TYPE::MEASURED;
    m_hostReportCategory = HostReportCategory::MEASURE;
    m_sortName = _T(""); // 영훈20131128 : sortname을정의한다. // 공백 혹은 1글짜로
    m_seleteProbeResult = -1;
    m_LossCode.Empty();
}
#endif

VisionInspectionResult::VisionInspectionResult(LPCTSTR moduleGuid, LPCTSTR specGuid, LPCTSTR inspName,
    LPCTSTR resultName, LPCTSTR targetName, LPCTSTR sortName, LPCTSTR unit, II_RESULT_TYPE resultType,
    HostReportCategory hostReportCategory)
    : m_moduleGuid(moduleGuid)
    , m_specGuid(specGuid)
    , m_inspName(inspName)
    , m_resultName(resultName)
    , m_targetName(targetName)
    , m_sortName(sortName)
    , m_itemUnit(unit)
    , m_resultType(resultType)
    , m_hostReportCategory(hostReportCategory)
{
    Clear();
}

void VisionInspectionResult::Clear()
{
    m_totalResult = NOT_MEASURED; ///< 각 항목 Result
    m_reviewImageFileName = _T("");

    m_minObjectIndex = 0;
    m_maxObjectIndex = 0;
    m_worstObjectIndex = 0;

    m_objectResults.clear();
    m_objectRects.clear();
    m_objectErrorValues.clear();
    m_objectNames.clear();
    m_objectGroupID.clear(); //kircheis_MED2.5
    vecbWriteResult.clear();
    m_objectValues.clear();
    m_nominalValues.clear();
    vecrtRejectROI.clear();
    vecrtMarginalROI.clear();
    nDefectAreaSize = 0;
    vecnDefectAreaID.clear();

    m_objectPositionX.clear();
    m_objectPositionY.clear();

    m_vecSurfaceCriteriaResult.clear(); //kircheis_Criteria
    m_objectCompTypes.clear(); //mc_CompType(DMS Name)
    m_LossCode.Empty();

    m_vecInspStatisticsValue.clear(); //kircheis_VSV
}

void VisionInspectionResult::Resize(long nNum)
{
    m_objectResults.resize(nNum, NOT_MEASURED);
    m_objectRects.resize(nNum, Ipvm::Rect32s(0, 0, 0, 0));
    m_objectErrorValues.resize(nNum, 0.f);
    m_objectNames.resize(nNum);
    m_objectGroupID.resize(nNum); //kircheis_MED2.5
    m_objectCompTypes.resize(nNum); //kircheis_MED2.5
    m_objectValues.resize(nNum, 0.f);
    m_nominalValues.resize(nNum, 0.f);
    vecbWriteResult.resize(nNum, FALSE);

    m_objectPositionX.resize(nNum, 0.f);
    m_objectPositionY.resize(nNum, 0.f);

    m_vecSurfaceCriteriaResult.clear(); //kircheis_Criteria

    m_vecInspStatisticsValue.clear(); //kircheis_VSV
}

long VisionInspectionResult::IsValueCheck(VisionInspectionSpec* pSpec, float fValue)
{
    if (!pSpec->m_use)
        return NOT_MEASURED;

    if (pSpec->m_useMin && !pSpec->m_useMax)
    {
        if (fValue >= pSpec->m_passMin)
            return PASS;

        else if (fValue >= pSpec->m_marginalMin && pSpec->m_useMarginal)
            return MARGINAL;

        return REJECT;
    }
    else if (!pSpec->m_useMin && pSpec->m_useMax)
    {
        if (fValue <= pSpec->m_passMax)
            return PASS;

        else if (fValue <= pSpec->m_marginalMax && pSpec->m_useMarginal)
            return MARGINAL;

        return REJECT;
    }
    else if (pSpec->m_useMin && pSpec->m_useMax)
    {
        if (fValue >= pSpec->m_passMin && fValue <= pSpec->m_passMax)
            return PASS;

        else if (fValue >= pSpec->m_marginalMin && fValue <= pSpec->m_marginalMax && pSpec->m_useMarginal)
            return MARGINAL;

        return REJECT;
    }

    return NOT_MEASURED;
}

BOOL VisionInspectionResult::SetValue_EachResult_AndTypicalValueBySingleSpec(const long objectIndex, LPCTSTR objectName,
    float errorValue, const VisionInspectionSpec& spec, const float objectPosX, const float objectPosY,
    float nominalValue, long specifiedResult /*bool invalidCheck*/) //kircheis_VSV
{
    ASSERT(objectIndex >= 0);
    ASSERT(objectIndex < m_objectErrorValues.size());
    ASSERT(objectIndex < m_objectNames.size());
    ASSERT(objectIndex < m_objectResults.size());

    if (objectIndex < 0)
        return FALSE;

    if (objectIndex >= (long)m_objectErrorValues.size())
        return FALSE;

    if (objectIndex >= (long)m_objectNames.size())
        return FALSE;

    if (objectIndex >= (long)m_objectResults.size())
        return FALSE;

    m_objectErrorValues[objectIndex] = errorValue;
    m_objectNames[objectIndex] = objectName;

    BOOL bIsValidData = TRUE; //kircheis_VSV

    if (errorValue == Ipvm::k_noiseValue32r)
    {
        // Noise 값은 Nomial Value을 더하지 말고 유지하자
        m_objectValues[objectIndex] = m_objectErrorValues[objectIndex];
        bIsValidData = FALSE; //kircheis_VSV
    }
    else if (spec.m_resultType == II_RESULT_TYPE::PERCENT || spec.m_resultType == II_RESULT_TYPE::DEGREE)
    {
        m_objectValues[objectIndex] = m_objectErrorValues[objectIndex];
    }
    else
    {
        m_objectValues[objectIndex] = m_objectErrorValues[objectIndex] + nominalValue;
    }

    m_nominalValues[objectIndex] = nominalValue;

    m_objectPositionX[objectIndex] = objectPosX;
    m_objectPositionY[objectIndex] = objectPosY;

    if (objectIndex == 0)
    {
        m_worstObjectIndex = objectIndex;
    }

    if (!spec.m_use)
    {
        m_objectResults[objectIndex] = NOT_MEASURED;

        return TRUE;
    }

    if (spec.m_useMin && !spec.m_useMax)
    {
        if (m_objectErrorValues[objectIndex] >= spec.m_passMin)
        {
            m_objectResults[objectIndex] = PASS;
        }
        else if (m_objectErrorValues[objectIndex] >= spec.m_marginalMin && spec.m_useMarginal == TRUE)
        {
            m_objectResults[objectIndex] = MARGINAL;
        }
        else
        {
            m_objectResults[objectIndex] = REJECT;
        }

        if (objectIndex != 0
            && m_objectErrorValues[objectIndex] - spec.m_passMin
                < m_objectErrorValues[m_worstObjectIndex] - spec.m_passMin)
        {
            m_worstObjectIndex = objectIndex;
        }

        if (specifiedResult > NOT_MEASURED) //kircheis_VSV
        {
            m_objectResults[objectIndex] = specifiedResult;
            RegistStatisticsValue(nominalValue, objectName, m_objectValues[objectIndex], specifiedResult, 1);
        }
        else if (bIsValidData) //kircheis_VSV
            RegistStatisticsValue(
                nominalValue, objectName, m_objectValues[objectIndex], m_objectResults[objectIndex], 0);

        return TRUE;
    }
    else if (!spec.m_useMin && spec.m_useMax)
    {
        if (m_objectErrorValues[objectIndex] <= spec.m_passMax)
        {
            m_objectResults[objectIndex] = PASS;
        }
        else if (m_objectErrorValues[objectIndex] <= spec.m_marginalMax && spec.m_useMarginal == TRUE)
        {
            m_objectResults[objectIndex] = MARGINAL;
        }
        else
        {
            m_objectResults[objectIndex] = REJECT;
        }

        if (objectIndex != 0
            && m_objectErrorValues[objectIndex] - spec.m_passMax
                > m_objectErrorValues[m_worstObjectIndex] - spec.m_passMax)
        {
            m_worstObjectIndex = objectIndex;
        }

        if (specifiedResult > NOT_MEASURED) //kircheis_VSV
        {
            m_objectResults[objectIndex] = specifiedResult;
            RegistStatisticsValue(nominalValue, objectName, m_objectValues[objectIndex], specifiedResult, 1);
        }
        else if (bIsValidData) //kircheis_VSV
            RegistStatisticsValue(
                nominalValue, objectName, m_objectValues[objectIndex], m_objectResults[objectIndex], 0);

        return TRUE;
    }
    else if (spec.m_useMin && spec.m_useMax)
    {
        float fSpecCenter = (spec.m_passMin + spec.m_passMax) * 0.5f;
        float fBadMax = 0.f;
        float fBadCur;

        if (m_objectErrorValues[objectIndex] == -1000000)
        {
            m_objectResults[objectIndex] = PASS;
        }

        if (m_objectErrorValues[objectIndex] >= spec.m_passMin && m_objectErrorValues[objectIndex] <= spec.m_passMax)
        {
            m_objectResults[objectIndex] = PASS;
        }
        else if (m_objectErrorValues[objectIndex] >= spec.m_marginalMin
            && m_objectErrorValues[objectIndex] <= spec.m_marginalMax && spec.m_useMarginal == TRUE)
        {
            m_objectResults[objectIndex] = MARGINAL;
        }
        else
        {
            m_objectResults[objectIndex] = REJECT;
        }

        fBadCur = abs(m_objectErrorValues[objectIndex] - fSpecCenter);
        fBadMax = abs(m_objectErrorValues[m_worstObjectIndex] - fSpecCenter);

        if (objectIndex != 0 && fBadCur > fBadMax)
        {
            m_worstObjectIndex = objectIndex;
        }

        if (specifiedResult > NOT_MEASURED) //kircheis_VSV
        {
            m_objectResults[objectIndex] = specifiedResult;
            RegistStatisticsValue(nominalValue, objectName, m_objectValues[objectIndex], specifiedResult, 1);
        }
        else if (bIsValidData) //kircheis_VSV
            RegistStatisticsValue(
                nominalValue, objectName, m_objectValues[objectIndex], m_objectResults[objectIndex], 0);

        return TRUE;
    }

    // SDY_IntensityChecker2D Spec에서 Use만 체크되어 있을 때에도 기존에 설정한 대로 나가도록 하기 위해 기능 추가
    else if (specifiedResult > NOT_MEASURED)
    {
        m_objectResults[objectIndex] = specifiedResult;
        RegistStatisticsValue(nominalValue, objectName, m_objectValues[objectIndex], specifiedResult, 1);

        return TRUE;
    }

    return TRUE;
}

void VisionInspectionResult::ResizeForComponent(long nNum)
{
    Resize(nNum);
    m_objectCompTypes.resize(nNum); //mc_CompType(DMS Name)
}

BOOL VisionInspectionResult::SetValue_EachResult_AndTypicalValueBySingleSpec(const long objectIndex, LPCTSTR objectName,
    CString objectGroupID, CString objectCompType, float errorValue, const VisionInspectionSpec& spec,
    const float objectPosX, const float objectPosY, float nominalValue, long specifiedResult) //kircheis_MED2.5
{
    ASSERT(objectIndex >= 0);
    ASSERT(objectIndex < m_objectErrorValues.size());
    ASSERT(objectIndex < m_objectNames.size());
    ASSERT(objectIndex < m_objectResults.size());

    if (objectIndex < 0)
        return FALSE;

    if (objectIndex >= (long)m_objectErrorValues.size())
        return FALSE;

    if (objectIndex >= (long)m_objectNames.size())
        return FALSE;

    if (objectIndex >= (long)m_objectGroupID.size())
        return FALSE;

    if (objectIndex >= (long)m_objectCompTypes.size())
        return FALSE;

    if (objectIndex >= (long)m_objectResults.size())
        return FALSE;

    m_objectErrorValues[objectIndex] = errorValue;
    m_objectNames[objectIndex] = objectName;
    m_objectGroupID[objectIndex] = objectGroupID;
    m_objectPositionX[objectIndex] = objectPosX;
    m_objectPositionY[objectIndex] = objectPosY;
    if (objectIndex < (long)m_objectCompTypes.size()) //ResizeForComponent를 꼭 먼저 호출되어야 한다
        m_objectCompTypes[objectIndex] = objectCompType;

    BOOL bIsValidData = TRUE; //kircheis_VSV
    if (errorValue == Ipvm::k_noiseValue32r)
    {
        // Noise 값은 Nomial Value을 더하지 말고 유지하자
        m_objectValues[objectIndex] = m_objectErrorValues[objectIndex];
        bIsValidData = FALSE; //kircheis_VSV
    }
    else if (spec.m_resultType == II_RESULT_TYPE::PERCENT || spec.m_resultType == II_RESULT_TYPE::DEGREE)
    {
        m_objectValues[objectIndex] = m_objectErrorValues[objectIndex];
    }
    else
    {
        m_objectValues[objectIndex] = m_objectErrorValues[objectIndex] + nominalValue;
    }

    m_nominalValues[objectIndex] = nominalValue;

    if (objectIndex == 0)
    {
        m_worstObjectIndex = objectIndex;
    }

    if (!spec.m_use)
    {
        m_objectResults[objectIndex] = NOT_MEASURED;

        return TRUE;
    }

    if (spec.m_useMin && !spec.m_useMax)
    {
        if (m_objectErrorValues[objectIndex] >= spec.m_passMin)
        {
            m_objectResults[objectIndex] = PASS;
        }
        else if (m_objectErrorValues[objectIndex] >= spec.m_marginalMin && spec.m_useMarginal == TRUE)
        {
            m_objectResults[objectIndex] = MARGINAL;
        }
        else
        {
            m_objectResults[objectIndex] = REJECT;
        }

        if (objectIndex != 0
            && m_objectErrorValues[objectIndex] - spec.m_passMin
                < m_objectErrorValues[m_worstObjectIndex] - spec.m_passMin)
        {
            m_worstObjectIndex = objectIndex;
        }

        if (specifiedResult > NOT_MEASURED) //kircheis_VSV
        {
            m_objectResults[objectIndex] = specifiedResult;
            RegistStatisticsValue(
                objectCompType, nominalValue, objectName, m_objectValues[objectIndex], specifiedResult, 1);
        }
        else if (bIsValidData) //kircheis_VSV
            RegistStatisticsValue(
                objectCompType, nominalValue, objectName, m_objectValues[objectIndex], m_objectResults[objectIndex], 0);

        return TRUE;
    }
    else if (!spec.m_useMin && spec.m_useMax)
    {
        if (m_objectErrorValues[objectIndex] <= spec.m_passMax)
        {
            m_objectResults[objectIndex] = PASS;
        }
        else if (m_objectErrorValues[objectIndex] <= spec.m_marginalMax && spec.m_useMarginal == TRUE)
        {
            m_objectResults[objectIndex] = MARGINAL;
        }
        else
        {
            m_objectResults[objectIndex] = REJECT;
        }

        if (objectIndex != 0
            && m_objectErrorValues[objectIndex] - spec.m_passMax
                > m_objectErrorValues[m_worstObjectIndex] - spec.m_passMax)
        {
            m_worstObjectIndex = objectIndex;
        }

        if (specifiedResult > NOT_MEASURED) //kircheis_VSV
        {
            m_objectResults[objectIndex] = specifiedResult;
            RegistStatisticsValue(
                objectCompType, nominalValue, objectName, m_objectValues[objectIndex], specifiedResult, 1);
        }
        else if (bIsValidData) //kircheis_VSV
            RegistStatisticsValue(
                objectCompType, nominalValue, objectName, m_objectValues[objectIndex], m_objectResults[objectIndex], 0);

        return TRUE;
    }
    else if (spec.m_useMin && spec.m_useMax)
    {
        float fSpecCenter = (spec.m_passMin + spec.m_passMax) * 0.5f;
        float fBadMax = 0.f;
        float fBadCur;

        if (m_objectErrorValues[objectIndex] == -1000000)
        {
            m_objectResults[objectIndex] = PASS;
        }

        if (m_objectErrorValues[objectIndex] >= spec.m_passMin && m_objectErrorValues[objectIndex] <= spec.m_passMax)
        {
            m_objectResults[objectIndex] = PASS;
        }
        else if (m_objectErrorValues[objectIndex] >= spec.m_marginalMin
            && m_objectErrorValues[objectIndex] <= spec.m_marginalMax && spec.m_useMarginal == TRUE)
        {
            m_objectResults[objectIndex] = MARGINAL;
        }
        else
        {
            m_objectResults[objectIndex] = REJECT;
        }

        fBadCur = abs(m_objectErrorValues[objectIndex] - fSpecCenter);
        fBadMax = abs(m_objectErrorValues[m_worstObjectIndex] - fSpecCenter);

        if (objectIndex != 0 && fBadCur > fBadMax)
        {
            m_worstObjectIndex = objectIndex;
        }

        if (specifiedResult > NOT_MEASURED) //kircheis_VSV
        {
            m_objectResults[objectIndex] = specifiedResult;
            RegistStatisticsValue(
                objectCompType, nominalValue, objectName, m_objectValues[objectIndex], specifiedResult, 1);
        }
        else if (bIsValidData) //kircheis_VSV
            RegistStatisticsValue(
                objectCompType, nominalValue, objectName, m_objectValues[objectIndex], m_objectResults[objectIndex], 0);

        return TRUE;
    }

    return TRUE;
}

BOOL VisionInspectionResult::SetInvalid_EachResult(const long objectIndex, LPCTSTR objectName, LPCTSTR objectGroupID,
    const float objectPosX, const float objectPosY, float nominalValue) //kircheis_MED2.5
{
    ASSERT(objectIndex >= 0);
    ASSERT(objectIndex < m_objectErrorValues.size());
    ASSERT(objectIndex < m_objectNames.size());
    ASSERT(objectIndex < m_objectResults.size());

    if (objectIndex < 0)
        return FALSE;

    if (objectIndex >= (long)m_objectErrorValues.size())
        return FALSE;

    if (objectIndex >= (long)m_objectNames.size())
        return FALSE;

    if (objectIndex >= (long)m_objectGroupID.size())
        return FALSE;

    if (objectIndex >= (long)m_objectResults.size())
        return FALSE;

    m_objectErrorValues[objectIndex] = 0.f;
    m_objectNames[objectIndex] = objectName;
    m_objectGroupID[objectIndex] = objectGroupID;
    m_objectValues[objectIndex] = nominalValue;
    m_nominalValues[objectIndex] = nominalValue;
    m_objectResults[objectIndex] = INVALID;

    m_objectPositionX[objectIndex] = objectPosX;
    m_objectPositionY[objectIndex] = objectPosY;

    return TRUE;
}

void VisionInspectionResult::UpdateTypicalValue(VisionInspectionSpec* pSpec)
{
    long nValueSize = (long)m_objectErrorValues.size();

    // 영훈 20150916 : 개수가 1개까지는 의미가 없다.
    if (nValueSize <= 0)
        return;

    m_worstObjectIndex = m_minObjectIndex = m_maxObjectIndex = 0;

    if (nValueSize <= 1)
    {
        for (auto& statisticsVlue : m_vecInspStatisticsValue) //kircheis_VSV
        {
            if (statisticsVlue.bCalcEnd)
                continue;
            statisticsVlue.CalcStatisticsValue(pSpec->m_useMin, pSpec->m_useMax);
        }
        return;
    }

    if (pSpec->m_useMin && !pSpec->m_useMax)
    {
        for (long nIndex = 1; nIndex < nValueSize; nIndex++)
        {
            if (m_objectErrorValues[m_worstObjectIndex] > m_objectErrorValues[nIndex])
            {
                m_minObjectIndex = nIndex;
                m_worstObjectIndex = nIndex;
            }
        }
    }

    if (!pSpec->m_useMin && pSpec->m_useMax)
    {
        for (long nIndex = 1; nIndex < nValueSize; nIndex++)
        {
            if (m_objectErrorValues[m_worstObjectIndex] < m_objectErrorValues[nIndex])
            {
                m_maxObjectIndex = nIndex;
                m_worstObjectIndex = nIndex;
            }
        }
    }

    if (pSpec->m_useMin && pSpec->m_useMax)
    {
        float fSpecCenter = (pSpec->m_passMin + pSpec->m_passMax) * 0.5f;
        float fBadMax = -999999.999999f; // pSpec->m_passMax;
        float fBadMin = 999999.999999f; //pSpec->m_passMin;
        float fPassMax = 0.f, fCurrent;
        long passMaxIndex = 0;

        for (long nIndex = 0; nIndex < nValueSize; nIndex++)
        {
            /*fBadCur = abs(m_objectErrorValues[nIndex] - fSpecCenter);
			fBadMax = abs(m_worstErrorValue - fSpecCenter);*/

            fCurrent = m_objectErrorValues[nIndex];

            /*if (fBadCur > fBadMax)
			{
			m_worstErrorValue = m_objectErrorValues[nIndex];
			}*/

            if (fCurrent == fBadMax || fCurrent == fBadMin)
                continue;

            if (fPassMax < fCurrent)
            {
                fPassMax = fCurrent; // Pass
                passMaxIndex = nIndex;
            }

            if (fCurrent > fBadMax)
            {
                fBadMax = m_objectErrorValues[nIndex];
                m_maxObjectIndex = nIndex;
            }
            if (fCurrent < fBadMin)
            {
                fBadMin = m_objectErrorValues[nIndex];
                m_minObjectIndex = nIndex;
            }
        }

        if (fBadMax != pSpec->m_passMax && fBadMin != pSpec->m_passMin)
        {
            if (abs(fBadMax - fSpecCenter) >= abs(fBadMin - fSpecCenter))
            {
                m_worstObjectIndex = m_maxObjectIndex;
            }
            else if (abs(fBadMax - fSpecCenter) < abs(fBadMin - fSpecCenter))
            {
                m_worstObjectIndex = m_minObjectIndex;
            }
        }
        else if (fBadMax != pSpec->m_passMax)
            m_worstObjectIndex = m_maxObjectIndex;
        else if (fBadMin != pSpec->m_passMin)
            m_worstObjectIndex = m_minObjectIndex;
        else
            m_worstObjectIndex = passMaxIndex;
    }

    for (auto& statisticsVlue : m_vecInspStatisticsValue) //kircheis_VSV
    {
        if (statisticsVlue.bCalcEnd)
            continue;

        statisticsVlue.CalcStatisticsValue(pSpec->m_useMin, pSpec->m_useMax);
    }
}

void VisionInspectionResult::SetTotalResult()
{
    BOOL bNotMeasured = FALSE;
    BOOL bPass = FALSE;
    BOOL bMarginal = FALSE;
    BOOL bReject = FALSE;
    BOOL bInvalid = FALSE;
    BOOL bEmpty = FALSE;
    BOOL bCoupon = FALSE;
    BOOL bDoubleDeviece = FALSE;

    vecnDefectAreaID.clear();
    vecnDefectAreaID.reserve((long)m_objectResults.size());

    vecrtRejectROI.clear();
    vecrtRejectROI.reserve((long)m_objectResults.size());

    vecrtMarginalROI.clear();
    vecrtMarginalROI.reserve((long)m_objectResults.size());

    //k 좌표찍기
    for (long i = 0; i < (long)(m_objectResults.size()); i++)
    {
        if (m_objectResults[i] != PASS)
            vecnDefectAreaID.push_back(i);

        if (m_objectResults[i] == NOT_MEASURED)
            bNotMeasured = TRUE;
        if (m_objectResults[i] == PASS)
            bPass = TRUE;
        if (m_objectResults[i] == MARGINAL)
            bMarginal = TRUE;
        if (m_objectResults[i] == REJECT)
            bReject = TRUE;
        if (m_objectResults[i] == INVALID)
            bInvalid = TRUE;
        if (m_objectResults[i] == EMPTY)
            bEmpty = TRUE;
        if (m_objectResults[i] == COUPON)
            bCoupon = TRUE;
        if (m_objectResults[i] == DOUBLEDEVICE)
            bDoubleDeviece = TRUE;

        if (m_objectResults[i] == MARGINAL)
        {
            if (m_objectRects[i] != Ipvm::Rect32s(0, 0, 0, 0))
                vecrtMarginalROI.push_back(m_objectRects[i]);
        }
        else if (m_objectResults[i] != PASS)
        {
            // 영훈 [ Reject ROI ] 20130809 : 솔직히 ROI Data가 없는건 넣을 필요 없잖아.. 괜히 공간만 차지한다.
            if (m_objectRects[i] != Ipvm::Rect32s(0, 0, 0, 0))
                vecrtRejectROI.push_back(m_objectRects[i]);
        }
    }

    // 영훈 [ Reject ROI ] 20130809 : 혹시 이미 저장된게 있을 지 몰라 한번 더 지워주도록 한다.
    for (long n = 0; n < (long)(vecrtRejectROI.size()); n++)
    {
        if (vecrtRejectROI[n] == Ipvm::Rect32s(0, 0, 0, 0))
        {
            vecrtRejectROI.erase(vecrtRejectROI.begin() + n);
            n--;
        }
    }
    if (bNotMeasured)
    {
        m_totalResult = NOT_MEASURED;
    }
    else if (bEmpty)
    {
        m_totalResult = EMPTY;
    }
    else if (bDoubleDeviece)
    {
        m_totalResult = DOUBLEDEVICE;
    }
    else if (bInvalid)
    {
        m_totalResult = INVALID;
    }
    else if (bReject)
    {
        m_totalResult = REJECT;
    }
    else if (bMarginal)
    {
        m_totalResult = MARGINAL;
    }
    else if (bPass)
    {
        m_totalResult = PASS;
    }
    else if (bCoupon)
    {
        m_totalResult = COUPON;
    }
}

BOOL VisionInspectionResult::SetResult(long nIndex, long nResult)
{
    if (nIndex < 0)
        return FALSE;
    if (nIndex >= (long)(m_objectResults.size()))
        return FALSE;
    m_objectResults[nIndex] = nResult;

    return TRUE;
}

// 동적으로 결과 추가용.
BOOL VisionInspectionResult::AddResult(long nResult, float fValue, CString sObjectID, CString sObjectGroupID,
    const Ipvm::Rect32s& rtRect) //kircheis_MED2.5
{
    m_objectResults.push_back(nResult);
    m_objectErrorValues.push_back(fValue);
    m_objectValues.push_back(fValue);
    m_nominalValues.push_back(0.f);
    m_objectNames.push_back(sObjectID);
    m_objectGroupID.push_back(sObjectGroupID);
    m_objectRects.push_back(rtRect);

    return TRUE;
}

BOOL VisionInspectionResult::AddSpecSize(float fSpecSize) // 임시 함수 스펙 크기
{
    vecnDefectAreaID.push_back((long)fSpecSize);

    return TRUE;
}

void VisionInspectionResult::SetInvalid()
{
    Clear();

    m_totalResult = INVALID;
}

BOOL VisionInspectionResult::SetValue(long nIndex, float fValue, float fSpecValue)
{
    if (nIndex < 0)
        return FALSE;
    if (nIndex >= (long)(m_objectErrorValues.size()))
        return FALSE;

    m_objectErrorValues[nIndex] = fValue;
    m_objectValues[nIndex] = fSpecValue + fValue;
    m_nominalValues[nIndex] = fSpecValue;

    return TRUE;
}

BOOL VisionInspectionResult::SetRect(long nIndex, const Ipvm::Rect32s& rtRect)
{
    if (nIndex < 0)
        return FALSE;
    if (nIndex >= (long)(m_objectResults.size()))
        return FALSE;
    m_objectRects[nIndex] = rtRect;

    return TRUE;
}

BOOL VisionInspectionResult::SetRejectROI(const Ipvm::Rect32s& rtRect)
{
    vecrtRejectROI.push_back(rtRect);

    return TRUE;
}

void VisionInspectionResult::AddSurfaceCriteriaResult(const VisionInspectionSurfaceResult& src) //kircheis_Criteria
{
    m_vecSurfaceCriteriaResult.push_back(src);
}

void VisionInspectionResult::RegistStatisticsValue(
    float fNominalSpec, CString strObjID, float fEachValue, long nEachResult, BYTE bObjIsInvalid) //kircheis_VSV
{
    long nGroupID = -1;
    if (m_vecInspStatisticsValue.size() == 0)
    {
        m_vecInspStatisticsValue.resize(1);
        nGroupID = 0;
        m_vecInspStatisticsValue[nGroupID].fSpecRefValue = fNominalSpec;
        m_vecInspStatisticsValue[nGroupID].strGroupID.Format(_T("Nominal:%.3f"), fNominalSpec);
    }
    //else //kircheis_VSV_Option//Nominal Spec 별로 Group을 분할 하는 Code
    //{
    //	long nGroupNum = (long)m_vecInspStatisticsValue.size();
    //	for (long nIdx = 0; nIdx < nGroupNum; nIdx++)
    //	{
    //		if (m_vecInspStatisticsValue[nIdx].fSpecRefValue == fNominalSpec)
    //		{
    //			nGroupID = nIdx;
    //			break;
    //		}
    //	}
    //	if (nGroupID == -1)
    //	{
    //		VisionInspectionStatisticsValue visionInspStatisticsValue;
    //		visionInspStatisticsValue.strGroupID.Format(_T("Nominal:%.3f"), fNominalSpec);
    //		visionInspStatisticsValue.fSpecRefValue = fNominalSpec;
    //		nGroupID = nGroupNum;
    //		m_vecInspStatisticsValue.push_back(visionInspStatisticsValue);
    //	}
    //}
    nGroupID = 0; //kircheis_VSV_Option

    m_vecInspStatisticsValue[nGroupID].SetEachValue(strObjID, fEachValue, (BYTE)nEachResult, bObjIsInvalid);
}

void VisionInspectionResult::RegistStatisticsValue(CString strGroupID, float fNominalSpec, CString strObjID,
    float fEachValue, long nEachResult, BYTE bObjIsInvalid) //kircheis_VSV
{
    CString strCurCompType = strGroupID;
    CString strCompType;
    BOOL bFineUnderBar = FALSE;
    char ch = (char)strCurCompType.GetAt(0);
    if ((ch == 'R' || ch == 'r' || ch == 'C' || ch == 'c') && strCurCompType.Find(_T("_")) >= 0)
    {
        long nLength = strCurCompType.GetLength();
        strCompType.Empty();
        long nIdx = 0;
        for (nIdx = 0; nIdx < nLength; nIdx++)
        {
            ch = (char)strCurCompType.GetAt(nIdx);
            if (ch == '_')
            {
                if (bFineUnderBar)
                    break;
                bFineUnderBar = TRUE;
            }
        }
        if (nIdx < nLength)
            strCompType = strCurCompType.Left(nIdx);
        else
            strCompType = strGroupID;
    }
    else
        strCompType = strGroupID;

    long nCompIdx = -1;
    if (m_vecInspStatisticsValue.size() == 0)
    {
        nCompIdx = 0;
        m_vecInspStatisticsValue.resize(1);
        m_vecInspStatisticsValue[nCompIdx].strGroupID = strCompType;
        m_vecInspStatisticsValue[nCompIdx].fSpecRefValue = fNominalSpec;
    }
    else
    {
        long nGroupNum = (long)m_vecInspStatisticsValue.size();
        for (long nIdx = 0; nIdx < nGroupNum; nIdx++)
        {
            if (m_vecInspStatisticsValue[nIdx].strGroupID
                == strCompType) // && m_vecInspStatisticsValue[nIdx].fSpecRefValue == fNominalSpec)
            {
                nCompIdx = nIdx;
                break;
            }
        }
        if (nCompIdx == -1)
        {
            VisionInspectionStatisticsValue visionInspStatisticsValue;
            visionInspStatisticsValue.strGroupID = strCompType;
            visionInspStatisticsValue.fSpecRefValue = fNominalSpec;
            nCompIdx = nGroupNum;
            m_vecInspStatisticsValue.push_back(visionInspStatisticsValue);
        }
    }

    m_vecInspStatisticsValue[nCompIdx].SetEachValue(strObjID, fEachValue, (BYTE)nEachResult, bObjIsInvalid);
}

float VisionInspectionResult::getObjectMinValue() const
{
    if (m_minObjectIndex < 0 || m_minObjectIndex >= long(m_objectValues.size()))
    {
        return Ipvm::k_noiseValue32r;
    }

    return m_objectValues[m_minObjectIndex];
}

float VisionInspectionResult::getObjectMinErrorValue() const
{
    if (m_minObjectIndex < 0 || m_minObjectIndex >= long(m_objectErrorValues.size()))
    {
        return Ipvm::k_noiseValue32r;
    }

    return m_objectErrorValues[m_minObjectIndex];
}

float VisionInspectionResult::getObjectMaxValue() const
{
    if (m_maxObjectIndex < 0 || m_maxObjectIndex >= long(m_objectValues.size()))
    {
        return Ipvm::k_noiseValue32r;
    }

    return m_objectValues[m_maxObjectIndex];
}

float VisionInspectionResult::getObjectMaxErrorValue() const
{
    if (m_maxObjectIndex < 0 || m_maxObjectIndex >= long(m_objectErrorValues.size()))
    {
        return Ipvm::k_noiseValue32r;
    }

    return m_objectErrorValues[m_maxObjectIndex];
}

float VisionInspectionResult::getObjectWorstValue() const
{
    if (m_worstObjectIndex < 0 || m_worstObjectIndex >= long(m_objectValues.size()))
    {
        return Ipvm::k_noiseValue32r;
    }

    return m_objectValues[m_worstObjectIndex];
}

float VisionInspectionResult::getObjectWorstErrorValue() const
{
    if (m_worstObjectIndex < 0 || m_worstObjectIndex >= long(m_objectErrorValues.size()))
    {
        return Ipvm::k_noiseValue32r;
    }

    return m_objectErrorValues[m_worstObjectIndex];
}

CString VisionInspectionResult::getObjectWorstID() const
{
    if (m_worstObjectIndex < 0 || m_worstObjectIndex >= long(m_objectValues.size()))
    {
        return _T("");
    }

    CString id;
    id = m_objectNames[m_worstObjectIndex];

    return id;
}

CString VisionInspectionResult::getObjectID(long ObjID) const
{
    if (ObjID < 0 || ObjID >= long(m_objectValues.size()))
    {
        return _T("");
    }

    CString id;
    id = m_objectNames[ObjID];

    return id;
}

CString VisionInspectionResult::getObjectGroupID(long ObjID) const //kircheis_MED2.5
{
    if (ObjID < 0 || ObjID >= long(m_objectGroupID.size()))
    {
        return _T("");
    }

    CString id;
    id = m_objectGroupID[ObjID];

    return id;
}
