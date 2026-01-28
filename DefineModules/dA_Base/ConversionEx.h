#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace Ipvm
{
struct Point32r2;
struct Quadrangle32r;
} // namespace Ipvm
struct PI_RECT;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
struct _DA_BASE_MODULE_API ConversionEx
{
    static Ipvm::Quadrangle32r ToQuadrangle32r(
        const Ipvm::Point32r2& lt, const Ipvm::Point32r2& rt, const Ipvm::Point32r2& lb, const Ipvm::Point32r2& rb);

    static Ipvm::Quadrangle32r ToQuadrangle32r(const PI_RECT& piRect);

    static Ipvm::Quadrangle32r ToQuadrangle32r(const FPI_RECT& fpiRect);
};