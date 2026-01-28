//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Result_EdgeAlign.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/LineEq32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
using namespace VisionEdgeAlign;

Result_EdgeAlign::Result_EdgeAlign()
{
}

Result_EdgeAlign::~Result_EdgeAlign()
{
}

void Result_EdgeAlign::Init(const Ipvm::Point32r2& ptCenter)
{
    VisionAlignResult::Init(ptCenter);

    m_inspectioned = false;
    m_success = false;

    vecLeftSearchROI.clear();
    vecTopSearchROI.clear();
    vecRightSearchROI.clear();
    vecBottomSearchROI.clear();
    vecLowTopSearchROI.clear();

    vecsRefLine.resize(10);

    for (auto& line : vecsRefLine)
    {
        line.m_a = 0.f;
        line.m_b = 0.f;
        line.m_c = 0.f;
    }

    vecLeftEdgeAlignPoints.clear();
    vecTopEdgeAlignPoints.clear();
    vecRightEdgeAlignPoints.clear();
    vecBottomEdgeAlignPoints.clear();
    vecLowTopEdgeAlignPoints.clear();

    vecLeftEdgeAlignPoints_Total.clear();
    vecTopEdgeAlignPoints_Total.clear();
    vecRightEdgeAlignPoints_Total.clear();
    vecBottomEdgeAlignPoints_Total.clear();
    vecLowTopEdgeAlignPoints_Total.clear();

    m_paneCenter = ptCenter;
}

void Result_EdgeAlign::SetImage(const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter)
{
    m_image = image;
    m_paneCenter = paneCenter;
}

void Result_EdgeAlign::FreeMemoryForCalculation()
{
    m_image.Free();
}

const Ipvm::Image8u& Result_EdgeAlign::getImage()
{
    return m_image;
}

const Ipvm::Point32r2& Result_EdgeAlign::getPaneCenter()
{
    return m_paneCenter;
}
