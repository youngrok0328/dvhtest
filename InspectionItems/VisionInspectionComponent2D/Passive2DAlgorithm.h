#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "PassiveAlign.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPI_EdgeDetect;
class CPI_Geometry;
class VisionScale;

//HDR_6_________________________________ Header body
//
enum enumPassiveSearchROIPos
{
    SearchROI_POS_TOP,
    SearchROI_POS_BTM,
    SearchROI_POS_LEFT,
    SearchROI_POS_RIGHT,
};

enum PassiveAngleType
{
    Passive_Horizontal,
    Passive_Vertical,
    Passive_MIA,
};

struct sEdgeSearchSizeInfo
{
    long nSearchLength;
    long nSearchWidth;
    long nROISize;
};

class CPassive2DAlgorithm
{
public:
    CPassive2DAlgorithm(sPassive_InfoDB* i_pPassiveInfoDB, PassiveAlignSpec::CapAlignSpec* i_pPassiveAlignParam,
        CPI_EdgeDetect* i_pEdgeDetect, ResultPassiveItem* i_pPassiveResult, PassiveAlignResult& o_PassiveAlignResult);
    virtual ~CPassive2DAlgorithm(void);

private:
    sPassive_InfoDB* m_pPassiveInfoDB;
    PassiveAlignSpec::CapAlignSpec* m_pPassiveAlignParam;
    PassiveAlignResult* m_pPassiveAlignResult;
    ResultPassiveItem* m_pPassiveResult_DebugItem;
    CPI_EdgeDetect* m_pEdgeDetect;

    struct sEdgeSearchNeedInfo
    {
        Ipvm::Rect32s rtSearchROI;
        long nSearchLength;
        long nSearchWidth;
        long nROISize;
        long nSearchDirection;
        long nSearchType;
        Ipvm::Point32s2 ptSearchStartPos;
        long nOriginSearchDirection; //현재 탐색할려고 하는 영역
    };

public:
    bool DoCapacitor_Align(const VisionScale& scale, const Ipvm::Image8u& i_RoughAlignImage,
        const Ipvm::Image8u& i_OutlineAlign_ElectrodeImage, const Ipvm::Image8u& i_OutlineAlign_BodyImage);
    bool DoMIA_Align(const VisionScale& scale, const Ipvm::Image8u& i_RoughAlignImage,
        const Ipvm::Image8u& i_OutlineAlign_BodyImage);

private:
    bool RoughAlign(const VisionScale& scale, const Ipvm::Image8u& i_RoughAlignImage, FPI_RECT i_frtSpecROI,
        FPI_RECT& o_frtRoughROI); //kircheis_MED2Passive

    bool MakeDetailAlignROI_byRoughAlign(const VisionScale& scale, FPI_RECT i_frtRoughROI,
        std::vector<std::vector<FPI_RECT>>& o_vec2frtChip_SearchROI); //kircheis_MED2Passive

    bool DoCapacitor_DetailAlign(const Ipvm::Image8u& i_OutlineAlign_ElectrodeImage,
        const Ipvm::Image8u& i_OutlineAlign_BodyImage,
        const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI, FPI_RECT i_frtRoughAlignROI,
        long i_nPassiveAngleType);
    bool DoMIA_DetailAlign(const Ipvm::Image8u& i_OutlineAlign_BodyImage,
        const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI, FPI_RECT i_frtRoughAlignROI,
        long i_nPassiveAngleType);

    bool PassiveBody_FindMajorAxisEdgePoint(const Ipvm::Image8u i_OutlineAlign_BodyImage,
        sEdgeSearchNeedInfo i_sEdgeSearchNeedInfo, long i_nPaasiveAngleType, FPI_RECT i_frtSpecROI,
        long i_nMinimum_EdgePointCount, EDGEPOINTS& o_vecfptEdgePoint, BOOL i_bFristEdgeDeteceMode = FALSE);
    float GetAveragePoint(const EDGEPOINTS& i_vecEdgePoint, long i_nPassiveAngleType);
    bool RemoveNoiseEdgePoint(EDGEPOINTS& io_vecEdgePoint, long i_nPassiveAngleType, Ipvm::LineSeg32r i_SpecRefLine);

