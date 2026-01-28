#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "SurfaceItemResult.h"

//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CDlgVisionInspectionSurface;
class VisionInspectionSurfacePara;
class CSurfaceAlgoPara;
class CSurfaceCustomROI;
class CSurfaceItem;
class CPackageSpec;
class SystemConfig;
class ScanAreaImageInfo;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class BlobAttribute;
class VisionDebugInfo;
class SurfacePrepairResult;

//HDR_6_________________________________ Header body
//
class __DPI_SURFACE_API__ VisionInspectionSurface : public VisionInspection
{
public:
    VisionInspectionSurface(
        LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionSurface(void);

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
    BOOL DoTeach();

    enum class enumRunMode
    {
        TeachMask,
        CriteriaTest,
        Normal,
    };

    BOOL DoInspection(const bool detailSetupMode, enumRunMode mode, Ipvm::Image32s* o_imageLabel);

    CDlgVisionInspectionSurface* m_pVisionInspDlg;

    bool GetDisplayAlignImage(CSurfaceItem& sItem, bool applyCombine, Ipvm::Image8u& o_grayImage);
    bool MakePrepairImage(CSurfaceItem& sItem, SurfacePrepairResult& o_result);
    bool MakeInspImage(
        CSurfaceItem& sItem, const SurfacePrepairResult& i_prepairResult, Ipvm::Image8u& o_thresholdImage);
    long Surface_CreateMaskGroup(BOOL bInsp = FALSE);

    VisionInspectionSurfacePara* m_surfacePara;
    CSurfaceItem* GetSurfaceItem();
    CSurfaceAlgoPara* GetSurfaceAlgoPara();
    CSurfaceCustomROI* GetSurfaceCustomROI();
    const CSurfaceItemResult& GetResult() const;

    void SetSurfaceItem(const CSurfaceItem& value);
    void SetSurfaceAlgoPara(const CSurfaceAlgoPara& value);

    bool GetInspSize(const bool& i_bUnitIsPixel, float& o_fBodySizeX, float& o_fBodySizeY);

    CPI_EdgeDetect* m_pEdgeDetect;

    float m_fCalcTime;

    CPI_Blob* m_pBlob;

    int32_t m_nBlobNum;
    Ipvm::BlobInfo* m_psObjInfo_;
    BlobAttribute* m_blobAttributes;

    Ipvm::Image32s* m_plMaskLabel_;
    Ipvm::BlobInfo* m_psMaskInfo_;

private:
    Ipvm::Image32s* m_plLabel_;
    BOOL m_bEdgeAlignValid;
    CSurfaceItemResult m_result;
    VisionAlignResult* m_EdgeAlignResult;

    bool DoInsp(const bool detailSetupMode, Ipvm::BlobInfo* psDarkObjInfo, Ipvm::BlobInfo* psBrightObjInfo,
        short nDarkBlobMaxNum, short nBrightBlobMaxNum, BlobAttribute* blobAttributes, BOOL i_bTeachMask,
        Ipvm::Image32s* o_imageLabel);

    bool DoInsp_Core(const bool detailSetupMode, Ipvm::BlobInfo* psDarkObjInfo, Ipvm::BlobInfo* psBrightObjInfo,
        short nDarkBlobMaxNum, short nBrightBlobMaxNum, Ipvm::Image8u& o_thresholdImage, Ipvm::Image32s* o_imageLabel);

    float GetConvertUnitFactor(long nIdx);
    void SetSurfaceCriteriaData(const Ipvm::Point32s2& realBodyCenterOffset,
        VisionInspectionResult* i_pInspectionResult, const CSurfaceItem& i_cSurfaceItem,
        const CSurfaceItemResult& i_cSurfaceItemResult); //mc_Surface Criteria에 검출된 Data 전부를 쓴다
    void SetCustomROI_SurfaceBitmapMask();
    void MakeSurfaceROI(CSurfaceItem& ItemSpec);

    bool MakePrepairImage2D(CSurfaceItem& sItem, SurfacePrepairResult& o_result);
    bool MakePrepairImage3D(CSurfaceItem& sItem, SurfacePrepairResult& o_result);

    bool MakeInspImage2D(
        CSurfaceItem& sItem, const SurfacePrepairResult& i_prepairResult, Ipvm::Image8u& o_thresholdImage);
    bool MakeInspImage3D(
        CSurfaceItem& sItem, const SurfacePrepairResult& i_prepairResult, Ipvm::Image8u& o_thresholdImage);

    void InitEdgeResult();
    bool GenerateMask(const bool detailSetupMode, CSurfaceItem& ItemSpec, Ipvm::Image8u& imageMask,
        Ipvm::Image8u& image, CSurfaceItem* pSurfaceItem);
    long GenerateMask_UseCustomROI(const CSurfaceAlgoPara& AlgoPara, LPCTSTR szROIName, int nMaskOrder, BOOL bValidate,
        Ipvm::Image8u& imageMask, Ipvm::Image8u& imageBuffer);
    long MakeDetectRect(const Ipvm::Image8u& image, const long lThreshX, const long lThreshY, Ipvm::Rect32s i_rtROI,
        Ipvm::Rect32s& o_rtDetectRect);
    long GenerateMask_SetMask(int nType, const Ipvm::Point32s2* pPointArray, long nOffsetX, long nOffsetY,
        const Ipvm::Point32r2* pOffsetArray_um, BYTE bMaskValue, BYTE bBackValue, Ipvm::Image8u& imageBuffer,
        Ipvm::Rect32s& rtMaskArea);
    long MakeObjectIgnorePattern(
        const CSurfaceItem& SurfaceItem, const Ipvm::Image8u& thresholdImage, const Ipvm::Image32s& imageLabel);
    long Segmentation(const Ipvm::Image8u& thresholdImage, Ipvm::Image32s& imageLabel_Dark,
        Ipvm::BlobInfo* psDarkObjInfo, int32_t nDarkBlobMaxNum, Ipvm::Image32s& imageLabel_Bright,
        Ipvm::BlobInfo* psBrightObjInfo, int32_t nBrightBlobMaxNum, Ipvm::Image32s* o_imageLabel);
    long CalcCharacteristics2D(const bool detailSetupMode, const Ipvm::Image8u& grayImage,
        const Ipvm::Image8u* aux1GrayImage, const Ipvm::Image8u* aux2GrayImage, const Ipvm::Image8u& maskImage,
        const Ipvm::Image8u& imageBackground, const CSurfaceItem& SurfaceItem);
    long CalcCharacteristics3D(const bool detailSetupMode, const Ipvm::Image8u& thresholdImage,
        const Ipvm::Image8u& grayImage, const Ipvm::Image8u& maskImage, const Ipvm::Image32r& heightMap,
        const CSurfaceItem& SurfaceItem);
    bool CalcCharacteristicsCommon(const bool detailSetupMode, const Ipvm::Image8u& grayImage,
        const Ipvm::Image8u& maskImage, const CSurfaceItem& SurfaceItem);

    long Calc2D_LocalContrast(const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel, const long nBlobNum,
        const Ipvm::Image8u& maskImage, const Ipvm::Image8u* grayImage, const Ipvm::Image8u* aux1Image,
        const Ipvm::Image8u* aux2Image, BlobAttribute* attributes);
    long Calc2D_Deviation(const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel, const long nBlobNum,
        const Ipvm::Image8u& image, BlobAttribute* attributes);
    long Calc2D_AuxDeviation(const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel, const long nBlobNum,
        const Ipvm::Image8u* aux1Image, const Ipvm::Image8u* aux2Image, BlobAttribute* attributes);

    bool Calc3D_AvgDelaHeight(const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel, const long nBlobNum,
        const Ipvm::Image8u& threshold, const Ipvm::Image32r& heightMap, BlobAttribute* attributes);
    bool Calc3D_KeyDelaHeight(const CSurfaceItem& sItem, const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel,
        const long nBlobNum, const Ipvm::Image8u& threshold, const Ipvm::Image32r& heightMap,
        BlobAttribute* attributes);
    bool GetCharacteristicResults(const bool detailSetupMode, const CSurfaceAlgoPara& AlgoPara);

    bool MakeGray_ThresholdedImage(const CSurfaceItem& sItem, const Ipvm::Image8u& i_image,
        const Ipvm::Image8u& i_imageBackground, const Ipvm::Image8u& i_maskImage, Ipvm::Image8u& o_imageDst);
    bool MakeGray_BackgroundImage(const CSurfaceItem& sItem, const Ipvm::Image8u& i_image,
        const Ipvm::Image8u& imageMask, Ipvm::Image8u& o_imageBackground);
    bool MakeZMap_BackgroundImage(const CSurfaceItem& sItem, const Ipvm::Image32r& i_zmap,
        const Ipvm::Image8u& imageMask, const Ipvm::Rect32s& roi, Ipvm::Image32r& o_imageBackground);
    bool MakeZMap_to_HeightZmap(
        const Ipvm::Image32r& i_zmap, const Ipvm::Image32r& i_backgroundZMap, Ipvm::Image32r& o_hightMap);
    bool MakeHeightMap_to_ThresholdImage(const CSurfaceItem& sItem, const Ipvm::Image32r& i_hightMap,
        const Ipvm::Image8u& i_maskImage, Ipvm::Image8u& o_thresholdImage);
    bool MakeHeightMap_to_GrayImage(
        const Ipvm::Image32r& i_hightMap, const Ipvm::Rect32s& roi, Ipvm::Image8u& o_grayImage);

    const VisionDebugInfo* FindVisionDebugInfo(const std::vector<VisionDebugInfo*>& i_infos, LPCTSTR name);
    void SetPass_TotalResult();

    bool GetSurfaceLayerImage(LPCTSTR name, Ipvm::Image8u& io_image);

    bool AnalisysBlobInfo(BOOL bUseBlobCount, BOOL bUseGVCheker, BOOL bDetailSetupMode); //kircheis_MED5_7_9
};
