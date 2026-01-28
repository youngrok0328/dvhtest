//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Inspection.h"

//CPP_2_________________________________ This project's headers
#include "Inspection_Impl.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
using namespace VisionEdgeAlign;

Inspection::Inspection(VisionProcessing& processor)
{
    m_impl = new Inspection_Impl(processor);
}

Inspection::~Inspection()
{
    delete m_impl;
}

void Inspection::resetResult()
{
    m_impl->resetResult();
}

bool Inspection::run(const Para& para, const bool detailSetupMode, const bool teachModeForHeightMoldToWall)
{
    return m_impl->run(para, detailSetupMode, teachModeForHeightMoldToWall);
}

Ipvm::Rect32s Inspection::getImageSourceRoiForNoStitch()
{
    return m_impl->getImageSourceRoiForNoStitch();
}

bool Inspection::getImageForAlign(Para& para, bool useFullImage, Ipvm::Image8u& dstImage)
{
    return m_impl->getImageForAlign(para, useFullImage, dstImage);
}

bool Inspection::getImageFor2DEmpty(Para& para, bool useFullImage, Ipvm::Image8u& dstImage)
{
    return m_impl->getImageFor2DEmpty(para, useFullImage, dstImage);
}

void Inspection::setGlobalAlignResult()
{
    m_impl->setGlobalAlignResult();
}

void Inspection::setGlobalAlignResult_ZeroAngle(const Ipvm::Point32r2& shift)
{
    m_impl->setGlobalAlignResult_ZeroAngle(shift);
}

void Inspection::getOverlayResult(
    const Para& para, VisionInspectionOverlayResult* overlayResult, long overlayMode, const bool detailSetupMode)
{
    m_impl->getOverlayResult(para, overlayResult, overlayMode, detailSetupMode);
}

void Inspection::appendTextResult(CString& textResult)
{
    m_impl->appendTextResult(textResult);
}

Result_EdgeAlign* Inspection::getResult_EdgeAlign()
{
    return m_impl->getResult_EdgeAlign();
}

Result_2DEmpty* Inspection::getResult_2DEmpty()
{
    return m_impl->getResult_2DEmpty();
}

float Inspection::getCalculationTime() const
{
    return m_impl->getCalculationTime();
}

LPCTSTR Inspection::getErrorLogText() const
{
    return m_impl->getErrorLogText();
}
