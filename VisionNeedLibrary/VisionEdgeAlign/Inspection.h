#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace VisionEdgeAlign
{
class Para;
class Result_EdgeAlign;
class Result_2DEmpty;
class Inspection_Impl;
} // namespace VisionEdgeAlign

class VisionDebugInfoGroup;
class VisionImageLot;
class VisionInspectionOverlayResult;
class VisionProcessing;
class VisionReusableMemory;

//HDR_6_________________________________ Header body
//
namespace VisionEdgeAlign
{
class __VISION_EDGE_ALIGN_CLASS Inspection
{
public:
    Inspection(VisionProcessing& processor);
    ~Inspection();

    void resetResult();
    bool run(const Para& para, const bool detailSetupMode, const bool teachModeForHeightMoldToWall = false);

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
    Inspection_Impl* m_impl;
};
} // namespace VisionEdgeAlign
