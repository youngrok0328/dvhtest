#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../InformationModule/dPI_DataBase/Ball.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionBgaBottom2DPara;
class CDlgVisionInspectionBgaBottom2D;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class VisionInspectionSpec;
class VisionInspectionResult;
//class Ball;
class Result;

// 영훈 20140709_BallAlign_refactory : Ball Width에 관련된 Parameter를 다 없앤다.
#define SPEC_WIDTH_SWEEP_LINE_COUNT_FAST 32 //16//kircheis_BallFlux
#define SPEC_WIDTH_SWEEP_LINE_COUNT_DETAIL 60 //kircheis_BallFlux
#define SPEC_QUALITY_SWEEP_LINE_COUNT 25
#define SPEC_BALL_EDGE_FILTERING 0.1f
#define SPEC_BALL_EDGE_COUNT_MIN 4
#define SEPC_WIDTH_COMPENSATION_RANGE_MIN 0.25f
#define SEPC_WIDTH_COMPENSATION_RANGE_MAX 0.75f
#define SEPC_QUALITY_COMPENSATION_RANGE_MIN 0.25f
#define SEPC_QUALITY_COMPENSATION_RANGE_MAX 0.75f

#define EDGE_THRESH_INVERT 3.f //kircheis_InvertCoax
#define FAST_WIDTH_SWEEP_LINE_COUNT 16 //kircheis_BKShiny

//HDR_6_________________________________ Header body
//
enum BallWidthGettingMethod
{
    enumGetPittingValue, //kircheis_20160602
    enumGetMaxValue
};

class __VISION_INSP_BALL_INSPECTION_CLASS__ VisionInspectionBgaBottom2D : public VisionInspection
{
public:
    VisionInspectionBgaBottom2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionBgaBottom2D(void);

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
    BOOL DoTeach(const bool detailSetupMode, long i_nBallGroupID = 0);

    VisionInspectionBgaBottom2DPara* m_pVisionPara;
    CDlgVisionInspectionBgaBottom2D* m_pVisionInspDlg;

    Ipvm::Image8u m_obliqueImage;
    Ipvm::Image8u m_coaxialImage;

    CPI_EdgeDetect* m_pEdgeDetect;

    float m_fCalcTime;

public:
    BOOL GetBallContrast(const Ipvm::Image8u& image);
    float GetBallContrastCompensation();
    BOOL GetFindBallEdge(const Ipvm::Image8u& oblique, const Ipvm::Image8u& coax);
    BOOL GetBallWidthByEdgeDetect(const bool detailSetupMode, const Ipvm::Image8u& image);
    BOOL GetReSearchBallWidthEdgeDetectByOblique(Ipvm::CircleEq32r i_circleEq, float i_fSecBallWidth,
        float i_fReSearchRatioInner, float i_fReSearchRatioOuter, long i_nBallType,
        std::vector<Ipvm::Point32r2>& o_vecTotalEdgePoint, std::vector<Ipvm::Point32r2>& o_vecFinalEdgePoint,
        Ipvm::CircleEq32r& o_circleEq);
    float GetVecterAvr(std::vector<float> vecfData);
    BOOL GetBallRoughCenterByCoaxImageBallPeak(const bool detailSetupMode, const Ipvm::Image8u& coaxImage,
        const Ipvm::Point32r2 i_ptSpecBallCenter, const float i_fSearchRadius, Ipvm::Point32r2& o_ptRoughCenter,
        Ipvm::Image8u& o_imageBallPeakDebugThresh); //kircheis_LKF
    BOOL GetBallWidthByEdgeDetect(
        const bool detailSetupMode, const Ipvm::Image8u& i_ballImage, const Ipvm::Image8u& i_combineImage);
    void UpdateSweepAngleTable(long nSectionNum, std::vector<float>& vecfCos, std::vector<float>& vecfSin);
    float GetBallEllipticity(std::vector<Ipvm::Point32r2> i_vecfptEdgePoints, Ipvm::CircleEq32r i_circleEqBall,
        BOOL bUseInnerCirclePoint, float fSpecRadiusPxl); //kircheis_TMI

    BOOL GetImageProcessing_Normal(Ipvm::Image8u& combineImage, Ipvm::Image8u& auxFrame, Ipvm::Rect32s rtInspectionROI);
    BOOL GetImageProcessing_Qaulity(Ipvm::Image8u& combineImage, Ipvm::Rect32s rtInspectionROI);

    BOOL GetBallCenterByBody();
    BOOL GetBallCenterAndWidth(
        const bool detailSetupMode, const Ipvm::Image8u& combineImage, const Ipvm::Image8u& auxFrame);
    BOOL AlignBalls();

    BOOL GetQualityInfo(const CString strBallQualityName, const std::vector<Package::Ball> i_BallData,
        const Ipvm::Image8u& i_ballImage, long nQualityThresh);
    BOOL GetQualityInfoByRingThickness(const CString strBallQualityName, const std::vector<Package::Ball> i_BallData,
        const Ipvm::Image8u& i_ballImage, long nQualityThresh);
    BOOL DoQualityTeach(const Ipvm::Image8u& i_ballImage, long nQualityThresh, long i_nBallGroupID = 0);
    BOOL Qualtiy_BallAreaPixelValidCount(
        const Ipvm::Image8u& combine, long nQualityThresh, std::vector<long>& vecnOverPixelCount);

