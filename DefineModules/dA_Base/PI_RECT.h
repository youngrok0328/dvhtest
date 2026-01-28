#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
struct _DA_BASE_MODULE_API PI_RECT
{
    PI_RECT();
    explicit PI_RECT(const Ipvm::Rect32s& Src);
    PI_RECT(const PI_RECT& Src);
    PI_RECT(const Ipvm::Point32s2& ptLT, const Ipvm::Point32s2& ptRT, const Ipvm::Point32s2& ptLB,
        const Ipvm::Point32s2& ptRB);
    PI_RECT(const Ipvm::Point32r2& ptLT, const Ipvm::Point32r2& ptRT, const Ipvm::Point32r2& ptLB,
        const Ipvm::Point32r2& ptRB);

    PI_RECT& operator=(const PI_RECT& aRect);
    PI_RECT& operator=(const Ipvm::Rect32s& Rect);

    PI_RECT& Move(int x, int y);
    PI_RECT& Move(int x, int y, BOOL bInflate);
    PI_RECT Offset(short nOffsetX, short nOffsetY, BOOL bInner);
    PI_RECT Offset(short nLeft, short nRight, short nTop, short nBottom, BOOL bInner);
    PI_RECT Rotate(double dRadian);
    PI_RECT& Rotate(float fOrigX, float fOrigY, float fCosVal, float fSinVal);

    BOOL Verify();
    void Normalize(Ipvm::Point32s2 ptImageSize);
    void Set(long ltx, long lty, long rtx, long rty, long lbx, long lby, long rbx, long rby);
    void Set(Ipvm::Point32s2 LT, Ipvm::Point32s2 RT, Ipvm::Point32s2 LB, Ipvm::Point32s2 RB);
    Ipvm::Rect32s GetExtRect() const;
    Ipvm::Rect32s GetMidRect() const;
    Ipvm::Point32r2 GetCenter() const;
    Ipvm::Point32s2 GetLTPoint() const;
    Ipvm::Point32s2 GetRTPoint() const;
    Ipvm::Point32s2 GetLBPoint() const;
    Ipvm::Point32s2 GetRBPoint() const;
    BOOL SetLTPoint(const Ipvm::Point32s2& ptLT);
    BOOL SetRTPoint(const Ipvm::Point32s2& ptRT);
    BOOL SetLBPoint(const Ipvm::Point32s2& ptLB);
    BOOL SetRBPoint(const Ipvm::Point32s2& ptRB);
    Ipvm::Rect32s CreateExternalMinimumRect() const;

    int32_t ltX, ltY; // left-top
    int32_t rtX, rtY; // right-top
    int32_t brX, brY; // bottom-right
    int32_t blX, blY; // bottom-left
};
