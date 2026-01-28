//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ConversionEx.h"

//CPP_2_________________________________ This project's headers
#include "FPI_RECT.h"
#include "PI_RECT.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Quadrangle32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Ipvm::Quadrangle32r ConversionEx::ToQuadrangle32r(
    const Ipvm::Point32r2& lt, const Ipvm::Point32r2& rt, const Ipvm::Point32r2& lb, const Ipvm::Point32r2& rb)
{
    return Ipvm::Quadrangle32r{lt.m_x, lt.m_y, rt.m_x, rt.m_y, lb.m_x, lb.m_y, rb.m_x, rb.m_y};
}

Ipvm::Quadrangle32r ConversionEx::ToQuadrangle32r(const PI_RECT& piRect)
{
    return Ipvm::Quadrangle32r{static_cast<float>(piRect.ltX), static_cast<float>(piRect.ltY),
        static_cast<float>(piRect.rtX), static_cast<float>(piRect.rtY), static_cast<float>(piRect.blX),
        static_cast<float>(piRect.blY), static_cast<float>(piRect.brX), static_cast<float>(piRect.brY)};
}

Ipvm::Quadrangle32r ConversionEx::ToQuadrangle32r(const FPI_RECT& fpiRect)
{
    return Ipvm::Quadrangle32r{fpiRect.fptLT.m_x, fpiRect.fptLT.m_y, fpiRect.fptRT.m_x, fpiRect.fptRT.m_y,
        fpiRect.fptLB.m_x, fpiRect.fptLB.m_y, fpiRect.fptRB.m_x, fpiRect.fptRB.m_y};
}
