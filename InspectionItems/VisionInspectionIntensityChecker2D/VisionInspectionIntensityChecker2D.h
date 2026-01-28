#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/LineEq32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class VisionInspectionIntensityChecker2DPara;
class CDlgVisionInspectionIntensityChecker2D;
class CPI_EdgeDetect;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_INTENSITYCHECKER_2D_CLASS__ VisionInspectionIntensityChecker2D : public VisionInspection
{
public:
    VisionInspectionIntensityChecker2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionIntensityChecker2D(void);

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
    VisionInspectionIntensityChecker2DPara* m_VisionPara;
    CDlgVisionInspectionIntensityChecker2D* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    VisionAlignResult* m_sEdgeAlignResult;

    Ipvm::LineEq32r m_linePackageCenterHor;
    Ipvm::LineEq32r m_linePackageCenterVer;

    CPI_EdgeDetect* m_pEdgeDetect;

    Ipvm::Image32u* m_histogram;
    Ipvm::Point32r2 m_ptPaneCenter;
    Ipvm::Rect32s m_rtPaneRect;

    std::vector<long> m_vecbItemUsage;
    std::vector<double> m_vecIntensityAVG;
    std::vector<long> m_vecIntensityMax;
    std::vector<long> m_vecIntensityMin;

    BOOL m_bUseBypassMode;

    /////////////////////////////////////////////////
    BOOL DoInsp(const bool detailSetupMode);
    BOOL InspIntensityCheck(const bool detailSetupMode, const long InspectionID,
        const Ipvm::Image8u image); //ROI 내부의 밝기값을 계산한다.
    BOOL GetCalibration(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& validRoi, double& calibrationValue);

    Ipvm::Rect32r GetBodyRect();

    void ResetROI();
    void SetSameROI();
};
