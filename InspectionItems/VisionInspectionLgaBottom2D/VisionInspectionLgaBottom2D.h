#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../InformationModule/dPI_DataBase/Land.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineSeg32r.h> //kircheis_LandShape

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionLgaBottom2DPara;
class CDlgVisionInspectionLgaBottom2D;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class LandInspectionResult;
class VisionImageLot;
class LandPara;
class CLandMap;
struct FPI_RECT;

#define CIRCLE_LAND_SWEEP_LINE_COUNT 16
#define EDGE_COUNT_MIN 5
#define EDGE_FILTERING 0.1f

//HDR_6_________________________________ Header body
//
class __VISION_INSP_LAND_INSPECTION_CLASS__ VisionInspectionLgaBottom2D : public VisionInspection
{
public:
    VisionInspectionLgaBottom2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionLgaBottom2D(void);

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
    BOOL DoInsp(const bool detailSetupMode);

    VisionInspectionLgaBottom2DPara* m_VisionPara;
    CDlgVisionInspectionLgaBottom2D* m_pVisionInspDlg;

    CPI_EdgeDetect* m_pEdgeDetect;
    CPI_Blob* m_pBlob;

    float m_fCalcTime;

public:
    BOOL ResetInspItem();

public:
    std::vector<CString> m_group_id_list;

    std::vector<std::vector<Package::Land>> m_vec2LandInfoPerLayer;

    VisionAlignResult* m_sEdgeAlignResult;
    Ipvm::Point32r2* m_pfptGetCenterPosbyPadAlign;

    //{{Inspection Function
    BOOL DoLandAlign(); //kircheis_LandMissing
    BOOL DoLandOffsetX();
    BOOL DoLandOffsetX(const CString strLandWidthName, const long nGroupIndex = -1);
    BOOL DoLandOffsetY();
    BOOL DoLandOffsetY(const CString strLandWidthName, const long nGroupIndex = -1);
    BOOL DoLandOffsetR();
    BOOL DoLandOffsetR(const CString strLandWidthName, const long nGroupIndex = -1);
    BOOL DoLandWidth();
    BOOL DoLandWidth(const CString strLandWidthName, const long nGroupIndex = -1);
    BOOL DoLandLength();
    BOOL DoLandLength(const CString strLandWidthName, const long nGroupIndex = -1);
    //}}

    //Result
    LandInspectionResult* m_result;

    //{{Align Function
    BOOL GetSpecLandCenterByBody(float fAngle_rad, std::vector<std::vector<Ipvm::Point32r2>>& o_vec2ptLandCenter,
        std::vector<std::vector<Ipvm::Point32r2>>& o_vec2ptRotateLandCenter, Ipvm::Point32r2& o_fptGravityCenter,
        Ipvm::Point32r2& o_fptRotateGravityCenter);

    BOOL MakeLandROI(const std::vector<std::vector<Package::Land>>& i_vec2LandMapData,
        const std::vector<std::vector<Ipvm::Point32r2>>& i_vec2fptLand,
        std::vector<std::vector<PI_RECT>>& o_vec2srtLand, std::vector<std::vector<FPI_RECT>>& o_vec2sfrtLand,
        std::vector<std::vector<std::vector<PI_RECT>>>& o_vec3srtSearchLand,
        std::vector<std::vector<std::vector<FPI_RECT>>>& o_vec3sfrtSearchLand);
    //kk Rect / Circle 형태 분기 시작
    void MakeLandROI_Rect(const std::vector<Package::Land>& i_vecLandMapData,
        const std::vector<Ipvm::Point32r2>& i_vecfptLand, const LandPara& i_landpara,
        std::vector<PI_RECT>& o_vecsrtLand, std::vector<FPI_RECT>& o_vecsfrtLand,
        std::vector<std::vector<PI_RECT>>& o_vec2srtSearchLand,
        std::vector<std::vector<FPI_RECT>>& o_vec2sfrtSearchLand);
    void MakeLandROI_Circle(const std::vector<Package::Land>& i_vecLandMapData,
        const std::vector<Ipvm::Point32r2>& i_vecfptLand, const LandPara& i_landpara,
        std::vector<PI_RECT>& o_vecsrtLand, std::vector<FPI_RECT>& o_vecsfrtLand);

    void RotateLandROI(const std::vector<std::vector<Package::Land>>& i_vec2LandMapData,
        std::vector<std::vector<Ipvm::Point32r2>> vec2fptLand, std::vector<std::vector<PI_RECT>>& o_vec2srtLand,
        std::vector<std::vector<FPI_RECT>>& o_vec2sfrtLand);

