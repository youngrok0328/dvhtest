//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ResultLayer.h"

//CPP_2_________________________________ This project's headers
#include "SpecLayer.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ResultLayer::ResultLayer()
{
}

ResultLayer::ResultLayer(const SpecLayer& spec, const Ipvm::Point32r2& px2um, const Ipvm::Point32r2& imageCenter)
    : m_name(spec.m_name)
{
    for (long roiIndex = 0; roiIndex < spec.m_single.count(); roiIndex++)
    {
        m_pre.Add(px2um, imageCenter, spec.m_single[roiIndex]);
    }
}

ResultLayer::~ResultLayer()
{
}
