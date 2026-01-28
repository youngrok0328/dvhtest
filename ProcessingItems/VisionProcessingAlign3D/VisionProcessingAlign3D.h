#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "VisionProcessingAlign3DPara.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/BodyInfo.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
namespace VisionEdgeAlign
{
class Para;
class Inspection;
} // namespace VisionEdgeAlign

class CPackageSpec;
class CDlgVisionProcessingAlign3D;
class Result;
class VisionImageLot;
struct S3DImageData;
struct FPI_RECT;
struct PI_RECT;
struct S3DMergeResult;

//HDR_6_________________________________ Header body
//
class __VISION_STITCHING_3D_ALIGN_CLASS VisionProcessingAlign3D : public VisionProcessing
{
public:
    VisionProcessingAlign3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingAlign3D(void);

public: // VisionProcessing virtual functions
    virtual BOOL DoInspection(const bool detailSetupMode,
        const enSideVisionModule i_ModuleStatus = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual BOOL OnInspection() override;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override;
    virtual void ResetSpecAndPara() override;
    virtual void ResetResult() override;
    virtual void GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode) override;
    virtual void AppendTextResult(CString& textResult);
    virtual long ShowDlg(const ProcessingDlgInfo& procDlgInfo) override;
    virtual void CloseDlg() override;
    virtual std::vector<CString> ExportRecipeToText() override; //kircheis_TxtRecipe

public:
    VisionEdgeAlign::Para* m_edgeAlign_para;
    VisionEdgeAlign::Inspection* m_edgeAlign_inspection;

    float m_fCalcTime;
    CVisionProcessingAlign3DPara m_VisionPara;
    CDlgVisionProcessingAlign3D* m_pVisionInspDlg;

    S3DMergeResult* m_3DMergeResult;

    BOOL SplitPackageAlign(const Ipvm::Image8u& image, long stitchCount, FPI_RECT& o_sfrtRawDevice,
        std::vector<Ipvm::Rect32s>& o_vecrtOverlap);

    BOOL CalcMatchOverlapROI(S3DImageData& split3DImage, Ipvm::Rect32s rtTopOverlap, Ipvm::Rect32s rtBottomOverlap,
        std::vector<Ipvm::Rect32s>& vecrtTempl, std::vector<Ipvm::Rect32s>& vecrtSearch,
        std::vector<Ipvm::Rect32s>& o_vecrtMatchOverlap, std::vector<Ipvm::LineSeg32r>& o_vecsMatchingLine);

    BOOL MakeSplitPackage(const FPI_RECT& sfrtRawDevice, long nGrabCount,
        const std::vector<Ipvm::LineSeg32r>& vecsTotalMatchingLine, std::vector<FPI_RECT>& o_vecsfrtSplit,
        std::vector<FPI_RECT>& o_vecsfrtSplitPackage);

    BOOL CopyFirstSplitImage(S3DImageData& split3DImage, S3DImageData& ref3DImage, Ipvm::Rect32s rtFirstSplit);
    BOOL CopySplitImage(S3DImageData& split3DImage, S3DImageData& ref3DImage, Ipvm::Rect32s rtSrc, Ipvm::Rect32s rtDst);

    BOOL StitchingImage(S3DImageData& split3DImage, const Ipvm::LineSeg32r& sTopMatchLineSeg,
        const Ipvm::LineSeg32r& sBottomMatchLineSeg, Ipvm::Rect32s rtRotate, const Ipvm::Point64r3& s3DTheta,
        const Ipvm::Point64r3& s3DTopOrigin_um, const Ipvm::Point64r3& s3DBottomOrigin_um, S3DImageData& ref3DImage,
        std::vector<Ipvm::Point64r3>& o_vecConfirmBottom3Ddata);

    BOOL Stitching2DImage(S3DImageData& split3DImage, const Ipvm::LineSeg32r& sTopMatchLineSeg,
        const Ipvm::LineSeg32r& sBottomMatchLineSeg, Ipvm::Rect32s rtCalc, S3DImageData& ref3DImage);

    double GetDeltaTheta(const Ipvm::Point64r2& v1, const Ipvm::Point64r2& v2);

    BOOL Rotate3DImage(const Ipvm::Image32r& i_image, const Ipvm::Rect32s& rtROI, const Ipvm::Point64r3& sOrigin_um,
        double xTheta, double yTheta, double zTheta, float fOffsetZ, Ipvm::Image32r& o_image);

    Ipvm::Point64r3 Rotate3D(const Ipvm::Point64r3& i_Data, const Ipvm::Point64r3& i_Origin, const double& cosThetaX,
        const double& cosThetaY, const double& cosThetaZ, const double& sinThetaX, const double& sinThetaY,
        const double& sinThetaZ);

