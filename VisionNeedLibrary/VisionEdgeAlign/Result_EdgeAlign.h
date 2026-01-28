#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../VisionCommon/VisionAlignResult.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace VisionEdgeAlign
{
class __VISION_EDGE_ALIGN_CLASS Result_EdgeAlign : public VisionAlignResult
{
public:
    Result_EdgeAlign();
    ~Result_EdgeAlign();

    void Init(const Ipvm::Point32r2& ptCenter);
    void SetImage(const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter);
    void FreeMemoryForCalculation();

    const Ipvm::Image8u& getImage();
    const Ipvm::Point32r2& getPaneCenter();

    //Edge Search ROI
    std::vector<Ipvm::Rect32s> vecLeftSearchROI;
    std::vector<Ipvm::Rect32s> vecTopSearchROI;
    std::vector<Ipvm::Rect32s> vecRightSearchROI;
    std::vector<Ipvm::Rect32s> vecBottomSearchROI;
    std::vector<Ipvm::Rect32s> vecLowTopSearchROI;

    std::vector<Ipvm::LineEq32r> vecsRefLine;

    //Final Edge Point
    std::vector<Ipvm::Point32r2> vecLeftEdgeAlignPoints;
    std::vector<Ipvm::Point32r2> vecTopEdgeAlignPoints;
    std::vector<Ipvm::Point32r2> vecRightEdgeAlignPoints;
    std::vector<Ipvm::Point32r2> vecBottomEdgeAlignPoints;
    std::vector<Ipvm::Point32r2> vecLowTopEdgeAlignPoints;

    // Total Edge Point
    std::vector<Ipvm::Point32r2> vecLeftEdgeAlignPoints_Total;
    std::vector<Ipvm::Point32r2> vecTopEdgeAlignPoints_Total;
    std::vector<Ipvm::Point32r2> vecRightEdgeAlignPoints_Total;
    std::vector<Ipvm::Point32r2> vecBottomEdgeAlignPoints_Total;
    std::vector<Ipvm::Point32r2> vecLowTopEdgeAlignPoints_Total;

    float fLowtopLeft;
    float fLowtopTop;
    float fLowtopRight;
    float fLowtopBottom;

    bool m_inspectioned;
    bool m_success;

private:
    Ipvm::Image8u m_image;
    Ipvm::Point32r2 m_paneCenter;
};
} // namespace VisionEdgeAlign
