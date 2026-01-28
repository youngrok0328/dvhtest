#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineEq32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class VisionInspectionGlassSubstrate2DPara;
class CDlgVisionInspectionGlassSubstrate2D;
class CPI_EdgeDetect;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_GLASS_SUBSTRATE_2D_CLASS__ VisionInspectionGlassSubstrate2D : public VisionInspection
{
public:
    VisionInspectionGlassSubstrate2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionGlassSubstrate2D(void);

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
    VisionInspectionGlassSubstrate2DPara* m_VisionPara;
    CDlgVisionInspectionGlassSubstrate2D* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    VisionAlignResult* m_sEdgeAlignResult;

    std::vector<VisionAlignResult> m_vecsGlassSubstrateAlignResult;
    Ipvm::Rect32s
        m_rtGlassSubstrateSpecROI; //지역 변수로 생각을 해보았으나 각 검사 항목의 함수에서 INV 발생 시 Rej ROI로 써야하는데 매번 계산하면 시간 낭비라서 멤버로 만듬.
    std::vector<Ipvm::LineEq32r> m_vecLineGlassSubstrate;

    Ipvm::LineEq32r m_linePackageCenterHor;
    Ipvm::LineEq32r m_linePackageCenterVer;
    std::vector<Ipvm::LineEq32r> m_veclineGlassSubstrateCenterHor;
    std::vector<Ipvm::LineEq32r> m_veclineGlassSubstrateCenterVer;

    CPI_EdgeDetect* m_pEdgeDetect;

    /////////////////////////////////////////////////
    BOOL IsValidGlassSubstrateSpecInfo();
    BOOL GetGlassSubstrateSpecROI(Ipvm::Rect32s& o_rtSpecROI);
    BOOL CalcAlignedCenterLine(VisionAlignResult i_AlignResult, Ipvm::LineEq32r& o_HorLine, Ipvm::LineEq32r& o_VerLine);

    BOOL DoAlignGlassSubstrate(const bool detailSetupMode);
    BOOL MakeEdgeSearchROI(const Ipvm::Rect32s i_rtSpecROI, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI);
    BOOL GetGlassSubstrateEdgePoint(
        Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge);
    BOOL CalcGlassSubstrateAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge,
        std::vector<Ipvm::Point32r2> vecfptTopEdge, std::vector<Ipvm::Point32r2> vecfptRightEdge,
        std::vector<Ipvm::Point32r2> vecfptBottomEdge, VisionAlignResult& o_glassSubstrateAlignResult);
    BOOL DoInspGlassSubstrateOffsetX();
    BOOL DoInspGlassSubstrateOffsetY();
    BOOL DoInspGlassSubstrateOffsetTheta();
    BOOL DoInspGlassSubstrateSizeX();
    BOOL DoInspGlassSubstrateSizeY();

    Ipvm::Rect32r GetBodyRect();
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
};
