#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionBgaBallPixelQualityPara;
class CDlgVisionInspectionBgaBallPixelQuality;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class __VISION_BALL_PIXEL_QUALITY_CLASS__ VisionInspectionBgaBallPixelQuality : public VisionInspection
{
public:
    VisionInspectionBgaBallPixelQuality(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionBgaBallPixelQuality(void);

public: // VisionProcessing virtual functions
    virtual BOOL DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus) override;
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
    VisionInspectionBgaBallPixelQualityPara* m_VisionPara;
    CDlgVisionInspectionBgaBallPixelQuality* m_pVisionInspDlg;

    CPI_Blob* m_pBlob;

    float m_fCalcTime;

public:
    BOOL GetBallROI(); //m_vecrtBallCircleROI에 Ball 좌표 넣기//kircheis_BPQDebug
    BOOL DoBallPixelQualityInsp(const bool detailSetupMode, const Ipvm::Image8u& image);
    BOOL DoFillRatioInsp(long nRingID, CString strRingID, long nBallNum, float* pfValue);
    BOOL DoContrastInsp(long nRingID, CString strRingID, long nBallNum, float* pfValue);
    void SetDebugImageRingMask(
        const bool detailSetupMode, long nRingID, CString strRingID, const Ipvm::Image8u& i_image);
    void SetDebugImageRingBrightPixel(
        const bool detailSetupMode, long nRingID, CString strRingID, const Ipvm::Image8u& i_image);
    void SetDebugImageRingDarkPixel(
        const bool detailSetupMode, long nRingID, CString strRingID, const Ipvm::Image8u& i_image);

    std::vector<Ipvm::Rect32r> m_vecrtBallCircleROI; //Ball Pixel Quality를 계산하기 위한 영역
    void MakeBallCircleROI(long nBallNum, long nRingID, Ipvm::Rect32s* o_prtEndRingROI);
    void MakeBallEllipse(
        long nBallNum, long nRingID, Ipvm::EllipseEq32r* o_psRingBeginEllipse, Ipvm::EllipseEq32r* o_psRingEndEllipse);
    Ipvm::Rect32s GetRectFromFloatData(Ipvm::Point32r2 fptCenter, float fHalfWidth, float fHalfHeight);

    CString GetStringCount(long n);
};
