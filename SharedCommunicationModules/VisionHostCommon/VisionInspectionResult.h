#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "VisionInspectionStatisticsValue.h" //kircheis_VSV
#include "VisionInspectionSurfaceResult.h" //kircheis_Criteria

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ArchiveAllType;
class VisionInspectionSpec;
enum class II_RESULT_TYPE : long;
enum class HostReportCategory : long;

//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ VisionInspectionResult
{
public:
#if defined(USE_BY_HANDLER)
    VisionInspectionResult();
#else
    VisionInspectionResult() = delete;
#endif

    VisionInspectionResult(const VisionInspectionResult& rhs) = default;
    VisionInspectionResult(LPCTSTR moduleGuid, LPCTSTR specGuid, LPCTSTR inspName, LPCTSTR resultName,
        LPCTSTR targetName, LPCTSTR sortName, LPCTSTR unit, II_RESULT_TYPE resultType,
        HostReportCategory hostReportCategory);
    ~VisionInspectionResult() = default;

    VisionInspectionResult& operator=(const VisionInspectionResult& rhs) = default;

public:
    void Clear();
    void Resize(long nNum);
    long IsValueCheck(VisionInspectionSpec* pSpec, float fValue);
    BOOL SetValue_EachResult_AndTypicalValueBySingleSpec(const long objectIndex, LPCTSTR objectName, float errorValue,
        const VisionInspectionSpec& spec, const float objectPosX, const float objectPosY, float nominalValue = 0.f,
        long specifiedResult = 0 /*bool invalidCheck = FALSE*/); //kircheis_VSV
    //{{mc_아래의 녀석은 Component검사에서만 사용하는 부분 다른검사항목에서 절대 사용하지 말것
    //	BOOL SetValue_EachResult_AndTypicalValueBySingleSpecForComponent(const long objectIndex, LPCTSTR objectName, CString objectCompType, float errorValue, const VisionInspectionSpec &spec, const float objectPosX, const float objectPosY, float nominalValue = 0.f, long specifiedResult = 0/*bool invalidCheck = FALSE*/);//kircheis_VSV
    BOOL SetValue_EachResult_AndTypicalValueBySingleSpec(const long objectIndex, LPCTSTR objectName,
        CString objectGroupID, CString objectCompType, float errorValue, const VisionInspectionSpec& spec,
        const float objectPosX, const float objectPosY, float nominalValue = 0.f,
        long specifiedResult = 0); //kircheis_MED2.5
    void ResizeForComponent(long nNum);
    //}}
    BOOL SetInvalid_EachResult(const long objectIndex, LPCTSTR objectName, LPCTSTR objectGroupID,
        const float objectPosX, const float objectPosY, float nominalValue = 0.f); //kircheis_MED2.5
    void UpdateTypicalValue(VisionInspectionSpec* pSpec);
    void SetTotalResult();
    BOOL SetResult(long nIndex, long nResult);
    // 동적으로 결과 추가용.
    BOOL AddResult(long nResult, float fValue, CString sObjectID, CString sObjectGroupID,
        const Ipvm::Rect32s& rtRect); //kircheis_MED2.5
    BOOL AddSpecSize(float fSpecSize); // 임시 함수 스펙 크기
    void SetInvalid();
    BOOL SetValue(long nIndex, float fValue, float fSpecValue = 0.f);
    BOOL SetRect(long nIndex, const Ipvm::Rect32s& rtRect);
    BOOL SetRejectROI(const Ipvm::Rect32s& rtRect);
    CString GetInspFullName() const
    {
        return m_fullName;
    } // for Host

    void AddSurfaceCriteriaResult(const VisionInspectionSurfaceResult& src); //kircheis_Criteria

    //{{//kircheis_VSV
    void RegistStatisticsValue(
        float fNominalSpec, CString strObjID, float fEachValue, long nEachResult, BYTE bObjIsInvalid);
    void RegistStatisticsValue(CString strGroupID, float fNominalSpec, CString strObjID, float fEachValue,
        long nEachResult, BYTE bObjIsInvalid);
    //}}

public:
    CString m_moduleGuid;
    CString m_specGuid;
    CString m_inspName; ///< 검사항목 이름
    CString m_resultName; ///< Result 이름
    CString m_fullName; ///< Result 이름
    CString m_targetName;
    CString m_sortName; ///< Host에서 표시될 sort이름을 정의한다.
    CString m_itemUnit; ///< 검사항목 단위
    CString m_reviewImageFileName; ///< 리뷰이미지 저장 파일이름 (폴더제외)
    CString m_reviewImageFileName2OnlyHandler;

    HostReportCategory
        m_hostReportCategory; ///< Host에서 사용할 검사항목 Type, Host에 보내기 위하여 표준 데이터타입 사용
    II_RESULT_TYPE m_resultType;
    long m_seleteProbeResult; // 어떤 비전이 최악의 결과를 가지고 있는지 For Host

    long m_totalResult; ///< 각 항목 Result

    long m_minObjectIndex;
    long m_maxObjectIndex;
    long m_worstObjectIndex;

    long m_ItemTypeID; // 결과 Type을 원함 for Host

    float getObjectMinValue() const;
    float getObjectMinErrorValue() const;
    float getObjectMaxValue() const;
    float getObjectMaxErrorValue() const;
    float getObjectWorstValue() const;
    float getObjectWorstErrorValue() const;
    CString getObjectWorstID() const;
    CString getObjectID(long ObjID) const;
    CString getObjectGroupID(long ObjID) const; //kircheis_MED2.5

    CString m_LossCode; // 이것은 Host에서만 쓴다. 비전에서 serialize 해줄 필요 없다.
    CString m_strUnitDisposition; // 이것은 Host에서만 쓴다. 비전에서 serialize 해줄 필요 없다.

    std::vector<long> m_objectResults; ///< 각 개체 Result
    std::vector<Ipvm::Rect32s> m_objectRects; ///< 각 개체 위치
    std::vector<CString> m_objectNames; ///< 각 개체 ID
    std::vector<CString> m_objectGroupID; ///< 각 개체가 소속된 Group ID //kircheis_MED2.5
    std::vector<float> m_objectErrorValues; ///< 각 개체 정상 값 대비 오차값. 이 값으로 양불 판정을 수행
    std::vector<float> m_objectValues; ///< 각 개체 실제 측정 값
    std::vector<float> m_nominalValues; ///< 각 개체의 스펙 상 정상 값
    std::vector<CString> m_objectCompTypes; ///< mc_Component에 한해서만 존재 CompType을 보내달라는 요청
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::vector<BOOL> vecbWriteResult; ///< Result가 이미 입력됐는지 확인한다.
    std::vector<Ipvm::Rect32s> vecrtRejectROI; ///< 불량 개체 위치
    std::vector<Ipvm::Rect32s> vecrtMarginalROI; ///< Marginal 개체 위치

    long nDefectAreaSize; ///< 불량이 포함된 영역의 개수
    std::vector<long> vecnDefectAreaID; ///< 불량이 포함된 영역의 위치

    std::vector<float> m_objectPositionX;
    std::vector<float> m_objectPositionY;

    std::vector<VisionInspectionSurfaceResult> m_vecSurfaceCriteriaResult; //kircheis_Criteria
    std::vector<VisionInspectionStatisticsValue> m_vecInspStatisticsValue; //kircheis_VSV
};

__VISION_HOST_COMMON_API__ ArchiveAllType& operator>>(ArchiveAllType& ar, VisionInspectionResult& dst);
__VISION_HOST_COMMON_API__ ArchiveAllType& operator<<(ArchiveAllType& ar, VisionInspectionResult& src);
