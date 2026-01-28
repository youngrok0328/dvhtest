#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CDlgVisionProcessingCoupon2D;
class VisionImageLot;
class VisionProcessingCoupon2DPara;

//HDR_6_________________________________ Header body
//
class __VISION_PROCESSING_COUPON2D_CLASS VisionProcessingCoupon2D : public VisionProcessing
{
public:
    VisionProcessingCoupon2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingCoupon2D(void);

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
    VisionProcessingCoupon2DPara* m_VisionPara;
    CDlgVisionProcessingCoupon2D* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    BOOL DoInsp(const bool detailSetupMode, Ipvm::Image8u& i_image);

    void setBlobRois(const Ipvm::BlobInfo* blobInfos, long nblobCount);
    void getMaxBlobSizes(const Ipvm::BlobInfo* blobInfos, long nblobCount);

    long m_nBlobResult;
    long m_nBlobCount;
    long m_nMaxBlobSize;

    std::vector<Ipvm::Rect32s> m_vecrtBlobROI;
};
