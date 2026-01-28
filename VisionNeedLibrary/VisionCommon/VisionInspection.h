#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "VisionInspectionResultGroup.h"
#include "VisionProcessing.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"

//HDR_3_________________________________ External library headers
#include "../../DefineModules/dA_Base/DefineForTR.h"

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionInspectionDefaultSpec
{
public:
    VisionInspectionDefaultSpec(LPCTSTR specGuid, LPCTSTR specName, LPCTSTR shortName, LPCTSTR sortingName,
        LPCTSTR unit, HostReportCategory hostReportCategory, II_RESULT_TYPE resultType, BOOL bUseIn,
        float fMarginalMinIn, float fPassMinIn, float fPassMaxIn, float fMarginalMaxIn, BOOL bUseMinIn, BOOL bUseMaxIn,
        BOOL bUseMarginalIn);

public:
    CString m_specGuid;
    CString m_specName; ///< Spec 이름
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
};

class __VISION_COMMON_API__ VisionInspection : public VisionProcessing
{
public:
    VisionInspection() = delete;
    VisionInspection(LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspection();

public: // VisionProcessing virtual functions
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override;

public:
    VisionInspectionSpec* GetSpecByGuid(LPCTSTR guid);
    VisionInspectionSpec* GetSpecByName(LPCTSTR name);

    void SetSpec(LPCTSTR specName, BOOL bUse, BOOL bUseMax, BOOL bUseMin, BOOL bUseMarginal, float fMarginalMax,
        float fPassMax, float fPassMin, float fMarginalMin);

    void ResetSpec(); // 스펙을 기본 스펙으로 되돌린다.
    void ResetResult(); // 결과를 현재 스펙 구성에 맞게 재생성한다.

    std::vector<CString> ExportInspectionSpecToText(CString strVisionName); //kircheis_TxtRecipe

public:
    std::vector<VisionInspectionDefaultSpec> m_defaultFixedInspectionSpecs;
    std::vector<VisionInspectionSpec> m_fixedInspectionSpecs;
    std::vector<VisionInspectionSpec> m_variableInspectionSpecs;
    VisionInspectionResultGroup m_resultGroup;

protected:
    void AppendDetailTextResult(CString& io_text, long specObjectCount = -1);
    void AppendDetailMarkTextResult(std::vector<VisionInspectionResult*> i_vecpInspResult,
        std::vector<VisionInspectionSpec*> i_vecpInspSpec, long i_nObjectNum, CString& io_text);
};