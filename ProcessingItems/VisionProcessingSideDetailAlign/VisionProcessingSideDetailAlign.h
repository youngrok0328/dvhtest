#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CDlgVisionProcessingSideDetailAlign;
class CPI_EdgeDetect;
class VisionImageLot;
class VisionProcessingSideDetailAlignPara;
class SideAlignParameters;

//HDR_6_________________________________ Header body
//
class __VISION_PROCESSING_SIDE_DETAIL_ALIGN_CLASS VisionProcessingSideDetailAlign : public VisionProcessing
{
public:
    VisionProcessingSideDetailAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingSideDetailAlign(void);

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
    VisionProcessingSideDetailAlignPara* m_VisionPara;
    CDlgVisionProcessingSideDetailAlign* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    BOOL DoInsp(const bool detailSetupMode, const Ipvm::Image8u& i_substrateImage, const Ipvm::Image8u& i_glassImage,
        const Ipvm::Image8u& i_glassImageLR);

    BOOL IsGlassCorePackage();

private:
    VisionAlignResult* m_sEdgeAlignResult;
    CPI_EdgeDetect* m_pEdgeDetect;

    BOOL DoAlign(const bool detailSetupMode, const Ipvm::Image8u& i_substrateImage, const Ipvm::Image8u& i_glassImage,
        const Ipvm::Image8u& i_glassImageLR);
    BOOL GetBodyAlignResult(VisionAlignResult* o_pEdgeAlignResult);
    BOOL GetDefaultPositionX_EachROI(const Ipvm::Rect32s& i_rtPackage, const long& i_nPointNum,
        std::vector<float>&
            o_vecfPosXpxl); //Top/Bottom의 Search ROI의 X 좌표는 모두 동일하니 별도의 변수에 기본 좌표 X를 계산해서 이 후 연산 과정에서 참조한다.
    BOOL GetTopBottomEdgeSearchROI(const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfPosXpxl,
        const float& i_fOffsetFromTopOutLinePxl, const float& i_fOffsetFromBottomOutLinePxl,
        const long& i_nSearchWidthPxl, const long& i_nSearchLengthPxl, std::vector<Ipvm::Rect32s>& o_vecrtTopSearchROI,
        std::vector<Ipvm::Rect32s>& o_vecrtBottomSearchROI); // 이함수는 Offset의 vector를 생성 후 아래 함수를 호출
    BOOL GetTopBottomEdgeSearchROI(const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfPosXpxl,
        const std::vector<float>& i_vecfOffsetFromTopOutLinePxl,
        const std::vector<float>& i_vecfOffsetFromBottomOutLinePxl, const long& i_nSearchWidthPxl,
        const long& i_nSearchLengthPxl, std::vector<Ipvm::Rect32s>& o_vecrtTopSearchROI,
        std::vector<Ipvm::Rect32s>& o_vecrtBottomSearchROI);
    BOOL GetAllDirectionEdgePoints(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& i_rtSearchROI,
        const long& i_nSearchDirection, const long& i_nEdgeDirection, const float& i_fEdgeThreshold,
        const BOOL& i_bIsFirstEdge, const long& i_nEdgeGap, const BOOL& i_bNoNeedAllPointDetection,
        std::vector<Ipvm::Point32r2>&
            o_vecptEdgePoint); // i_nEdgeGap이 0이하이면 ROI내에서 하나만, 1 이상이면 ROI안에서 그 Gap으로 여러개
    BOOL DoNormalUnitSideAlign(const bool detailSetupMode, const Ipvm::Image8u& i_substrateImage,
        const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfPosXpx,
        std::vector<Ipvm::Point32r2>& o_vecptAlignPoint);
    BOOL DoGlassCoreUnitSideAlign(const bool detailSetupMode, const Ipvm::Image8u& i_substrateImage,
        const Ipvm::Image8u& i_glassImage, const Ipvm::Image8u& i_glassImageLR, const Ipvm::Rect32s& i_rtPackage,
        const std::vector<float>& i_vecfSubstratePosXpx, const std::vector<float>& i_vecfGlassPosXpx,
        std::vector<Ipvm::Point32r2>& o_vecptTopAlignPoint, std::vector<Ipvm::Point32r2>& o_vecptBottomAlignPoint,
        std::vector<Ipvm::Point32r2>& o_vecptGlassAlignPoint);