    BOOL AlignLand(const bool detailSetupMode, const Ipvm::Image8u& image,
        std::vector<std::vector<Ipvm::Point32r2>> vec2fptSpecLand,
        std::vector<std::vector<std::vector<FPI_RECT>>> vec3sfrtSearchLand,
        std::vector<std::vector<FPI_RECT>>& o_vec2sfrtLand,
        std::vector<std::vector<Ipvm::Point32r2>>& o_vec2fptLandCenter, std::vector<std::vector<float>>& o_vec2fWidth,
        std::vector<std::vector<float>>& o_vec2fLength, std::vector<std::vector<BOOL>>& o_vec2bAlignResult,
        Ipvm::Point32r2& o_fptGravityCenter); //kircheis_LandMissing
    BOOL GetSearchLine(Ipvm::Point32r2 fptLandCenter, const FPI_RECT& sfrtSearch, long nDir,
        Ipvm::LineSeg32r& o_sLineStart, Ipvm::LineSeg32r& o_sLineEnd);
    BOOL RemoveNoisebyLine(std::vector<Ipvm::Point32r2> i_vecptEdge, std::vector<Ipvm::Point32r2>& o_vecptFitEdge,
        Ipvm::LineEq32r& o_line, float fTolerence);
    BOOL FindLineEdge(const Ipvm::Image8u& image, Ipvm::LineSeg32r sLineStart, Ipvm::LineSeg32r sLineEnd,
        BOOL bRisingEdge, BOOL bFirstEdge, std::vector<Ipvm::Point32r2>& o_vecfptEdge);
    BOOL AlignLandForCircleShape(const Ipvm::Image8u& image, Ipvm::Point32r2 i_fptSpecLandCenter, float i_fSpecWidthPXL,
        long i_nLayer, std::vector<Ipvm::Point32r2>& o_vecptFitEdge,
        Ipvm::CircleEq32r& o_AlignResult); //kircheis_LandShape
    std::vector<float> m_vecfCosForBallCenter; //kircheis_LandShape
    std::vector<float> m_vecfSinForBallCenter; //kircheis_LandShape
    void UpdateSweepAngleTable(
        long nSectionNum, std::vector<float>& vecfCos, std::vector<float>& vecfSin); //kircheis_LandShape

    bool RoughAlignLand(const Ipvm::Image8u& image, Ipvm::Image8u& imageBin,
        std::vector<std::vector<FPI_RECT>> vec2sfrtSpecLand, std::vector<std::vector<Ipvm::Rect32s>>& o_vec2rtObj,
        std::vector<std::vector<Ipvm::Point32r2>>& o_vec2fptObjCenter,
        std::vector<std::vector<Ipvm::Point32r2>>& o_vec2fptObjMassCenter, Ipvm::Point32r2& o_fptBlobGravityCenter,
        Ipvm::Point32r2& o_fptBlobMassGravityCenter, std::vector<Ipvm::Image8u>& vecMask);

    BOOL ModifyLandPoints(Ipvm::Point32r2 fptSpecGravityCenterbyOrigin,
        std::vector<std::vector<Ipvm::Point32r2>> vec2fptLandCenter,
        std::vector<std::vector<Ipvm::Point32r2>>& vec2fptLandbyOrigin,
        std::vector<std::vector<Ipvm::Point32r2>>& vec2fptSpecLandbyOrigin);

    BOOL GetBallCenterByLand();
    double m_affineTransformSpecToReal[2][3];
    double m_affineTransformRealToSpec[2][3];

    std::vector<Ipvm::LineSeg32r> m_rough_vecSweepLine; //for DebugInfo//kircheis_LandShape
    std::vector<Ipvm::LineSeg32r> m_vecSweepLine; //for DebugInfo//kircheis_LandShape
    //}}

    //kk GroupSpec
    void UpdateSpec();
    void GetVisionInspectionSpecs();
    void SetVisionInspectionSpecs();

    BOOL FillArbitraryRect(const PI_RECT& srtFillRect, BYTE byVal, Ipvm::Image8u& io_image);
    void getGroupInfo(std::vector<Package::Land> i_vecLandMap, std::vector<CString>& o_group_id_list);

    void SetDefaultLandROI(const Ipvm::Point32r2 i_SpecCenter, const float fLandHalfWidth, const float fLandHalfLength,
        const float fCurLayerAngle, Ipvm::Point32r2& o_fptLB, Ipvm::Point32r2& o_fptRB, Ipvm::Point32r2& o_fptLT,
        Ipvm::Point32r2& o_fptRT); //kircheis_LandMissing
};
