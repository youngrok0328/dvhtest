//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ResultAlign.h"

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
ResultAlign::ResultAlign()
{
}

void ResultAlign::Init(const Ipvm::Point32r2& ptCenter)
{
    VisionAlignResult::Init(ptCenter);

    m_inspectioned = false;
    m_success = false;

    m_carrierTapeInspectionArea = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    m_roughPocketCenter_px = {Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r};

    for (auto& roi : m_carrierTapeEdgeSearchRois)
    {
        roi.SetRectEmpty();
    }

    for (auto& roi : m_pocketEdgeSearchRois)
    {
        roi.SetRectEmpty();
    }

    m_carrierTapeRefLine.resize(2);
    for (auto& line : m_carrierTapeRefLine)
    {
        line.m_a = 0.f;
        line.m_b = 0.f;
        line.m_c = 0.f;
    }

    for (auto& line : m_pocketRefLine)
    {
        line.m_a = 0.f;
        line.m_b = 0.f;
        line.m_c = 0.f;
    }

    for (auto& vecEdgePoints : m_carrierTapeEdgePoints)
    {
        vecEdgePoints.clear();
    }

    for (auto& vecEdgePoints : m_pocketEdgePoints)
    {
        vecEdgePoints.clear();
    }

    for (auto& vecEdgePoints : m_carrierTapeEdgePointsTotal)
    {
        vecEdgePoints.clear();
    }

    for (auto& vecEdgePoints : m_pocketEdgePointsTotal)
    {
        vecEdgePoints.clear();
    }

    m_paneCenter = ptCenter;
}

void ResultAlign::SetImage(
    const Ipvm::Image8u& imageCarrierTape, const Ipvm::Image8u& imagePocket, const Ipvm::Point32r2& paneCenter)
{
    m_imageCarrierTape = imageCarrierTape;
    m_imagePocket = imagePocket;

    m_paneCenter = paneCenter;
}

void ResultAlign::FreeMemoryForCalculation()
{
    m_imageCarrierTape.Free();
    m_imagePocket.Free();
}

const Ipvm::Image8u& ResultAlign::getImage(bool isPocket) const
{
    return isPocket ? m_imagePocket : m_imageCarrierTape;
}

const Ipvm::Point32r2& ResultAlign::getPaneCenter() const
{
    return m_paneCenter;
}
