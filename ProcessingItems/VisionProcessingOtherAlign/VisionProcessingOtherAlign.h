#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CVisionProcessingOtherAlignPara;
class CDlgVisionProcessingOtherAlign;
class ImageProcPara;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class ImageLotView;

//HDR_6_________________________________ Header body
//
class __VISION_OTHER_ALIGN_CLASS VisionProcessingOtherAlign : public VisionProcessing
{
public:
    VisionProcessingOtherAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingOtherAlign(void);

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
    CVisionProcessingOtherAlignPara* m_VisionPara;
    CDlgVisionProcessingOtherAlign* m_pVisionInspDlg;

    long DoAlign(const bool detailSetupMode);

    long OtherEdgeDetect_Left(const Ipvm::Image8u& image, Ipvm::Rect32s rtSearchROI);
    long OtherEdgeDetect_Top(const Ipvm::Image8u& image, Ipvm::Rect32s rtSearchROI);
    long OtherEdgeDetect_Right(const Ipvm::Image8u& image, Ipvm::Rect32s rtSearchROI);
    long OtherEdgeDetect_Bottom(const Ipvm::Image8u& image, Ipvm::Rect32s rtSearchROI);

    long RoughAlign_LineAlign(long nDirection);
    long SetResult(long nSearchROIID);

    void SetDebugInfo();

    CPI_EdgeDetect* m_pEdgeDetect;
    CPI_Blob* m_pBlob;

    VisionAlignResult* m_sEdgeAlignResult;
    void GetEdgeAlignResult(); //kircheis_USI_OA

    // Align Result...
    float m_fCalcTime;

private:
    // 저장 후 전송될 ROI
    std::vector<Ipvm::Rect32r> m_vecfrtAlignROI;
    std::vector<Ipvm::Rect32s> m_vecSearchROI;

    // Debug View
    std::vector<Ipvm::Point32r2> m_vecptLeftTop;
    std::vector<Ipvm::Point32r2> m_vecptRightTop;
    std::vector<Ipvm::Point32r2> m_vecptRightBottom;
    std::vector<Ipvm::Point32r2> m_vecptLeftBottom;

    std::vector<Ipvm::Point32r2> vecLeftEdgeAlignPoints;
    std::vector<Ipvm::Point32r2> vecTopEdgeAlignPoints;
    std::vector<Ipvm::Point32r2> vecRightEdgeAlignPoints;
    std::vector<Ipvm::Point32r2> vecBottomEdgeAlignPoints;

    std::vector<Ipvm::Point32r2> vecLeftEdgeAlignPoints_Total;
    std::vector<Ipvm::Point32r2> vecTopEdgeAlignPoints_Total;
    std::vector<Ipvm::Point32r2> vecRightEdgeAlignPoints_Total;
    std::vector<Ipvm::Point32r2> vecBottomEdgeAlignPoints_Total;
};
