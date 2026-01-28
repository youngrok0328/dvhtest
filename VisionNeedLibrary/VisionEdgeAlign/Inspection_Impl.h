#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Result_2DEmpty.h"
#include "Result_EdgeAlign.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
namespace VisionEdgeAlign
{
class Para;
}

class VisionDebugInfoGroup;
class VisionImageLot;
class VisionInspectionOverlayResult;
class VisionProcessing;
class VisionReusableMemory;
class CPackageSpec;
class CPI_EdgeDetect;

//HDR_6_________________________________ Header body
//

namespace VisionEdgeAlign
{
class Inspection_Impl
{
public:
    Inspection_Impl(VisionProcessing& processor);
    ~Inspection_Impl();

    void resetResult();
    bool run(const Para& para, const bool detailSetupMode, const bool teachModeForHeightMoldToWall);

    Ipvm::Rect32s getImageSourceRoiForNoStitch();
    bool getImageForAlign(Para& para, bool useFullImage, Ipvm::Image8u& dstImage);
    bool getImageFor2DEmpty(Para& para, bool useFullImage, Ipvm::Image8u& dstImage);

    void setGlobalAlignResult();
    void setGlobalAlignResult_ZeroAngle(const Ipvm::Point32r2& shift);

    void getOverlayResult(
        const Para& para, VisionInspectionOverlayResult* overlayResult, long overlayMode, const bool detailSetupMode);
    void appendTextResult(CString& textResult);

    Result_EdgeAlign* getResult_EdgeAlign();
    Result_2DEmpty* getResult_2DEmpty();
    float getCalculationTime() const;
    LPCTSTR getErrorLogText() const;

private:
    VisionProcessing& m_processor;
    CPackageSpec& m_packageSpec;

    Result_EdgeAlign m_result_edgeAlign;
    Result_2DEmpty m_result_2DEmpty;

    bool m_teachModeForHeightMoldToWall;
    CString m_errorLogText;

    CPI_EdgeDetect* m_pEdgeDetect;
    float m_fHeightMoldToWall;
    float m_fBodySizeX_Pixel;
    float m_fBodySizeY_Pixel;
    float m_fBodySizeX_Check_Offset_LowLimit;
    float m_fBodySizeY_Check_Offset_LowLimit;
    float m_fCalcTime;

    void setDebugInfo(const bool detailSetupMode);
    bool DoAlign(const Para& para, const bool detailSetupMode);

    long BodyEdgeDetect_LeftbyMatching(const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter,
        Ipvm::LineEq32r eqLineCenter = Ipvm::LineEq32r(0.f, 0.f, 0.f));
    long BodyEdgeDetect_TopbyMatching(const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter);
    long BodyEdgeDetect_RightbyMatching(const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter,
        Ipvm::LineEq32r eqLineCenter = Ipvm::LineEq32r(0.f, 0.f, 0.f));
    long BodyEdgeDetect_BottombyMatching(
        const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter);
    long BodyEdgeDetect_Lowtop(const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter);

    bool MoveEdgeROIforSide(const Ipvm::LineEq32r& i_eqLineCenter, Ipvm::Rect32s& io_rtEdgeROI);

    Ipvm::Rect32s ModifiedROIbyImage(const Ipvm::Image8u& i_image,
        const Ipvm::Rect32s&
            i_rtEdgeSearchROI); //mc_KM Site에서 EdgeAlign 영역이 Image 끝점에 위치하여 EdgeAlign이 수행되지 않은 경우가 있었다.

    long RoughAlign_LineAlign(const Para& para, long nDirection, std::vector<Ipvm::Point32r2> vecfpNewPoints,
        std::vector<Ipvm::Point32r2> vecfpFirstEdge, std::vector<Ipvm::Point32r2> vecfpBestEdge);

    long SetResult(BOOL bLowtopMode);
    long SetResultForRound_UpDown(
        const bool detailSetupMode, const Ipvm::Image8u& image, long nSelPackType); //kircheis_RoundPKG
    long ReAlignForRound_UpDown(const bool detailSetupMode, const Ipvm::Image8u& image,
        const Ipvm::EllipseEq32r& sEllipseUp, const Ipvm::EllipseEq32r& sEllipseDown); //kircheis_RoundPKG

    BOOL CheckEmptyPocket3D(
        const Para& para, const Ipvm::Image8u& i_image, float& o_fCurPackageHeight, BOOL& o_bIsEmptyPocket);

    // return FALSE는 검사 결과를 믿지 말라는 의미.
    // i_fPkgMoldSizeX_PXL, i_fPkgMoldSizeY_PXL는 Package의 크기(Lead Package의 경우 Lead 길이 제외)를 Pixel 단위로 입력한다.  MIL * m_fCalcMilsToPixel(X oy Y)
    // i_fPkgHeight_GV 는 Package의 높이를 GV로 환산하여 입력한다.  MIL * 25.4 *2 / m_px2um_z  (*2는 GV가 2배 증폭되니까 이를 맞추기 위함)
    // o_bIsEmpty 는 함수 내부에서 Empty로 판정하면 TRUE로 리턴된다.
    // 함수의 Return Value와 o_bIsEmpty가 모두 TRUE면 EMPTY이다.
    BOOL EmptyCheckUsePaneCenter(const Para& para, const Ipvm::Image8u& i_image, Ipvm::Point32s2 i_ptPaneCenter,
        float i_fPkgMoldSizeX_PXL, float i_fPkgMoldSizeY_PXL, float i_fPkgHeightGV, float& o_fCenterHeightGV,
        BOOL& o_bIsEmpty);

    //kircheis_3DEmpty for Leadless // 이하는 Leadless Package를 위한 알고리즘 이다
    //kircheis_20160713
    BOOL CalcMoldHeight(const Para& para, const bool detailSetupMode, const Ipvm::Image8u& i_image,
        float& o_fMoldHeight, BOOL& o_bIsEmpty, BOOL& o_bIsDouble);
    BOOL CalcTrayWallHeight(const Para& para, const Ipvm::Image8u& i_image, Ipvm::Rect32s i_rtTrayWallSearchArea,
        Ipvm::Rect32s i_rtMold, float i_fMoldHeight, float& o_fTrayWallHeight);

    // return FALSE는 검사 결과를 믿지 말라는 의미.
    // o_bIsEmpty 는 함수 내부에서 Empty로 판정하면 TRUE로 리턴된다.
    // o_bIsDouble는 함수 내부에서 Double로 판정하면 TRUE로 리턴된다.
    // 함수의 Return Value와 o_bIsEmpty가 모두 TRUE면 EMPTY이다.
    // 함수의 Return Value와 o_bIsDouble가 모두 TRUE면 DOUBLE이다.
    // 이 함수의 호출 시점은 Edge Align이 끝난 이후에 하도록 한다.
    BOOL EmptyDoubleCheckUseTrayWallForLeadless(const Para& para, const bool detailSetupMode,
        const Ipvm::Image8u& i_image, BOOL& o_bIsEmpty, BOOL& o_bIsDouble);

    bool emptyCheck2D(const Para& para);
    void calcParallelism();
};
} // namespace VisionEdgeAlign
