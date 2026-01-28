#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Quadrangle32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionOTITRPara;
class CDlgVisionInspectionOTITR;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class VisionInspectionSpec;
class VisionInspectionResult;
class SprocketHoleInfo;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_OTI_TR_CLASS__ VisionInspectionOTITR : public VisionInspection
{
public:
    VisionInspectionOTITR(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionOTITR(void);

public: // VisionProcessing virtual functions
    virtual BOOL DoInspection(const bool detailSetupMode,
        const enSideVisionModule i_ModuleStatus = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual BOOL OnInspection() override;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override;
    virtual void ResetSpecAndPara() override;
    virtual void ResetResult() override;
    virtual void GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode) override;
    virtual void AppendTextResult(CString& textResult) override;
    virtual long ShowDlg(const ProcessingDlgInfo& procDlgInfo) override;
    virtual void CloseDlg() override;
    virtual std::vector<CString> ExportRecipeToText() override; //kircheis_TxtRecipe

public:
    BOOL DoInsp(const bool detailSetupMode);

    void SetFillPocketInfoInImageLot(bool receivedFillPocket);

    VisionInspectionOTITRPara* m_VisionPara;
    CDlgVisionInspectionOTITR* m_pVisionInspDlg;

    CPI_EdgeDetect* m_pEdgeDetect;

    float m_fCalcTime;

    void SetReceivedFillPocketStatus(const bool& receivedFillPocket);
    bool GetReceivedFillPocketStatus() const;

private:
    bool InspOTI_EmptyStatus(const bool detailSetupMode, const Ipvm::Rect32s& i_rtPocket);
    bool InspOTI_CoverTapeShift(const bool detailSetupMode, const bool& succeedCoverTapeAlign,const Ipvm::LineEq32r& coverTapeLeftLine,
        const Ipvm::LineEq32r& coverTapeRightLine, const Ipvm::Quadrangle32r& inspectionArea, const Ipvm::Rect32s& invalidRejROI);
    bool InspOTI_SealingShift(const bool detailSetupMode, const bool& succeedSealingAlign, const Ipvm::Rect32s& invalidRejROI, 
        const std::vector<Ipvm::LineEq32r>& leftSealingLine, const std::vector<Ipvm::LineEq32r>& rightSealingLine);
    bool InspOTI_SealingWidth(
        const bool detailSetupMode, const bool& succeedSealingAlign, const Ipvm::Rect32s& invalidRejROI, 
        const std::vector<Ipvm::Point32r2>& vecLeftSealingLeft,
        const std::vector<Ipvm::Point32r2>& vecLeftSealingRight, 
        const std::vector<Ipvm::Point32r2>& vecRightSealingLeft,
        const std::vector<Ipvm::Point32r2>& vecRightSealingRight, 
        const std::vector<Ipvm::LineEq32r>& leftSealingLine,
        const std::vector<Ipvm::LineEq32r>& rightSealingLine); //여기에서 Sealing LineEq까지 끌어다 쓰는 이유는 추출 못한 Edge로 인한 불량 ROI 생성을 위해서임.
    bool InspOTI_SealingSpan(
        const bool detailSetupMode, const bool& succeedSealingAlign, const Ipvm::Rect32s& invalidRejROI, 
        const std::vector<Ipvm::LineEq32r>& leftSealingLine,
        const std::vector<Ipvm::LineEq32r>& rightSealingLine);
    //bool InspOTI_SealingCutting(
    //    const bool detailSetupMode, const bool& succeedSealingAlign, const Ipvm::Rect32s& invalidRejROI);
    bool InspOTI_SealingParallelism(
        const bool detailSetupMode, const bool& succeedSealingAlign, const Ipvm::Rect32s& invalidRejROI, 
        const std::vector<Ipvm::Quadrangle32r>& sealingAlignQrt);


    void SetDebugInfo(const bool detailSetupMode);
    VisionAlignResult* m_sEdgeAlignResult;

    SprocketHoleInfo* m_sprocketHoleInfo;
    Ipvm::Rect32s m_rtInspection;
    Ipvm::Rect32s m_rtPocket; // 현재 Pocket 영역
    Ipvm::Quadrangle32r m_qrDevice; // 현재 Pocket 영역 (Quadrangle 형태로 변환)
    bool CollectAlignInfoInDebugInfo(const Ipvm::Point32r2& pointImageCenter);

    bool m_receivedFillPocket; // Handler로부터 받은 Fill Pocket 여부
    bool m_alignedFillPocket; // Detail Tape Align에서 확인한 Fill Pocket 여부

    bool GetCheckCoverTapeAlign();
    bool GetCheckSealingAlign();

    bool GetSprocketHoleCenter(Ipvm::Point32r2& leftHolesCenter, Ipvm::Point32r2& rightHolesCenter);

    bool DetectVerticalAxisEdgePointaInROI(const Ipvm::Image8u& image, const Ipvm::Rect32s& searchROI,
        const float& edgeThreshold, const long& edgeSearchDirection /*LEFT/RIGHT*/, 
        const long& edgeDetectMode,
        const long& edgeSearchWidth, const float& edgeJumpStep, const bool& isFirstEdge, const bool& needDetectAllPoint,
        const bool& skipNoDetectedAllPoint, std::vector<Ipvm::Point32r2>& edgePoints);


    //Cover Tape Align과 Sealing Align 모두 사용하는 함수
    bool GetObjectEdgePoints(const bool detailSetupMode, const long& edgeObject,
        const Ipvm::Rect32s& leftEdgeSearchROI, const Ipvm::Rect32s& rightEdgeSearchROI, 
        std::vector<Ipvm::Point32r2>& leftEdgePoint, std::vector<Ipvm::Point32r2>& rightEdgePoint);
    bool GetTopBottomBoundaryFittingLine(const Ipvm::Rect32s& referenceROI,
        Ipvm::LineEq32r& topFittingLine, Ipvm::LineEq32r& bottomFittingLine);
    float GetParameterEdgeThreshold(const long& edgeObject);
    long GetParameterEdgeDetectMode(const long& edgeObject);
    long GetParameterEdgeDetectGap_um(const long& edgeObject);
    bool GetParameterIsFirstEdge(const long& edgeObject);
    LPCTSTR GetDebugInfoName(const long& edgeObject, const bool& isLeftEdge);
    bool GetObjectLineFitting(const std::vector<Ipvm::Point32r2> vecPoint, const Ipvm::LineEq32r& topFittingLine,
        const Ipvm::LineEq32r& bottomFittingLine, Ipvm::LineEq32r& fittingLine, Ipvm::LineSeg32r& fittingLineSeg);


    //Cover Tape Align 전용 함수
    bool AlignCoverTape(
        const bool detailSetupMode, const Ipvm::Point32r2& leftHolesCenter, const Ipvm::Point32r2& rightHolesCenter, Ipvm::LineEq32r& leftCoverTapeLine, 
        Ipvm::LineEq32r& rightCoverTapeLine, Ipvm::Quadrangle32r& coverTapeArea, Ipvm::Rect32s& mergedSearchROI); //mergedSearchROI는 Align 실패 시의 불량 ROI로 사용하기 위함.
    bool GetCoverTapeEdgeSearchROI(const bool detailSetupMode, const Ipvm::Point32r2& leftHolesCenter,
        const Ipvm::Point32r2& rightHolesCenter,
        Ipvm::Rect32s& leftCoverTapeEdgeSearchROI, Ipvm::Rect32s& rightCoverTapeEdgeSearchROI);

    //Sealing Align 전용 함수
    bool AlignSealing(const bool detailSetupMode, const Ipvm::Point32r2& leftHolesCenter, const Ipvm::Point32r2& rightHolesCenter, 
        std::vector<Ipvm::Point32r2>& vecLeftSealingLeft, std::vector<Ipvm::Point32r2>& vecLeftSealingRight, 
        std::vector<Ipvm::Point32r2>& vecRightSealingLeft, std::vector<Ipvm::Point32r2>& vecRightSealingRight, 
        std::vector<Ipvm::LineEq32r>& leftSealingLine, std::vector<Ipvm::LineEq32r>& rightSealingLine, 
        std::vector<Ipvm::Quadrangle32r>& sealingArea,
        Ipvm::Rect32s& mergedSearchROI); //mergedSearchROI는 Align 실패 시의 불량 ROI로 사용하기 위함.
    bool GetSealingEdgeSearchROI(const bool detailSetupMode, 
        const Ipvm::Point32r2& leftHolesCenter, const Ipvm::Point32r2& rightHolesCenter,
        Ipvm::Rect32s& leftSealingLeftSearchROI, Ipvm::Rect32s& leftSealingRightSearchROI, 
        Ipvm::Rect32s& rightSealingLeftSearchROI, Ipvm::Rect32s& rightSealingRightSearchROI);

    //검사 함수의 호출 함수
    void GetCrossPointForSealingSpan(const long& sprocketHoleID, const bool& existLeftHole, const bool& existRightHole,
       std::vector<Ipvm::Point32r2>& crossPoints);

 public:
    void GetCoverTapeSearchROI(std::vector<Ipvm::Rect32s>& searchROIs);
    void GetSealingSearchROI(std::vector<Ipvm::Rect32s>& searchROIs);

    void GetCoverTapeSearchLines(std::vector<Ipvm::LineSeg32r>& searchLines);
    void GetSealingSearchLines(std::vector<Ipvm::LineSeg32r>& searchLines);
};