    BOOL MakeSpecBallROI(FPI_RECT sfrtPackageBody, Ipvm::Rect32s rtMatchOverlap,
        std::vector<Ipvm::Point32r2>& vecfptBallSpec, std::vector<Ipvm::Rect32s>& vecrtBallSpec,
        std::vector<Ipvm::Rect32s>& vecrtBallSearch, std::vector<long>& vecnBallID);
    BOOL MakeSpecBallROI(const std::vector<long>& i_vecnRefBallID, FPI_RECT sfrtPackageBody,
        Ipvm::Rect32s rtMatchOverlap, std::vector<Ipvm::Point32r2>& vecfptBallSpec,
        std::vector<Ipvm::Rect32s>& vecrtBallSpec, std::vector<Ipvm::Rect32s>& vecrtBallSearch);

    void SaveResulteData(long nPane, CString strResult);
    void SaveResulteData(long nPane, const Ipvm::Point64r3& s3DTheta, float fOffsetZ);
    void SaveResulteData(long nPane, const std::vector<Ipvm::Point64r3>& vecs3DTheta,
        const Ipvm::Point64r3& s3DAverageTheta, float fOffsetZ);

    BOOL FindBall_Blob(S3DImageData s3DImageInfo, Ipvm::Rect32s rtROI, std::vector<Ipvm::Rect32s>& vecrtObjSearch,
        std::vector<Ipvm::Rect32s>& o_vecrtObj, std::vector<Ipvm::Point32r2>& o_vecfptObjCenter);
    BOOL Get3DPoints_byBall(const S3DImageData& split3DImage, const std::vector<Ipvm::Point32r2>& vecfptObjCenter,
        std::vector<Ipvm::Rect32s>& o_vecrtMeasure, std::vector<Ipvm::Rect32s>& o_vecrtMeasured,
        std::map<long, BOOL>& o_validObjectIndex, std::vector<Ipvm::Point64r3>& o_vec3Ddata_um);

    BOOL GetPlaneData_RemoveNoise(std::map<long, BOOL>& io_validObjectIndexTop,
        std::map<long, BOOL>& io_validObjectIndexBtm, std::vector<Ipvm::Point64r3>& io_top3DData,
        std::vector<Ipvm::Point64r3>& io_btm3DData, Ipvm::Point64r3& o_averageTop, Ipvm::Point64r3& o_averageBtm,
        Ipvm::Point64r3& o_planeABC);

    BOOL GetLandPlaneData(const S3DImageData& split3DImage, FPI_RECT sfrtPackage, Ipvm::Rect32s rtMatchingOverlap,
        const std::vector<Ipvm::Point32r2>& i_vec2fptObjectSpec, const std::vector<Ipvm::Rect32s>& i_vec2rtObjSearch,
        std::vector<Ipvm::Point64r3>& o_vec3DObj_um, Ipvm::Point64r3& o_s3DAverage_um, Ipvm::PlaneEq64r& o_sPlaneEq_um);
    BOOL MakeSpecLand_ByPKGSize(FPI_RECT sfrtPackageBody, Ipvm::Rect32s rtMatchOverlap, float fObjSearchX_mm,
        float fObjSearchY_mm, std::vector<Ipvm::Point32r2>& o_vec2fptObjectSpec,
        std::vector<Ipvm::Rect32s>& o_vec2rtObjSearch, std::vector<long>& o_vec2rtObjID);
    BOOL FindLand_Blob(S3DImageData s3DImageInfo, Ipvm::Rect32s rtROI, Ipvm::Image8u& o_binaryImage,
        const std::vector<Ipvm::Rect32s>& vec2rtObjSearch, std::vector<Ipvm::Rect32s>& o_vec2rtObj);
    BOOL FillArbitraryRect(const PI_RECT& srtFillRect, BYTE byVal, Ipvm::Image8u& io_image);
    BOOL CalcZPosbyMask(const Ipvm::Image32r& zmap, const Ipvm::Rect32s& rtArea, float& o_fZPos,
        Ipvm::Point32r2& o_fptCenter, const Ipvm::Image8u& mask, const Ipvm::Image16u* pwInten = nullptr);
    float CalcSrZPosbyMask(const Ipvm::Image32r& zmap, const Ipvm::Rect32s& roi, const Ipvm::Image8u& maskImage);

