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
class VisionInspectionDie2DPara;
class CDlgVisionInspectionDie2D;
class CPI_EdgeDetect;
class VisionImageLot;

//HDR_6_________________________________ Header body
//

class __VISION_INSP_LID_2D_CLASS__ VisionInspectionDie2D : public VisionInspection
{
public:
    VisionInspectionDie2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionDie2D(void);

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
    VisionInspectionDie2DPara* m_VisionPara;
    CDlgVisionInspectionDie2D* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    VisionAlignResult* m_sEdgeAlignResult;

    std::vector<VisionAlignResult> m_vecsDieAlignResult;
    std::vector<SDieLidPatchInfoDB> m_vecsDieInfoDB;

    Ipvm::LineEq32r m_linePackageCenterHor;
    Ipvm::LineEq32r m_linePackageCenterVer;
    std::vector<Ipvm::LineEq32r> m_veclineDieCenterHor;
    std::vector<Ipvm::LineEq32r> m_veclineDieCenterVer;
    std::vector<Ipvm::LineEq32r> m_veclineDieLeft;
    std::vector<Ipvm::LineEq32r> m_veclineDieTop;
    std::vector<Ipvm::LineEq32r> m_veclineDieRight;
    std::vector<Ipvm::LineEq32r> m_veclineDieBottom;

    CPI_EdgeDetect* m_pEdgeDetect;

    /////////////////////////////////////////////////
    BOOL GetDieInfoDB(); //DMS에서 Data를 가져와서 m_vecsDieInfoDB에 넣는다.
    BOOL CalcAlignedCenterLine(VisionAlignResult i_AlignResult, Ipvm::LineEq32r& o_HorLine, Ipvm::LineEq32r& o_VerLine);

    BOOL DoAlignLid(const bool detailSetupMode);
    BOOL MakeEdgeSearchROI(SDieLidPatchInfoDB dieInfoDB, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI);
    BOOL GetDieEdgePoint(
        Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge);
    BOOL CalcDieAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge, std::vector<Ipvm::Point32r2> vecfptTopEdge,
        std::vector<Ipvm::Point32r2> vecfptRightEdge, std::vector<Ipvm::Point32r2> vecfptBottomEdge,
        Ipvm::LineEq32r& o_dieAlignLeftLine, Ipvm::LineEq32r& o_dieAlignTopLine, Ipvm::LineEq32r& o_dieAlignRightLine,
        Ipvm::LineEq32r& o_dieAlignBottomLine, VisionAlignResult& o_dieAlignResult);
    BOOL DoInspDieOffsetX();
    BOOL DoInspDieOffsetY();
    BOOL DoInspDieOffsetTheta();
    BOOL DoInspDieSizeX();
    BOOL DoInspDieSizeY();

    Ipvm::Rect32r GetBodyRect();
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
};