    float GetRotateDegree(
        float i_fSpecAngle_Degree, const EDGEPOINTS& i_vecfptEdgePonts1, const EDGEPOINTS& i_vecfptEdgePonts2);
    void CreateRefLine_UseFindMinorEdgePoint(long i_nPassiveType, FPI_RECT i_frtRoughAlign_Result,
        Ipvm::LineEq32r& o_line_Ref1_RoughAlignPoint, Ipvm::LineEq32r& o_line_Ref2_RoughAlignPoint);

    bool PassiveBody_FindMinorAxisEdgePoint(const Ipvm::Image8u i_OutlineAlign_ElectrodeImage,
        Ipvm::LineEq32r& i_line_Ref1, Ipvm::LineEq32r& i_line_Ref2,
        const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI, long i_nSearchType,
        long i_nPassiveDirection, long i_nSearchDirection1, long i_nSearchDirection2, long i_nSearchDirection,
        EDGEPOINTS& o_vecfptMinorAxisEdgePoints, BOOL i_bFristEdgeDeteceMode = FALSE);
    bool CreateMinor_EdgeDetectROI(Ipvm::Point32r2 i_fptCenterPointBy2Line, Ipvm::Point32r2 i_fptF_LineCenter,
        Ipvm::Point32r2 i_fptS_LineCenter, Ipvm::Point32r2 i_fptSearchROISize, long i_nPassiveAngleType,
        std::vector<Ipvm::Rect32s>& o_vecrtSearchROI);
    bool MinorDetectEdgePoint(const Ipvm::Image8u& i_OutlineAlign_ElectrodeImage, long i_nPassiveAngleType,
        long i_nSearchType, long i_nEdgeSearchDir, const std::vector<Ipvm::Rect32s>& i_vecrtShortenSearchROI,
        Ipvm::Point32r2& o_fptMinoraxisEdgePoint, BOOL i_bFristEdgeDeteceMode = FALSE);
    Ipvm::Point32r2 GetMinorEdgePoint(const EDGEPOINTS& i_vecfptEdgePoint);

    bool MakeRectForPointProjection(long i_nPassiveAngleType, const EDGEPOINTS& i_vecfptMajorPoints1,
        const EDGEPOINTS& i_vecfptMajorPoints2, Ipvm::Point32r2 i_fptMinorAxisPoint1,
        Ipvm::Point32r2 i_fptMinorAxisPoint2, FPI_RECT& o_frtBodyAlignResult);
    bool MakeRectForLineProjection(const std::vector<EDGEPOINTS>& i_vecfptMajorPoints, FPI_RECT& o_frtBodyAlignResult);

    bool ModifyBodyAlignRect(long i_nPassiveAngleType, long i_nResultWidth, long i_nResultLength);
    bool Create_Capacitor_Electrode(long i_nPassiveType, FPI_RECT i_frtAlignResult);
    bool Set_ElectrodeArea(long i_nDirection, FPI_RECT i_frtAlign_Result, long i_nElectrodeWidth);
    EDGEPOINTS GetElectordeTargetPoints(
        long i_nDirection, FPI_RECT i_frtAlign_Result, long i_nElectrodeWidth, bool i_bRefPoints = true);
    bool Create_PAD_Area(const VisionScale& scale, long i_nPassiveAngleType, FPI_RECT i_frtAlignResult);
    Ipvm::Point32r2 Get_PAD_Center(const VisionScale& scale, long i_nPassiveAngleType, FPI_RECT i_frtAlignResult,
        bool i_bMinusDirection = true); //기준은 - 방향으로부터
    Ipvm::Rect32r Get_PAD_Area(
        const VisionScale& scale, long i_nPassiveAngleType, Ipvm::Point32r2 i_fptOffsetPadCenter);
    //
    bool GetMajorEdgeSearchNeedInfo(long i_nPassiveType,
        const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI,
        std::vector<sEdgeSearchNeedInfo>& o_vecsEdgeSearchNeedInfo);
    std::vector<long> GetEdgeSearchSDirectionType(long i_nPassiveType, bool i_bIsMinorEdgeType = false);
    sEdgeSearchSizeInfo GetEdgeSearchSize_DirectionType(const long i_nDirection, const Ipvm::Rect32s i_rtSearchROI);
    long GetEdgeSearchDirection(long i_nCurDirectionType);
    Ipvm::Point32s2 Get_EdgeSearch_StartPoint(
        long i_nPassiveDirection, long i_nEdgeSearchDir, long i_SearchDir, Ipvm::Rect32s i_rtSearchROI);
};