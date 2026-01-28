//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionSpec.h"

//CPP_2_________________________________ This project's headers
#include "DBObject.h"
#include "VisionHostBaseDef.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
#if defined(USE_BY_HANDLER)
VisionInspectionSpec::VisionInspectionSpec(
    LPCTSTR inspName, LPCTSTR specName, LPCTSTR shortName, HostReportCategory hostReportCategory, BOOL bUseIn)
    : m_moduleGuid("")
    , m_specGuid("")
    , m_inspName(inspName)
    , m_specName(specName)
    , m_shortName(shortName)
    , m_unit("")
    , m_hostReportCategory(hostReportCategory)
    , m_resultType(II_RESULT_TYPE::MEASURED)
    , m_use(bUseIn)
    , m_marginalMin(0)
    , m_passMin(0)
    , m_passMax(0)
    , m_marginalMax(0)
    , m_useMin(FALSE)
    , m_useMax(FALSE)
    , m_useMarginal(FALSE)
{
    m_fullName.Format(_T("[%s]%s"), inspName, specName);
}
#endif

VisionInspectionSpec::VisionInspectionSpec(LPCTSTR moduleGuid, LPCTSTR specGuid)
    : m_moduleGuid(moduleGuid)
    , m_specGuid(specGuid)
    , m_hostReportCategory(HostReportCategory::NO_INSP)
    , m_resultType(II_RESULT_TYPE::MEASURED)
    , m_use(FALSE)
    , m_marginalMin(0.f)
    , m_passMin(0.f)
    , m_passMax(0.f)
    , m_marginalMax(0.f)
    , m_useMin(FALSE)
    , m_useMax(FALSE)
    , m_useMarginal(FALSE)
{
}

VisionInspectionSpec::VisionInspectionSpec(LPCTSTR moduleGuid, LPCTSTR specGuid, LPCTSTR inspName, LPCTSTR specName,
    LPCTSTR shortName, LPCTSTR sortingName, LPCTSTR unit, HostReportCategory hostReportCategory,
    II_RESULT_TYPE resultType, BOOL bUseIn, float fMarginalMinIn, float fPassMinIn, float fPassMaxIn,
    float fMarginalMaxIn, BOOL bUseMinIn, BOOL bUseMaxIn, BOOL bUseMarginalIn)
    : m_moduleGuid(moduleGuid)
    , m_specGuid(specGuid)
    , m_inspName(inspName)
    , m_specName(specName)
    , m_shortName(shortName)
    , m_sortingName(sortingName)
    , m_unit(unit)
    , m_hostReportCategory(hostReportCategory)
    , m_resultType(resultType)
    , m_use(bUseIn)
    , m_marginalMin(fMarginalMinIn)
    , m_passMin(fPassMinIn)
    , m_passMax(fPassMaxIn)
    , m_marginalMax(fMarginalMaxIn)
    , m_useMin(bUseMinIn)
    , m_useMax(bUseMaxIn)
    , m_useMarginal(bUseMarginalIn)
{
    MakeSpecFullName();
}

bool VisionInspectionSpec::Link(const bool save, CiDataBase& db)
{
    db[_T("ModuleGuid")].Link(save, m_moduleGuid);
    db[_T("SpecGuid")].Link(save, m_specGuid);
    db[_T("InspName")].Link(save, m_inspName);
    db[_T("SpecName")].Link(save, m_specName);
    db[_T("FullName")].Link(save, m_fullName);
    db[_T("ShortName")].Link(save, m_shortName);
    db[_T("SortingName")].Link(save, m_sortingName);
    //db[_T("Unit")].Link(save, m_unit);
    //db[_T("HostReportCategory")].Link(save, *(long *)&m_hostReportCategory);
    //db[_T("ResultType")].Link(save, *(long *)&m_resultType);

    db[_T("Use")].Link(save, m_use);
    db[_T("MarginalMin")].Link(save, m_marginalMin);
    db[_T("PassMin")].Link(save, m_passMin);
    db[_T("PassMax")].Link(save, m_passMax);
    db[_T("MarginalMax")].Link(save, m_marginalMax);
    db[_T("UseMin")].Link(save, m_useMin);
    db[_T("UseMax")].Link(save, m_useMax);
    db[_T("UseMarginal")].Link(save, m_useMarginal);

    return true;
}

void VisionInspectionSpec::MakeSpecFullName()
{
    m_fullName.Format(_T("[%s]%s"), (LPCTSTR)m_inspName, (LPCTSTR)m_specName);
}

