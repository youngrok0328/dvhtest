#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Constants.h"
#include "StitchPara.h"
#include "StitchResult.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/CustomPolygon.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h> //kircheis_NGRVAF

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace VisionEdgeAlign
{
class Para;
class Inspection;
} // namespace VisionEdgeAlign

class CustomFixed;
class CustomPolygonMap;
class CPackageSpec;
class DlgVisionProcessingAlign2D;
class EngineBasedOnMatching;
class EngineBasedOnBodySize;
class ImageProcPara;
class VisionImageLot;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class __VISION_STITCHING_ALIGN_CLASS VisionProcessingAlign2D : public VisionProcessing
{
public:
    VisionProcessingAlign2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingAlign2D(void);

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
    VisionEdgeAlign::Para* m_edgeAlign_para;
    VisionEdgeAlign::Inspection* m_edgeAlign_inspection;

    StitchPara m_VisionPara;
    StitchResult m_result;
    DlgVisionProcessingAlign2D* m_pVisionInspDlg;

    // Align Result...

    float m_fCalcTime;

    BOOL m_bImageSaveMode;

    float GetMoveDistanceXbetweenFOVs_mm();
    float GetMoveDistanceYbetweenFOVs_mm();

    bool Do2DStitching_GetStitchInfo(const bool detailSetupMode);

    // 작업중
    BOOL Do2DStitching_EmptyCheckFrame(const bool detailSetupMode);
    BOOL Do2DStitching_AlignFrame(const bool detailSetupMode, const Ipvm::Rect32s& alignImageRoi);
    BOOL Do2DAlignedStitchAll(const Ipvm::Rect32s& alignImageRoi, const bool detailSetupMode);
    BOOL Do2DAlignAllImages_NoStitch(const Ipvm::Rect32s& alignImageRoi, const bool detailSetupMode);

    bool MakeDefaultTemplateROI_BaseOnMatching();

    bool GetConstants();
    bool SetROIsInRaw();

    bool FindAllStitchInfo_InRaw(const bool detailSetupMode);

    //////////////////////////////////////////////////

    //std::vector<std::pair<Ipvm::Rect32s, float>> m_vecfMatchRate;

    Constants m_constants;

    BOOL GetNGRVAFInfo(BOOL& o_bIsValidPlaneRefInfo, std::vector<Ipvm::Point32s2>& o_vecptRefPos_UM,
        long& o_nAFImgSizeX, long& o_nAFImgSizeY, std::vector<BYTE>& o_vecbyAFImage); //kircheis_NGRVAF

    bool CheckGrabFailure(); //Grab Success는 true, Fail은 false 리턴

private:
    EngineBasedOnMatching* m_basedOnMatching;
    EngineBasedOnBodySize* m_basedOnBodySize;

    Ipvm::Point32s2 m_StichedOffset; //  Stitching 완료된 영상을 이미지의 중심에 놓기 위한 Offset

    //bool doStitch(long x, long y);

    bool doStitch_OneFrame(long x, long y, const Ipvm::Image8u& srcImageRaw, const Ipvm::Point32r2& fptCenter,
        const float& fRotateAngle, const Ipvm::Point32r2& fptShift_FOV, const Ipvm::Point32r2& fptShift_Total,
        Ipvm::Image8u& dstImageStitch);

    bool GetCopyRegion(const long x, const long y, const Ipvm::Rect32s& verPreROIOverlap,
        const Ipvm::Rect32s& horPreROIOverlap, Ipvm::Point32s2& ptSrcCopyStart, Ipvm::Point32s2& ptDstCopyStart,
        Ipvm::Size32s2& roiCopySize);

    bool InterpolationStitchX(const long x, const long y, const Ipvm::Rect32s& verPreROIOverlap,
        const Ipvm::Rect32s& horPreROIOverlap, Ipvm::Image8u& srcImageSmall, Ipvm::Image8u& dstImageTotal);
    bool InterpolationStitchY(const long x, const long y, const Ipvm::Rect32s& verPreROIOverlap,
        const Ipvm::Rect32s& horPreROIOverlap, Ipvm::Image8u& srcImageSmall, Ipvm::Image8u& dstImageTotal);
    bool InterpolationStitchXY(const Ipvm::Rect32s& verPreROIOverlap, const Ipvm::Rect32s& horPreROIOverlap,
        Ipvm::Image8u& srcImageSmall, Ipvm::Image8u& dstImageTotal);

    bool RotateShiftFOVImage(const long x, const long y, const long nFrameID, const Ipvm::Point32r2& ptCenter,
        const float fAngle, const Ipvm::Point32r2& pfShift, Ipvm::Image8u& dstImage);

    bool RotateShiftFOVImage(const long x, const long y, const Ipvm::Image8u& srcImageRaw,
        const Ipvm::Point32r2& ptCenter, const float fAngle, const Ipvm::Point32r2& pfShift, Ipvm::Image8u& dstImage);

    Ipvm::Point32r2 GetShift_FOVRotateCenter(const Ipvm::Point32r2& fptRotateCenter,
        const Ipvm::Point32r2& fptAlignedCenter, const float fCos, const float fSin);

    bool UpdatePreOverlapROI(const long x, const long y, const Ipvm::Rect32s& verPreROIOverlap,
        const Ipvm::Rect32s& horPreROIOverlap, const Ipvm::Point32r2& fptShift_Total, Ipvm::Rect32s& shiftedVerPreROI,
        Ipvm::Rect32s& shiftedHorPreROI);

private:
    void SetCustomLayerMask(const Ipvm::Point32r2 i_fptAlignCenter); //mc_CustomLayer Mask
    std::vector<CustomFixed> GetCustomFixedMapInfo_px(const Ipvm::Point32r2 i_fptAlignCenter);
    std::vector<Package::CustomPolygon> GetCustomPolygonMapInfo_px(const Ipvm::Point32r2 i_fptAlignCenter);
    Ipvm::Polygon32r GetPolygon(const std::vector<Ipvm::Point32r2> i_vecfptPolygonPos);
};
