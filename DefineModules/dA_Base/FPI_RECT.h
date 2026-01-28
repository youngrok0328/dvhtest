#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
struct PI_RECT;

//HDR_6_________________________________ Header body
//
struct _DA_BASE_MODULE_API FPI_RECT
{
    FPI_RECT();
    FPI_RECT(Ipvm::Point32r2 _fptLT, Ipvm::Point32r2 _fptRT, Ipvm::Point32r2 _fptLB, Ipvm::Point32r2 _fptRB);
    FPI_RECT& operator=(const FPI_RECT& spiRect);
    PI_RECT GetSPI_RECT() const;
    void SetSPI_RECT(PI_RECT spiRect); //kircheis_EdgePeeling

    FPI_RECT& Move(float x, float y);
    Ipvm::Rect32s GetRoughRect() const;
    Ipvm::Rect32s GetCRect() const;
    Ipvm::Rect32r GetFRect() const;
    Ipvm::Rect32r GetExtFRect() const;
    Ipvm::Rect32s GetExtCRect() const;
    Ipvm::Point32r2 GetCenter() const;
    FPI_RECT Rotate(double dRadian) const;
    FPI_RECT Rotate(double dRadian, Ipvm::Point32r2 fptCenter) const;
    void NormalizeRect();

    Ipvm::Point32r2 fptLT;
    Ipvm::Point32r2 fptRT;
    Ipvm::Point32r2 fptRB;
    Ipvm::Point32r2 fptLB;
};