ArchiveAllType& operator>>(ArchiveAllType& ar, SurfaceSpec& dst)
{
    ar >> dst.nType;
    ar >> dst.strName;
    ar >> dst.nColor;
    ar >> dst.fMax;
    ar >> dst.fMin;
    ar >> dst.strUnit;

    return ar;
}

ArchiveAllType& operator<<(ArchiveAllType& ar, SurfaceSpec& src)
{
    ar << src.nType;
    ar << src.strName;
    ar << src.nColor;
    ar << src.fMax;
    ar << src.fMin;
    ar << src.strUnit;

    return ar;
}

ArchiveAllType& operator>>(ArchiveAllType& ar, VisionInspectionSpec& dst)
{
    ar >> dst.m_moduleGuid;
    ar >> dst.m_specGuid;
    ar >> dst.m_inspName;
    ar >> dst.m_specName;
    ar >> dst.m_fullName;
    ar >> dst.m_shortName;
    ar >> dst.m_sortingName;
    ar >> dst.m_unit;

    // 영훈 20160623 : 이 다음 버전부터 풀도록 한다.
    ar >> *(long*)&dst.m_hostReportCategory;
    ar >> *(long*)&dst.m_resultType;

    ar >> dst.m_use;
    ar >> dst.m_marginalMin;
    ar >> dst.m_passMin;
    ar >> dst.m_passMax;
    ar >> dst.m_marginalMax;
    ar >> dst.m_useMin;
    ar >> dst.m_useMax;
    ar >> dst.m_useMarginal;

    // criteria 전체 Size 받기
    long nSize = 0;
    ar >> nSize;
    dst.m_vecSurfaceCriteriaSpec.resize(nSize);

    // 개별 Size 받기
    long nSize2 = 0;
    for (long n = 0; n < nSize; n++)
    {
        ar >> nSize2;
        dst.m_vecSurfaceCriteriaSpec[n].resize(nSize2);
    }

    // 개별 Data 받기
    for (int n = 0; n < nSize; n++)
    {
        for (long n2 = 0; n2 < (long)dst.m_vecSurfaceCriteriaSpec[n].size(); n2++)
        {
            ar >> dst.m_vecSurfaceCriteriaSpec[n][n2];
        }
    }

    ar >> nSize;
    dst.m_vecfNominal.resize(nSize);
    for (long n = 0; n < nSize; n++)
    {
        ar >> dst.m_vecfNominal[n];
    }

#ifdef EXCHANGE_SURFACE_ITEM_NAME
    if (dst.m_hostReportCategory == HostReportCategory::SURFACE)
    {
        dst.m_specName = dst.m_inspName;
        dst.m_inspName = _T("Surface");
        dst.m_fullName.Format(_T("[%s]%s"), LPCTSTR(dst.m_inspName), LPCTSTR(dst.m_specName));
    }
#endif

    return ar;
}

ArchiveAllType& operator<<(ArchiveAllType& ar, VisionInspectionSpec& src)
{
    ar << src.m_moduleGuid;
    ar << src.m_specGuid;
    ar << src.m_inspName;
    ar << src.m_specName;
    ar << _T("[") + src.m_inspName + _T("]") + src.m_specName;
    ar << src.m_shortName;
    ar << src.m_sortingName;
    ar << src.m_unit;

    // 영훈 20160623 : 이 다음 버전부터 풀도록 한다.
    ar << *(long*)&src.m_hostReportCategory;
    ar << *(long*)&src.m_resultType;

    ar << src.m_use;
    ar << src.m_marginalMin;
    ar << src.m_passMin;
    ar << src.m_passMax;
    ar << src.m_marginalMax;
    ar << src.m_useMin;
    ar << src.m_useMax;
    ar << src.m_useMarginal;

    // criteria 전체 Size 전송
    long nSize = (long)src.m_vecSurfaceCriteriaSpec.size();
    ar << nSize;

    // 개별 Size 전송
    for (int n = 0; n < nSize; n++)
    {
        ar << (long)src.m_vecSurfaceCriteriaSpec[n].size();
    }

    // 개별 Data 전송
    for (int n = 0; n < nSize; n++)
    {
        for (int n2 = 0; n2 < (long)src.m_vecSurfaceCriteriaSpec[n].size(); n2++)
        {
            ar << src.m_vecSurfaceCriteriaSpec[n][n2];
        }
    }

    nSize = (long)src.m_vecfNominal.size();
    ar << nSize;
    for (long n = 0; n < nSize; n++)
    {
        ar << src.m_vecfNominal[n];
    }

    return ar;
}