    //k 18.02.22 ball이 없을 시 stitch 하는 알고리즘 이식
    BOOL MakeFixMeasureROI(
        Ipvm::Rect32s rtMatchOverlap, std::vector<Ipvm::Point32r2>& o_vecfptObjCenter, long nNumX, long nNumY);
    //k 18.02.28 DebugInfo 추가
    void SetDebugInfo(const bool detailSetupMode);
    //k
    BOOL GetPlaneData_byTop(const S3DImageData& split3DImage, const std::vector<Ipvm::Point32r2>& vecfptObjCenter,
        const std::vector<long>& vecnObjectLayer, std::vector<Ipvm::Rect32s>& o_vecrtMeasure,
        std::vector<Ipvm::Rect32s>& o_vecrtMeasured, std::vector<Ipvm::Point64r3>& o_vec3Ddata,
        Ipvm::PlaneEq64r& o_sPlaneEq, Ipvm::Point64r3& o_s3DAverage);

private:
    Result* m_result;
    CString m_errorLog;
    bool DoAlign(const bool detailSetupMode, Ipvm::Image8u& o_imageHMap, Ipvm::Image8u& o_imageIntensity,
        Ipvm::Image32r& o_imageZMap, Ipvm::Image16u& o_imageVMap);

    BOOL MakeRoughOverlap(std::vector<Ipvm::Rect32s> i_vecrtOverlap, FPI_RECT i_sfrtRawDevice,
        std::vector<Ipvm::LineSeg32r>& o_vecRoughMatchLine);
    BOOL MakeMatchingOverlap(S3DImageData& io_split3DImage, const std::vector<Ipvm::Rect32s> i_vecrtOverlap,
        std::vector<Ipvm::Rect32s>& o_vecrtTotalMatchOverlap, std::vector<Ipvm::LineSeg32r>& o_vecsTotalMatchingLine);

    BOOL SplitAreaCalc(const std::vector<Ipvm::Rect32s>& i_vecrtTotalMatchOverlap,
        const std::vector<Ipvm::LineSeg32r>& i_vecsTotalMatchingLine, const FPI_RECT& i_sfrtRawDevice,
        const long i_nGrabCount, std::vector<FPI_RECT>& o_vecsfrtSplitPackage, FPI_RECT& o_sfrtPackage,
        std::vector<Ipvm::Rect32s>& o_vecrtCalc);

    BOOL MakeTopMatchingObject(const S3DImageData& i_split3DImage, const S3DImageData& i_ref3DImage,
        const Ipvm::Rect32s i_rtTopMatchingOverlap, const Ipvm::Rect32s i_rtBottomMatchingOverlap, const long i_nMerge,
        Ipvm::Point64r3& o_s3DTopAverage_um, Ipvm::Point64r3& o_s3DBottomAverage_um, Ipvm::Point64r3& o_s3DTheta,
        CString& o_strResult);

    bool SearchMatchingObject_Ball(const S3DImageData& i_split3DImage, const S3DImageData& i_ref3DImage,
        const FPI_RECT i_sfrtRawDevice, const FPI_RECT i_sfrtBottomPackage, const FPI_RECT i_sfrtTopPackage,
        const Ipvm::Rect32s i_rtTopMatchingOverlap, const Ipvm::Rect32s i_rtBottomMatchingOverlap, const long i_nMerge,
        Ipvm::Point64r3& o_s3DTopAverage_um, Ipvm::Point64r3& o_s3DBottomAverage_um, Ipvm::Point64r3& o_s3DTheta,
        CString& o_strResult, bool& o_selectedSearchObject);

    bool SearchMatchingObject_Land(const S3DImageData& i_split3DImage, const S3DImageData& i_ref3DImage,
        const FPI_RECT i_sfrtBottomPackage, const FPI_RECT i_sfrtTopPackage, const Ipvm::Rect32s i_rtTopMatchingOverlap,
        const Ipvm::Rect32s i_rtBottomMatchingOverlap, Ipvm::Point64r3& o_s3DTopAverage_um,
        Ipvm::Point64r3& o_s3DBottomAverage_um, Ipvm::Point64r3& o_s3DTheta, bool& o_selectedSearchObject);

    bool getNormalizePoints(const std::vector<Ipvm::Point64r3>& i_points, std::vector<Ipvm::Point64r3>& o_points,
        Ipvm::Point64r3& o_center, Ipvm::Point64r3& o_halfRange);
    bool calcAutoPaneOffsetX(const eRoughAlignUsingDefine i_eRoughAlignUsingDefine, double& o_paneOffsetX_um);
    bool calcAutoPaneOffsetY(const eRoughAlignUsingDefine i_eRoughAlignUsingDefine, double& o_paneOffsetY_um);
    bool FindIdFromProfileForRoughAlign(
        const std::vector<double>& i_vecProfile, long& o_positiveSlopeEndID, long& o_negativeSlopeStartID);
};