    BOOL GetGlassCoreSubstrateTopBottomOuterEdgePoint(const bool detailSetupMode, const Ipvm::Image8u& i_substrateImage,
        const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfSubstratePosXpx,
        std::vector<Ipvm::Point32r2>& o_vecptTopAlignPoint, std::vector<Ipvm::Point32r2>& o_vecptBottomAlignPoint);
    BOOL GetGlassCoreSubstrateTopBottomInnerEdgePoint(const bool detailSetupMode, const Ipvm::Image8u& i_glassImage,
        const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfSubstratePosXpx,
        std::vector<Ipvm::Point32r2>& o_vecptTopAlignPoint, std::vector<Ipvm::Point32r2>& o_vecptBottomAlignPoint);
    BOOL GetGlassCoreSubstrateLeftRightEdgePoint(const bool detailSetupMode, const Ipvm::Image8u& i_substrateImage,
        const bool i_bIsTop, const Ipvm::Quadrangle32r& i_qrtSubstrate,
        std::vector<Ipvm::Point32r2>& o_vecptLeftAlignPoint, std::vector<Ipvm::Point32r2>& o_vecptRightAlignPoint);
    BOOL GetRoughAlignResultROI(const bool detailSetupMode, const Ipvm::Image8u& i_glassImage, const BOOL& bIsGlass,
        std::vector<Ipvm::Rect32s>& io_vecrtTopSearchROI, std::vector<Ipvm::Rect32s>& io_vecrtBottomSearchROI);
    BOOL RemakeEdgeSearchRoiByRoughAlign(const Ipvm::LineEq32r& i_lineEqVer,
        const std::vector<Ipvm::Point32r2>& i_vecfptPonit, const long& nDestWidth, const long& nDestLength,
        Ipvm::Rect32s& io_rtSrcDstSearchROI);

    BOOL RunSingleObjectAlign(const bool detailSetupMode, const Ipvm::Image8u& i_image, const Ipvm::Image8u& i_imageLR,
        const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfPosXpx,
        const std::vector<float>& i_vecfOffsetFromTopOutLinePxl,
        const std::vector<float>& i_vecfOffsetFromBottomOutLinePxl, const SideAlignParameters& alignParams,
        const BOOL& bIsGlass, std::vector<Ipvm::Point32r2>& o_vecptAlignPoint);
    BOOL MakeROI(const Ipvm::Point32r2& i_fptCenter, const long& i_nHalfWidth, const long& i_nHalfLength,
        Ipvm::Rect32s& o_rtROI);
    BOOL MakeLeftRightROIByThickness(const Ipvm::Point32r2& i_fptRef, const long& i_nHalfSearchLengthPxl,
        const long& i_nThicknessPxl, const float& i_fSearchWidthRatio, const bool& i_bIsTop,
        Ipvm::Rect32s& o_rtROI); //Top Substrate에는 i_fptRef에 Bottom Point, Bottom에는 반대로
    BOOL GetEdgeSearchInfo(const Ipvm::Rect32s& i_rtSearchROI, const long& i_nSearchDirection,
        Ipvm::Point32s2& o_ptStart, long& o_SearchWidth, long& o_SearchLength);
    BOOL GetEdgeSearchInitInfo(const Ipvm::Rect32s& i_rtSearchROI, const long& i_nSearchDirection,
        const long& i_nGapOffset, Ipvm::Point32s2& o_ptStart, Ipvm::Point32s2& o_ptOffset);
    BOOL GetLeftRightSearchROI(const Ipvm::Rect32s& i_rtPackage, const float& i_fSearchWidthLRratio,
        const float& i_fSearchLengthLRum, Ipvm::Rect32s& o_rtLeftSearchROI, Ipvm::Rect32s& o_rtRightSearchROI);
    BOOL ReAlignOutermostPoint(const Ipvm::LineEq32r& lineEqLeft, const Ipvm::LineEq32r& lineEqRight,
        std::vector<Ipvm::Point32r2>& io_vecptTopPoint, std::vector<Ipvm::Point32r2>& io_vecptBottomPoint);
    BOOL ReAlignOutermostPoint(const std::vector<Ipvm::Point32r2>& i_vecfptLeft,
        const std::vector<Ipvm::Point32r2>& i_vecfptRight, std::vector<Ipvm::Point32r2>& io_vecptTopPoint,
        std::vector<Ipvm::Point32r2>& io_vecptBottomPoint);
    BOOL MergeTopBottomAlignPoint(const std::vector<Ipvm::Point32r2>& i_vecptTopAlignPoint,
        const std::vector<Ipvm::Point32r2>& i_vecptBottomAlignPoint,
        std::vector<Ipvm::Point32r2>& o_vecptTotalAlignPoint);

    BOOL SetSurfaceMask(const std::vector<Ipvm::Point32r2>& i_vecptAlignPoint, const CString& i_strMaskName);

    BOOL CompensateForOrthogonalOutermostPoints(
        std::vector<Ipvm::Point32r2>& io_vecptTopPoint, std::vector<Ipvm::Point32r2>& io_vecptBottomPoint);

