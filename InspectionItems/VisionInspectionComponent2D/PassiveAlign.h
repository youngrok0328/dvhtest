#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "TypeDefine.h"
#include "VisionInspectionComponent2DPara.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionComponent2D;
class VisionScale;
struct ResultPassiveItem;

#define PassiveAngle_Horizontal 0.f
#define PassiveAngle_Horizontal_reverse 180.f
#define PassiveAngle_Vertical 90.f
#define PassiveAngle_Vertical_reverse 270.f
//HDR_6_________________________________ Header body
//
class PassiveAlignResult : public BaseAlignResult
{
public:
    Ipvm::Rect32s rtChipSearch;

    // Chip
    Ipvm::Rect32r m_chip_spec;
    FPI_RECT m_chip_resFinal;
    FPI_RECT m_chip_resFinalBody;

    Ipvm::Rect32s m_chip_resRough;
    Ipvm::Rect32s m_chip_resRoughBody;
    FPI_RECT m_chip_resDetailAlign;
    FPI_RECT m_chip_resDetailBody;

    Ipvm::Rect32s rtSpecBody;

    BOOL bAlignSuccess;

    // Elect
    std::vector<Ipvm::Rect32s> m_elect_spec;
    std::vector<FPI_RECT> m_elect_result;

    std::vector<Ipvm::Rect32s> vecrtResElectbyELPair;
    std::vector<Ipvm::Rect32s> vecrtResElectbyEWPair;

    std::vector<Ipvm::Rect32s> vecrtResElectbyBL;
    std::vector<Ipvm::Rect32s> vecrtResElectbyEL;
    std::vector<Ipvm::Rect32s> vecrtResElectbyEW;

    std::vector<Ipvm::Rect32s> vecrtElectSearch;
    EDGEPOINTS vecfptElectEdge;
    EDGEPOINTS vecfptFitElectEdge;
    std::vector<long> vecnElectSpecEdgeCount;
    std::vector<long> vecnElectResEdgeCount;
    std::vector<float> vecfElectQuality;

    // PAD
    std::vector<Ipvm::Rect32r> m_pad_spec;
    std::vector<Ipvm::Quadrangle32r> m_pad_finalRes; // 최정 판정 위치
    std::vector<Ipvm::Rect32s> m_pad_inspRegion;
    std::vector<Ipvm::LineEq32r> m_pad_baseLine;
    std::vector<Ipvm::Rect32s> m_pad_baseline_searchROI;
    EDGEPOINTS m_pad_baseLine_edges;
    std::vector<Ipvm::LineEq32r> m_pad_sideLine;
    std::vector<Ipvm::LineEq32r> m_pad_topLine;
    std::vector<Ipvm::LineEq32r> m_pad_btmLine;
    std::vector<long> m_pad_baseLineDirection;
    std::vector<Ipvm::Rect32s> m_pad_top_bottom_line_searchROI;
    EDGEPOINTS m_pad_top_bottom_edges;
    Ipvm::Point32r2 m_centerOfPADs;

    //{{mc_DetailAlign관련
    //Debug
    Ipvm::Rect32s m_rtPassiveRough_Align_Result_ROI;
    std::vector<FPI_RECT> m_vecfrtDebug_DetailSearchROI;
    EDGEPOINTS m_vecfptDebug_DetailSearchEdgePoints;
    std::vector<Ipvm::Rect32s> m_vecrtShortenROI; //Horizontal : Left, Right, Vertical : Top, Bottom
    EDGEPOINTS m_vecfptMinoraxisEdgePoint; //Shorten Search Edge Point

    //RotateDegree
    float m_fRotateDegree;

    //}}

    void Clear()
    {
        bAlignSuccess = FALSE;

        FPI_RECT empty_fpi_rect = FPI_RECT(
            Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f));
        ;

        m_chip_spec = Ipvm::Rect32r(0.f, 0.f, 0.f, 0.f);
        m_chip_resRough = Ipvm::Rect32s(0, 0, 0, 0);
        m_chip_resRoughBody = Ipvm::Rect32s(0, 0, 0, 0);
        m_chip_resFinal = empty_fpi_rect;
        m_chip_resFinalBody = empty_fpi_rect;
        m_chip_resDetailAlign = empty_fpi_rect;
        m_chip_resDetailBody = empty_fpi_rect;

        rtChipSearch = Ipvm::Rect32s(0, 0, 0, 0);
        rtSpecBody = Ipvm::Rect32s(0, 0, 0, 0);

        m_elect_spec.clear();
        m_elect_result.clear();

        vecrtElectSearch.clear();
        vecfptElectEdge.clear();
        vecfptFitElectEdge.clear();

        vecrtResElectbyELPair.clear();
        vecrtResElectbyEWPair.clear();

        vecrtResElectbyBL.clear();
        vecrtResElectbyEL.clear();
        vecrtResElectbyEW.clear();

        vecnElectSpecEdgeCount.clear();
        vecnElectResEdgeCount.clear();

        m_pad_spec.clear();
        m_pad_finalRes.clear();
        m_pad_inspRegion.clear();
        m_pad_baseLine.clear();
        m_pad_baseline_searchROI.clear();
        m_pad_baseLine_edges.clear();
        m_pad_sideLine.clear();
        m_pad_topLine.clear();
        m_pad_btmLine.clear();
        m_pad_baseLineDirection.clear();
        m_pad_top_bottom_line_searchROI.clear();
        m_pad_top_bottom_edges.clear();

        m_rtPassiveRough_Align_Result_ROI = Ipvm::Rect32s(0, 0, 0, 0);

        m_centerOfPADs = Ipvm::Point32r2(0.f, 0.f);

        vecfElectQuality.clear();

        m_vecfrtDebug_DetailSearchROI.clear();
        m_vecfptDebug_DetailSearchEdgePoints.clear();
        m_vecrtShortenROI.clear();
        m_vecfptMinoraxisEdgePoint.clear();

        m_fRotateDegree = 0.f;
    }

    PassiveAlignResult()
    {
        Clear();
    }

    virtual ~PassiveAlignResult()
    {
    }
};

class CPassiveAlign
{
public:
    CPassiveAlign(VisionInspectionComponent2D* pChipAlign);
    virtual ~CPassiveAlign(void);

    VisionInspectionComponent2D* m_pChipVision;
    PassiveAlignResult m_result;

    PassiveAlignSpec::CapAlignSpec* GetCapAlignSpec(CString i_strSelectSpecName);

    BOOL DoAlign(const VisionScale& scale, sPassive_InfoDB* i_pPassiveInfoDB, ResultPassiveItem* out_result);
    BOOL MakeSpecROIs(sPassive_InfoDB* i_pPassiveInfoDB);
    BOOL RotateImage(const Ipvm::Image8u& imgOrg, Ipvm::Image8u& o_imgRotate, sPassive_InfoDB* i_pPassiveInfoDB);

private:
    ResultPassiveItem* m_outResult;
};