    void SetDebugInfo(const bool detailSetupMode);
    void PrepareInspection();
    bool SkipMissing(const Package::Ball& ball, VisionInspectionSpec* spec, VisionInspectionResult* result,
        VisionInspectionSpec* missingSpec, VisionInspectionResult* missingResult, CString strCompID, const long nindex,
        const float nominalValue = 0.f); //kircheis_MED2.5

public:
    BOOL DoBallMissing();
    BOOL DoBallOffsetX();
    BOOL DoBallOffsetX(const CString strBallContrastName, const std::vector<Package::Ball> BallData,
        BOOL bEachGroupInsp
        = FALSE); //kircheis_MED2.5 //내부에서 돌리면서 확인하면 안되는 게 SkipMissing()때문에 안된다
    BOOL DoBallOffsetY();
    BOOL DoBallOffsetY(const CString strBallContrastName, const std::vector<Package::Ball> BallData,
        BOOL bEachGroupInsp = FALSE); //kircheis_MED2.5
    BOOL DoBallOffsetR();
    BOOL DoBallOffsetR(const CString strBallContrastName, const std::vector<Package::Ball> BallData,
        BOOL bEachGroupInsp = FALSE); //kircheis_MED2.5
    BOOL DoBallGridOffsetXByKircheis();
    BOOL DoBallGridOffsetXByKircheis(const CString strBallContrastName, const std::vector<Package::Ball> BallData,
        BOOL bEachGroupInsp = FALSE); //kircheis_MED2.5
    BOOL DoBallGridOffsetYByKircheis();
    BOOL DoBallGridOffsetYByKircheis(const CString strBallContrastName, const std::vector<Package::Ball> BallData,
        BOOL bEachGroupInsp = FALSE); //kircheis_MED2.5
    BOOL DoBallWidth();
    BOOL DoBallWidth(const CString strBallContrastName, const std::vector<Package::Ball> BallData,
        BOOL bEachGroupInsp = FALSE); //kircheis_MED2.5
    BOOL DoBallQuality();
    BOOL DoBallQuality(const CString strBallContrastName, const std::vector<Package::Ball> BallData,
        BOOL bEachGroupInsp = FALSE); //kircheis_MED2.5
    BOOL DoBallContrast();
    BOOL DoBallContrast(const CString strBallContrastName, const std::vector<Package::Ball> BallData,
        BOOL bEachGroupInsp = FALSE); //kircheis_MED2.5
    BOOL DoBallEllipticity(const bool detailSetupMode); //kircheis_TMI

    //{{ //kircheis_BKShiny
    BOOL FindEdge(const Ipvm::Image8u& image, Ipvm::Point32r2 fptStart, Ipvm::Point32r2 fptEnd, BOOL bRisingEdge,
        BOOL bFirstEdge, Ipvm::Point32r2& o_fptEdge, float& o_fEdgeRadius);
    BOOL IntensityXPos(const Ipvm::Image8u& image, Ipvm::Rect32s rtRough, long nSearchLength, Ipvm::Rect32s& o_rtResult,
        float& o_fResult, BOOL bMaxPos);
    BOOL IntensityYPos(const Ipvm::Image8u& image, Ipvm::Rect32s rtRough, long nSearchLength, Ipvm::Rect32s& o_rtResult,
        float& o_fResult, BOOL bMaxPos);
    //}}

    VisionAlignResult* m_sBallAlignResult;
    VisionAlignResult* m_sEdgeAlignResult;
    Ipvm::Point32r2* m_pfptGetCenterPosbyPadAlign; //mc_PadAAlign의 결과

    double m_affineTransformSpecToReal[2][3];
    double m_affineTransformRealToSpec[2][3];

    float m_f2DScale;

    struct MissingBallInfo
    {
        MissingBallInfo(const long ballIndex, const Ipvm::Rect32s& ballSpecROI)
            : m_ballIndex(ballIndex)
            , m_ballSpecROI(ballSpecROI)
        {
        }

        long m_ballIndex;
        Ipvm::Rect32s m_ballSpecROI;
    };

    std::vector<MissingBallInfo> m_missingBalls;

    std::vector<long> m_nWidthSweepLineCount; //kircheis_BKShiny

    std::vector<float> m_vecfCosForQuality;
    std::vector<float> m_vecfSinForQuality;

    std::vector<std::vector<float>> m_vecfCosForBallCenter;
    std::vector<std::vector<float>> m_vecfSinForBallCenter;

    std::vector<Ipvm::EllipseEq32r> m_vecBallSpec_Body;
    std::vector<Ipvm::Rect32s> m_vecrtBallPosition_pixel;

    //{{ //kircheis_BKShiny
    std::vector<Ipvm::Rect32s> m_vecrtRoughBall;
    //}}

    // Debug Info에 필요한 변수들...
    std::vector<Ipvm::Rect32r> m_vecrtBallCircleROI;
    std::vector<std::vector<CPoint>> m_vec2ptBallCircleEdgePoint;
    std::vector<Ipvm::Point32r2> m_vecfptQualityCheckPairPoint;

    std::vector<Ipvm::Rect32s> m_vecrtDebugMissingBallROI;
    std::vector<float> m_vecDebugInfoValue;
    std::vector<CPoint> m_vecBlobBallPoint;
    std::vector<Ipvm::LineSeg32r> m_vecQualitySweepLine;

    //KK Group Ball Data
    std::vector<std::vector<Package::Ball>> m_vec2BallData;

private:
    Result* m_result;

public:
    //kk GroupSpec
    void UpdateSpec();
    void GetVisionInspectionSpecs();
    void SetVisionInspectionSpecs();
};