    //Inspection Result Overlay Draw
    bool DrawInspectioOverlay(
        VisionInspectionOverlayResult* overlayResult, const std::vector<Ipvm::Point32r2>& i_vecfptAlignPoint);
    std::vector<Ipvm::Point32r2> m_vecfptNormalAlignPoint;
    std::vector<Ipvm::Point32r2> m_vecfptGlassAlignPoint;
    std::vector<Ipvm::Point32r2> m_vecfptTopSubstrateAlignPoint;
    std::vector<Ipvm::Point32r2> m_vecfptBottomSubstrateAlignPoint;

    //Debug Info용
    //다른 Data는 생성 즉시 등록 가능하나 여기 Data는 여러 함수로 쪼개져 생성되므로 멤버 변수로 수집하여 등록이 필요하다.
    //멤버로 선언할 DebugInfo용 변수들은 Setup UI가 아니면 쓸일이 없는 애들로만 선언이 필요하다.
    std::vector<Ipvm::Rect32s> m_vecrtSubstrateRoughEdgeSearchROI_TB;
    std::vector<Ipvm::Point32r2> m_vecptSubstrateRoughEdgePoint_TB;
    std::vector<Ipvm::Rect32s> m_vecrtSubstrateEdgeSearchROI_TB;
    std::vector<Ipvm::Point32r2> m_vecptSubstrateEdgePoint_TB;
    std::vector<Ipvm::Rect32s> m_vecrtSubstrateEdgeSearchROI_LR;
    std::vector<Ipvm::Point32r2> m_vecptSubstrateEdgePoint_LR;

    std::vector<Ipvm::Rect32s> m_vecrtGlassRoughEdgeSearchROI_TB;
    std::vector<Ipvm::Point32r2> m_vecptGlassRoughEdgePoint_TB;
    std::vector<Ipvm::Rect32s> m_vecrtGlassEdgeSearchROI_TB;
    std::vector<Ipvm::Point32r2> m_vecptGlassEdgePoint_TB;
    std::vector<Ipvm::Rect32s> m_vecrtGlassEdgeSearchROI_LR;
    std::vector<Ipvm::Point32r2> m_vecptGlassEdgePoint_LR;

    void SetDebugInfoMultiLayerSpecROI(const bool detailSetupMode, const Ipvm::Rect32s& i_rtPackage);
    void ReSetDebugInfoMemberVariable();
    void SetDebugInfoMemberVariable(const bool detailSetupMode);

    void AddSubstrateRoughEdgeSearchROI_TB(
        const bool detailSetupMode, const std::vector<Ipvm::Rect32s>& vecrtSubstrateRoughEdgeSearchROI_TB);
    void AddSubstrateRoughEdgePoint_TB(
        const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptSubstrateRoughEdgePoint_TB);
    void AddSubstrateEdgeSearchROI_TB(
        const bool detailSetupMode, const std::vector<Ipvm::Rect32s>& vecrtSubstrateEdgeSearchROI_TB);
    void AddSubstrateEdgePoint_TB(
        const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptSubstrateEdgePoint_TB);
    void AddSubstrateEdgeSearchROI_LR(
        const bool detailSetupMode, const Ipvm::Rect32s& rtSubstrateLeftROI, const Ipvm::Rect32s& rtSubstrateRightROI);
    void AddSubstrateEdgePoint_LR(
        const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptSubstrateEdgePoint_LR);

    void AddGlassRoughEdgeSearchROI_TB(
        const bool detailSetupMode, const std::vector<Ipvm::Rect32s>& vecrtGlassRoughEdgeSearchROI_TB);
    void AddGlassRoughEdgePoint_TB(
        const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptGlassRoughEdgePoint_TB);
    void AddGlassEdgeSearchROI_TB(
        const bool detailSetupMode, const std::vector<Ipvm::Rect32s>& vecrtGlassEdgeSearchROI_TB);
    void AddGlassEdgePoint_TB(const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptGlassEdgePoint_TB);
    void AddGlassEdgeSearchROI_LR(
        const bool detailSetupMode, const Ipvm::Rect32s& rtGlassLeftROI, const Ipvm::Rect32s& rtGlassRightROI);
    void AddGlassEdgePoint_LR(const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptGlassEdgePoint_LR);

    void AddRoughEdgeSearchROI_TB(
        const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Rect32s>& vecrtRoughEdgeSearchROI_TB);
    void AddRoughEdgePoint_TB(
        const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Point32r2>& vecptRoughEdgePoint_TB);
    void AddEdgeSearchROI_TB(
        const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Rect32s>& vecrtEdgeSearchROI_TB);
    void AddEdgePoint_TB(
        const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Point32r2>& vecptEdgePoint_TB);
    void AddEdgeSearchROI_LR(const bool detailSetupMode, const bool bIsGlass, const Ipvm::Rect32s& rtLeftROI,
        const Ipvm::Rect32s& rtRightROI);
    void AddEdgePoint_LR(
        const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Point32r2>& vecptEdgePoint_LR);
};
