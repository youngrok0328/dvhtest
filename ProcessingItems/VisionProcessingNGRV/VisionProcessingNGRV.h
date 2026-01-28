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
class CDlgVisionProcessingNGRV;
class VisionImageLot;
class VisionProcessingNGRVPara;

//HDR_6_________________________________ Header body
enum VisionModuleID
{
    VISION_MODULE_ID_2D_TOP = 0,
    VISION_MODULE_ID_2D_BTM,
};

#define MAX_2D_VISION_TYPE 2
//
class __VISION_PROCESSING_NGRV_CLASS VisionProcessingNGRV : public VisionProcessing
{
public:
    VisionProcessingNGRV(
        LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingNGRV(void);

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
    VisionProcessingNGRVPara* m_VisionParaBTM;
    VisionProcessingNGRVPara* m_VisionParaTOP;
    CDlgVisionProcessingNGRV* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    BOOL DoInsp(const bool detailSetupMode, Ipvm::Image8u& i_image);

    void setBlobRois(const Ipvm::BlobInfo* blobInfos, long nblobCount);
    void getMaxBlobSizes(const Ipvm::BlobInfo* blobInfos, long nblobCount);

    long m_nBlobResult;
    long m_nBlobCount;
    long m_nMaxBlobSize;

    std::vector<Ipvm::Rect32s> m_vecrtBlobROI;

public:
    bool CheckmakedROIinimage_ChnageStitch(const Ipvm::Rect32s& srcROI, const long& ImageSizeX, const long& ImageSizeY,
        const long& ROI_Width, const long& ROI_Height, const long& StitchCountX, const long& StitchCountY);

    void VerifyROIusingimageSize(
        const long& imageSizeX, const long& imageSizeY, const Ipvm::Rect32s& SrcROI, Ipvm::Rect32s& dstROI);

    void SetNGRV_ref_image_size(const long& vision_type, const long& imageSizeX, const long& imageSizeY);
    long getNGRV_ref_image_size_X(const long& vision_type);
    long getNGRV_ref_image_size_Y(const long& vision_type);

private: // Btm, Top 2개의 Vision
    long ref_image_sizeX[MAX_2D_VISION_TYPE];
    long ref_image_sizeY[MAX_2D_VISION_TYPE];
};
