//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionSurfaceResult.h"

//CPP_2_________________________________ This project's headers
#include "ArchiveAllType.h"
#include "VisionSurfaceCriteriaDefinitions.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionSurfaceResult::VisionInspectionSurfaceResult()
{
    Init();
}

VisionInspectionSurfaceResult::~VisionInspectionSurfaceResult()
{
    Init();
}

void VisionInspectionSurfaceResult::Init()
{
    m_bIs3DVision = false;
    m_strCriteriaName.Empty();
    m_strCriteriaResult.Empty();
    m_strCriteriaColor.Empty();
    m_fCriteriaROICenX = 0.f;
    m_fCriteriaROICenY = 0.f;
    m_rtCriteriaROI.SetRectEmpty();
    m_vecstrCriteriaValue.clear();
}

void VisionInspectionSurfaceResult::ResizeCriteria(bool bIs3DVision)
{
    Init();
    m_bIs3DVision = bIs3DVision;

    m_vecstrCriteriaValue.resize(long(VisionSurfaceCriteria_Column::Surface_Criteria_END));
}

void VisionInspectionSurfaceResult::SetData(const VisionInspectionSurfaceResult src)
{
    Init();
    m_bIs3DVision = src.m_bIs3DVision;
    m_strCriteriaName = src.m_strCriteriaName;
    m_strCriteriaResult = src.m_strCriteriaResult;
    m_strCriteriaColor = src.m_strCriteriaColor;
    m_rtCriteriaROI = src.m_rtCriteriaROI;
    m_fCriteriaROICenX = src.m_fCriteriaROICenX;
    m_fCriteriaROICenY = src.m_fCriteriaROICenY;

    long nSize = (long)src.m_vecstrCriteriaValue.size();
    m_vecstrCriteriaValue.resize(nSize);
    for (long idx = 0; idx < nSize; idx++)
        m_vecstrCriteriaValue[idx] = src.m_vecstrCriteriaValue[idx];
}

CArchive& operator<<(CArchive& ar, const VisionInspectionSurfaceResult& src)
{
    ar << src.m_bIs3DVision;
    ar << src.m_strCriteriaName;
    ar << src.m_strCriteriaResult;
    ar << src.m_strCriteriaColor;
    ar << src.m_rtCriteriaROI;
    ar << src.m_fCriteriaROICenX;
    ar << src.m_fCriteriaROICenY;

    ar << (long)src.m_vecstrCriteriaValue.size();
    for (auto& fValue : src.m_vecstrCriteriaValue)
        ar << fValue;

    return ar;
}

CArchive& operator>>(CArchive& ar, VisionInspectionSurfaceResult& dst)
{
    long nSize = 0;
    ar >> dst.m_bIs3DVision;
    ar >> dst.m_strCriteriaName;
    ar >> dst.m_strCriteriaResult;
    ar >> dst.m_strCriteriaColor;
    ar >> dst.m_rtCriteriaROI;
    ar >> dst.m_fCriteriaROICenX;
    ar >> dst.m_fCriteriaROICenY;
    ar >> nSize;
    dst.m_vecstrCriteriaValue.resize(nSize);
    for (auto& fValue : dst.m_vecstrCriteriaValue)
        ar >> fValue;

    return ar;
}
