#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "VisionWarpageShapeResult.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ArchiveAllType;
class VisionInspectionResult;
class Analysis;

//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ VisionDeviceResult
{
public:
    VisionDeviceResult();
    ~VisionDeviceResult();

    Ipvm::Rect32s GetRoiForReview(const Ipvm::Rect32s& roi) const;

    void Init();
    void Serialize(ArchiveAllType& ar);
    void SetInspectResults(long nTheadNum, long nTrayID, long nInspectionRepeatIndex, long nPocketID, long nScanID,
        long nPane, long nTotalReuslt, std::vector<VisionInspectionResult*> vecVisionInspResult, CString strMajorID,
        std::vector<CString> vecStrSub2DID, std::vector<CString> vecStrBaby2DID,
        VisionWarpageShapeResult visionWarpageShapeResult,
        std::vector<VisionWarpageShapeResult> vecLidWarpageShapeResult); //kircheis_InspLid

    void SetInspectResults(long nTheadNum, long nTrayID, long nInspectionRepeatIndex, long nPocketID, long nScanID,
        long nPane, long nTotalReuslt, CString strColorImageName,
        std::vector<VisionInspectionResult*> vecVisionInspResult, CString strMajorID,
        std::vector<CString> vecStrSub2DID, std::vector<CString> vecStrBaby2DID,
        VisionWarpageShapeResult visionWarpageShapeResult,
        std::vector<VisionWarpageShapeResult> vecLidWarpageShapeResult, float pocketShiftY_TR = -999.f);

    void SetinspectionStartTime(const SYSTEMTIME i_SysTime);
    SYSTEMTIME GetinspectionStartTime();

#if defined(USE_BY_HANDLER)
    void SetInspectResults(long nTheadNum, long nTrayID, long nPocketID, long nPane, long nTotalReuslt,
        std::vector<VisionInspectionResult> vecVisionInspResult);
    void SetInspectResults(std::vector<VisionInspectionResult> vecVisionInspResult);
    VisionDeviceResult& operator=(const VisionDeviceResult& Obj);
    void DeleteMemVisionResult();
#endif

public:
    Analysis* m_analysis;

    long m_nThreadID;
    long m_nProbeID;
    long m_nTrayID;
    long m_nInspectionRepeatIndex;
    long m_nPocketID;
    long m_nScanID; // 핸들러에서 필요해서 임시로 넣었음. 추후 변경 예정
    long m_nPane;
    long m_nTotalResult;
    CString m_strReviewColorImageName; // 20190619 Review에서 both 설정한 경우 1개 color image 남김
    CString m_strReviewColorImageName2OnlyHandler;

    Ipvm::Rect32s m_reviewRoi; // = Pane Roi (와 같음)
    std::vector<VisionInspectionResult> TempResult;
    std::vector<VisionInspectionResult*> m_vecVisionInspResult;
    std::vector<VisionInspectionResult*> m_vecTempResult;

    CString m_strMajor2DID;
    std::vector<CString> m_vecStrSub2DID;
    std::vector<CString> m_vecStrBaby2DID;
    VisionWarpageShapeResult m_visionWarpageShapeResult;
    std::vector<VisionWarpageShapeResult> m_vecVisionLidWarpageShapeResult; //kircheis_InspLid
    SYSTEMTIME m_inspectionStartTime; //mc_Vision inspection StartTime

    float m_pocketShiftY_TR; //Pocket Shift Y [Unit:mm](TR기준)

private:
    void InitSystemTime();
};
