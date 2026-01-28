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
class VisionInspectionPatch2DPara;
class CDlgVisionInspectionPatch2D;
class CPI_EdgeDetect;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_PATCH_2D_CLASS__ VisionInspectionPatch2D : public VisionInspection
{
public:
    VisionInspectionPatch2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionPatch2D(void);

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
    VisionInspectionPatch2DPara* m_VisionPara;
    CDlgVisionInspectionPatch2D* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    VisionAlignResult* m_sEdgeAlignResult;

    std::vector<VisionAlignResult> m_vecsPatchAlignResult;
    std::vector<SDieLidPatchInfoDB> m_vecsPatchInfoDB;

    Ipvm::LineEq32r m_linePackageCenterHor;
    Ipvm::LineEq32r m_linePackageCenterVer;
    std::vector<Ipvm::LineEq32r> m_veclinePatchCenterHor;
    std::vector<Ipvm::LineEq32r> m_veclinePatchCenterVer;

    CPI_EdgeDetect* m_pEdgeDetect;

    /////////////////////////////////////////////////
    BOOL GetPatchInfoDB(); //DMS에서 Data를 가져와서 m_vecsPatchInfoDB에 넣는다.
    BOOL CalcAlignedCenterLine(VisionAlignResult i_AlignResult, Ipvm::LineEq32r& o_HorLine, Ipvm::LineEq32r& o_VerLine);

    BOOL DoAlignPatch(const bool detailSetupMode);
    BOOL MakeEdgeSearchROI(SDieLidPatchInfoDB patchInfoDB, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI);
    BOOL GetPatchEdgePoint(
        Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge);
    BOOL CalcPatchAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge, std::vector<Ipvm::Point32r2> vecfptTopEdge,
        std::vector<Ipvm::Point32r2> vecfptRightEdge, std::vector<Ipvm::Point32r2> vecfptBottomEdge,
        VisionAlignResult& o_patchAlignResult);
    BOOL DoInspPatchOffsetX();
    BOOL DoInspPatchOffsetY();
    BOOL DoInspPatchOffsetTheta();
    BOOL DoInspPatchSizeX();
    BOOL DoInspPatchSizeY();

    Ipvm::Rect32r GetBodyRect();
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
    Ipvm::Point32r2* m_pfptGetCenterPosbyPadAlign; //mc_PadAAlign의 결과
};
