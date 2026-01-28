#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ArchiveAllType;
class CiDataBase;
enum class II_RESULT_TYPE : long;
enum class HostReportCategory : long;

//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ SurfaceSpec
{
public:
    long nType;
    CString strName;
    long nColor; // 0 : Bright 1: Dark
    float fMax;
    float fMin;
    CString strUnit; // %, px, GV, mil, mm, um...
};

class __VISION_HOST_COMMON_API__ VisionInspectionSpec
{
public:
    VisionInspectionSpec() = default;
#if defined(USE_BY_HANDLER)
    VisionInspectionSpec(
        LPCTSTR inspName, LPCTSTR specName, LPCTSTR shortName, HostReportCategory hostReportCategory, BOOL bUseIn);
#endif
    VisionInspectionSpec(LPCTSTR moduleGuid, LPCTSTR specGuid);
    VisionInspectionSpec(LPCTSTR moduleGuid, LPCTSTR specGuid, LPCTSTR inspName, LPCTSTR specName, LPCTSTR shortName,
        LPCTSTR sortingName, LPCTSTR unit, HostReportCategory hostReportCategory, II_RESULT_TYPE resultType,
        BOOL bUseIn, float fMarginalMinIn, float fPassMinIn, float fPassMaxIn, float fMarginalMaxIn, BOOL bUseMinIn,
        BOOL bUseMaxIn, BOOL bUseMarginalIn);
    VisionInspectionSpec(const VisionInspectionSpec& rhs) = default;

    VisionInspectionSpec& operator=(const VisionInspectionSpec& src) = default;

    bool Link(const bool save, CiDataBase& db);
    CString GetSpecFullName()
    {
        return m_fullName;
    } // for Host
    void MakeSpecFullName();

public:
    // 아래 GUID 는 레시피 저장시 키 값으로 사용하므로 소스 코드 레벨에서도 변경하면 안됨.
    CString m_moduleGuid;
    CString m_specGuid;

    CString m_inspName; ///< 검사항목 이름
    CString m_specName; ///< Spec 이름
    CString m_fullName; ///< Host에서 사용하는 이름
    CString m_shortName; ///< Host에서 사용할 축약명
    CString m_sortingName; ///< Host에서 사용할 소팅 이름
    CString m_unit; ///< 검사항목 단위
    HostReportCategory
        m_hostReportCategory; ///< Host에서 사용할 검사항목 Type, Host에 보내기 위하여 표준 데이터타입 사용
    II_RESULT_TYPE m_resultType;
    CString m_reportName; // 리포트에 표기 이름 For Host

    BOOL m_use; ///< 개런티 여부
    float m_marginalMin; ///< Marginal Min
    float m_passMin; ///< Pass Min
    float m_passMax; ///< Pass Max
    float m_marginalMax; ///< Marginal Max
    BOOL m_useMin; ///< Pass Min 값 사용 여부
    BOOL m_useMax; ///< Pass Max 값 사용 여부
    BOOL m_useMarginal; ///< Marginal 값 사용 여부

    std::vector<std::vector<SurfaceSpec>> m_vecSurfaceCriteriaSpec;
    std::vector<float> m_vecfNominal; ///< Nomial ( Layer에 따라 Spec이 다르다. )
};

__VISION_HOST_COMMON_API__ ArchiveAllType& operator>>(ArchiveAllType& ar, SurfaceSpec& dst);
__VISION_HOST_COMMON_API__ ArchiveAllType& operator<<(ArchiveAllType& ar, SurfaceSpec& src);
__VISION_HOST_COMMON_API__ ArchiveAllType& operator>>(ArchiveAllType& ar, VisionInspectionSpec& dst);
__VISION_HOST_COMMON_API__ ArchiveAllType& operator<<(ArchiveAllType& ar, VisionInspectionSpec& src);
