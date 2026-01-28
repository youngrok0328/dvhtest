//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PI_RECT.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
// PI_RECT
PI_RECT::PI_RECT()
    : ltX(0)
    , ltY(0)
    , rtX(0)
    , rtY(0)
    , brX(0)
    , brY(0)
    , blX(0)
    , blY(0)
{
}

PI_RECT::PI_RECT(const Ipvm::Rect32s& Src)
    : ltX(Src.m_left)
    , ltY(Src.m_top)
    , rtX(Src.m_right - 1)
    , rtY(Src.m_top)
    , brX(Src.m_right - 1)
    , brY(Src.m_bottom - 1)
    , blX(Src.m_left)
    , blY(Src.m_bottom - 1)
{
}

PI_RECT::PI_RECT(const PI_RECT& Src)
    : ltX(Src.ltX)
    , ltY(Src.ltY)
    , rtX(Src.rtX)
    , rtY(Src.rtY)
    , brX(Src.brX)
    , brY(Src.brY)
    , blX(Src.blX)
    , blY(Src.blY)
{
}

PI_RECT::PI_RECT(
    const Ipvm::Point32s2& ptLT, const Ipvm::Point32s2& ptRT, const Ipvm::Point32s2& ptLB, const Ipvm::Point32s2& ptRB)
    : ltX(ptLT.m_x)
    , ltY(ptLT.m_y)
    , rtX(ptRT.m_x)
    , rtY(ptRT.m_y)
    , blX(ptLB.m_x)
    , blY(ptLB.m_y)
    , brX(ptRB.m_x)
    , brY(ptRB.m_y)
{
}

PI_RECT::PI_RECT(
    const Ipvm::Point32r2& ptLT, const Ipvm::Point32r2& ptRT, const Ipvm::Point32r2& ptLB, const Ipvm::Point32r2& ptRB)
    : ltX((long)(ptLT.m_x))
    , ltY((long)(ptLT.m_y))
    , rtX((long)(ptRT.m_x))
    , rtY((long)(ptRT.m_y))
    , blX((long)(ptLB.m_x))
    , blY((long)(ptLB.m_y))
    , brX((long)(ptRB.m_x))
    , brY((long)(ptRB.m_y))
{
}

PI_RECT& PI_RECT::operator=(const PI_RECT& aRect)
{
    ltX = aRect.ltX;
    ltY = aRect.ltY;
    rtX = aRect.rtX;
    rtY = aRect.rtY;
    brX = aRect.brX;
    brY = aRect.brY;
    blX = aRect.blX;
    blY = aRect.blY;

    return *this;
}

PI_RECT& PI_RECT::operator=(const Ipvm::Rect32s& Rect)
{
    ltX = Rect.m_left;
    ltY = Rect.m_top;
    rtX = Rect.m_right;
    rtY = Rect.m_top;
    brX = Rect.m_right;
    brY = Rect.m_bottom;
    blX = Rect.m_left;
    blY = Rect.m_bottom;

    return *this;
}

PI_RECT& PI_RECT::Move(int x, int y)
{
    ltX += x;
    rtX += x;
    blX += x;
    brX += x;
    ltY += y;
    rtY += y;
    blY += y;
    brY += y;

    return *this;
}

PI_RECT& PI_RECT::Move(int x, int y, BOOL bInflate)
{
    if (bInflate)
    {
        ltX -= x;
        rtX += x;
        blX -= x;
        brX += x;
        ltY -= y;
        rtY -= y;
        blY += y;
        brY += y;
    }
    else
    {
        ltX += x;
        rtX -= x;
        blX += x;
        brX -= x;
        ltY += y;
        rtY += y;
        blY -= y;
        brY -= y;
    }

    return *this;
}

PI_RECT PI_RECT::Offset(short nOffsetX, short nOffsetY, BOOL bInner)
{
    //binner가 true이면 rect의 안쪽으로 0이면 바깥쪽으로 나가는 spi_rect임.
    float fx, fy, fx2, fy2, fx3, fy3, fx4, fy4, fdet;
    float fcos, fsin;
    PI_RECT rcspiOut;

    fx = (float)(rtX - ltX);
    fy = (float)(rtY - ltY);
    fdet = (float)sqrt(fx * fx + fy * fy);
    if (fdet == 0)
        return *this;
    fcos = fx / fdet;
    fsin = fy / fdet;

    fx2 = fcos * nOffsetX * (bInner == 1 ? 1.0f : -1.0f);
    fy2 = fsin * nOffsetX * (bInner == 1 ? 1.0f : -1.0f);
    //horizontal pos.

    fx = (float)(blX - ltX);
    fy = (float)(blY - ltY);
    fdet = (float)sqrt(fx * fx + fy * fy);
    if (fdet == 0)
        return *this;
    fcos = fx / fdet;
    fsin = fy / fdet;

    fx3 = fcos * nOffsetY * (bInner == 1 ? 1 : -1);
    fy3 = fsin * nOffsetY * (bInner == 1 ? 1 : -1);
    //vertical pos..
    fx4 = fx2 + fx3;
    fy4 = fy2 + fy3;
    fx4 += ltX;
    fy4 += ltY;

    rcspiOut.ltX = (long)fx4;
    rcspiOut.ltY = (long)fy4;

    fx = (float)(ltX - rtX);
    fy = (float)(ltY - rtY);
    fdet = (float)sqrt(fx * fx + fy * fy);
    if (fdet == 0)
        return *this;
    fcos = fx / fdet;
    fsin = fy / fdet;

    fx2 = fcos * nOffsetX * (bInner == 1 ? 1 : -1);
    fy2 = fsin * nOffsetX * (bInner == 1 ? 1 : -1);
    //horizontal pos.

    fx = (float)(brX - rtX);
    fy = (float)(brY - rtY);
    fdet = (float)sqrt(fx * fx + fy * fy);
    if (fdet == 0)
        return *this;
    fcos = fx / fdet;
    fsin = fy / fdet;

    fx3 = fcos * nOffsetY * (bInner == 1 ? 1 : -1);
    fy3 = fsin * nOffsetY * (bInner == 1 ? 1 : -1);
    //vertical pos..
    fx4 = fx2 + fx3;
    fy4 = fy2 + fy3;
    fx4 += (float)rtX;
    fy4 += (float)rtY;

    rcspiOut.rtX = (long)fx4;
    rcspiOut.rtY = (long)fy4;

    fx = (float)(brX - blX);
    fy = (float)(brY - blY);
    fdet = (float)sqrt(fx * fx + fy * fy);
    if (fdet == 0)
        return *this;
    fcos = fx / fdet;
    fsin = fy / fdet;

    fx2 = fcos * nOffsetX * (bInner == 1 ? 1 : -1);
    fy2 = fsin * nOffsetX * (bInner == 1 ? 1 : -1);
    //horizontal pos.

    fx = (float)(ltX - blX);
    fy = (float)(ltY - blY);
    fdet = (float)sqrt(fx * fx + fy * fy);
    if (fdet == 0)
        return *this;
    fcos = fx / fdet;
    fsin = fy / fdet;

    fx3 = fcos * nOffsetY * (bInner == 1 ? 1 : -1);
    fy3 = fsin * nOffsetY * (bInner == 1 ? 1 : -1);
    //vertical pos..
    fx4 = fx2 + fx3;
    fy4 = fy2 + fy3;
    fx4 += blX;
    fy4 += blY;

    rcspiOut.blX = (long)fx4;
    rcspiOut.blY = (long)fy4;

    fx = (float)(blX - brX);
    fy = (float)(blY - brY);
    fdet = (float)sqrt(fx * fx + fy * fy);
    if (fdet == 0)
        return *this;
    fcos = fx / fdet;
    fsin = fy / fdet;

    fx2 = fcos * nOffsetX * (bInner == 1 ? 1 : -1);
    fy2 = fsin * nOffsetX * (bInner == 1 ? 1 : -1);
    //horizontal pos.

    fx = (float)(rtX - brX);
    fy = (float)(rtY - brY);
    fdet = (float)sqrt(fx * fx + fy * fy);
    if (fdet == 0)
        return *this;
    fcos = fx / fdet;
    fsin = fy / fdet;

    fx3 = fcos * nOffsetY * (bInner == 1 ? 1 : -1);
    fy3 = fsin * nOffsetY * (bInner == 1 ? 1 : -1);
    //vertical pos..
    fx4 = fx2 + fx3;
    fy4 = fy2 + fy3;
    fx4 += brX;
    fy4 += brY;

    rcspiOut.brX = (long)fx4;
    rcspiOut.brY = (long)fy4;

    return rcspiOut;
}

PI_RECT PI_RECT::Offset(short nLeft, short nRight, short nTop, short nBottom, BOOL bInner)
{
    Ipvm::Point32s2 ptLT, ptLB, ptRT, ptRB;
    PI_RECT srtTemp;

    srtTemp = Offset(nLeft, nTop, bInner);
    ptLT = Ipvm::Point32s2(srtTemp.ltX, srtTemp.ltY);

    srtTemp = Offset(nLeft, nBottom, bInner);
    ptLB = Ipvm::Point32s2(srtTemp.blX, srtTemp.blY);

    srtTemp = Offset(nRight, nTop, bInner);
    ptRT = Ipvm::Point32s2(srtTemp.rtX, srtTemp.rtY);

    srtTemp = Offset(nRight, nBottom, bInner);
    ptRB = Ipvm::Point32s2(srtTemp.brX, srtTemp.brY);

    return PI_RECT(ptLT, ptRT, ptRB, ptLB);
}

PI_RECT& PI_RECT::Rotate(float fOrigX, float fOrigY, float fCosVal, float fSinVal)
{
    float fTempX;
    float fTempY;

    fTempX = (blX - fOrigX) * fCosVal + (blY - fOrigY) * fSinVal + fOrigX;
    fTempY = -(blX - fOrigX) * fSinVal + (blY - fOrigY) * fCosVal + fOrigY;

    blX = long(fTempX + 0.5f);
    blY = long(fTempY + 0.5f);

    fTempX = (brX - fOrigX) * fCosVal + (brY - fOrigY) * fSinVal + fOrigX;
    fTempY = -(brX - fOrigX) * fSinVal + (brY - fOrigY) * fCosVal + fOrigY;

    brX = long(fTempX + 0.5f);
    brY = long(fTempY + 0.5f);

    fTempX = (ltX - fOrigX) * fCosVal + (ltY - fOrigY) * fSinVal + fOrigX;
    fTempY = -(ltX - fOrigX) * fSinVal + (ltY - fOrigY) * fCosVal + fOrigY;

    ltX = long(fTempX + 0.5f);
    ltY = long(fTempY + 0.5f);

    fTempX = (rtX - fOrigX) * fCosVal + (rtY - fOrigY) * fSinVal + fOrigX;
    fTempY = -(rtX - fOrigX) * fSinVal + (rtY - fOrigY) * fCosVal + fOrigY;

    rtX = long(fTempX + 0.5f);
    rtY = long(fTempY + 0.5f);

    return *this;
}

PI_RECT PI_RECT::Rotate(double dRadian)
{
    PI_RECT srtTemp(
        Ipvm::Point32s2(ltX, ltY), Ipvm::Point32s2(rtX, rtY), Ipvm::Point32s2(blX, blY), Ipvm::Point32s2(brX, brY));

    //1) 원점 이동.
    srtTemp.Move((int)(-GetCenter().m_x), (int)(-GetCenter().m_y)); // 원점 이동.

    //2) 회전.
    double cosine = cos(-dRadian);
    double sine = sin(-dRadian);

    PI_RECT srtTemp2 = srtTemp;
    srtTemp2.ltX = (long)(srtTemp.ltX * cosine - srtTemp.ltY * sine);
    srtTemp2.ltY = (long)(srtTemp.ltX * sine + srtTemp.ltY * cosine);
    srtTemp2.rtX = (long)(srtTemp.rtX * cosine - srtTemp.rtY * sine);
    srtTemp2.rtY = (long)(srtTemp.rtX * sine + srtTemp.rtY * cosine);
    srtTemp2.blX = (long)(srtTemp.blX * cosine - srtTemp.blY * sine);
    srtTemp2.blY = (long)(srtTemp.blX * sine + srtTemp.blY * cosine);
    srtTemp2.brX = (long)(srtTemp.brX * cosine - srtTemp.brY * sine);
    srtTemp2.brY = (long)(srtTemp.brX * sine + srtTemp.brY * cosine);

    //3) 다시 본래위치로 이동.
    srtTemp2.Move((int)(GetCenter().m_x), (int)(GetCenter().m_y));

    return srtTemp2;
}

BOOL PI_RECT::Verify()
{
    BOOL bPositive = (ltX >= 0 && ltY >= 0 && rtX >= 0 && rtY >= 0 && brX >= 0 && brY >= 0 && blX >= 0 && blY >= 0);
    BOOL bValid = (ltX < rtX || ltY < blY || rtY < brY || blX < brX);

    return bPositive && bValid;
}

void PI_RECT::Normalize(Ipvm::Point32s2 ptImageSize)
{
    int nXSize = ptImageSize.m_x;
    int nYSize = ptImageSize.m_y;

    if (ltX < 0)
        ltX = 0;
    if (ltX >= nXSize)
        ltX = nXSize - 1;

    if (ltY >= nYSize)
        ltY = nYSize - 1;
    if (ltY < 0)
        ltY = 0;

    if (rtX < 0)
        rtX = 0;
    if (rtX >= nXSize)
        rtX = nXSize - 1;

    if (rtY < 0)
        rtY = 0;
    if (rtY >= nYSize)
        rtY = nYSize - 1;

    if (blX < 0)
        blX = 0;
    if (blX >= nXSize)
        blX = nXSize - 1;

    if (blY >= nYSize)
        blY = nYSize - 1;
    if (blY < 0)
        blY = 0;

    if (brX < 0)
        brX = 0;
    if (brX >= nXSize)
        brX = nXSize - 1;

    if (brY < 0)
        brY = 0;
    if (brY >= nYSize)
        brY = nYSize - 1;
}

void PI_RECT::Set(long ltx, long lty, long rtx, long rty, long lbx, long lby, long rbx, long rby)
{
    ltX = ltx;
    ltY = lty;
    rtX = rtx;
    rtY = rty;
    brX = rbx;
    brY = rby;
    blX = lbx;
    blY = lby;
}

void PI_RECT::Set(Ipvm::Point32s2 LT, Ipvm::Point32s2 RT, Ipvm::Point32s2 LB, Ipvm::Point32s2 RB)
{
    ltX = LT.m_x;
    ltY = LT.m_y;
    rtX = RT.m_x;
    rtY = RT.m_y;
    brX = RB.m_x;
    brY = RB.m_y;
    blX = LB.m_x;
    blY = LB.m_y;
}

Ipvm::Rect32s PI_RECT::GetExtRect() const
{
    Ipvm::Rect32s rtRect(min(ltX, blX), min(ltY, rtY), max(rtX, brX) + 1, max(blY, brY) + 1);
    rtRect.NormalizeRect();

    return rtRect;
}

Ipvm::Rect32s PI_RECT::GetMidRect() const
{
    Ipvm::Rect32s rtRect((int)(0.5f * (ltX + blX) + 0.5f), (int)(0.5f * (ltY + rtY) + 0.5f),
        (int)(0.5f * (rtX + brX) + 1.5f), (int)(0.5f * (blY + brY) + 1.5f));
    rtRect.NormalizeRect();

    return rtRect;
}

Ipvm::Point32r2 PI_RECT::GetCenter() const
{
    Ipvm::Point32r2 fptCenter;
    fptCenter.m_x = float(ltX + rtX + blX + brX) / 4.0f;
    fptCenter.m_y = float(ltY + rtY + blY + brY) / 4.0f;

    return fptCenter;
}

Ipvm::Point32s2 PI_RECT::GetLTPoint() const
{
    return Ipvm::Point32s2(ltX, ltY);
}

Ipvm::Point32s2 PI_RECT::GetRTPoint() const
{
    return Ipvm::Point32s2(rtX, rtY);
}

Ipvm::Point32s2 PI_RECT::GetLBPoint() const
{
    return Ipvm::Point32s2(blX, blY);
}

Ipvm::Point32s2 PI_RECT::GetRBPoint() const
{
    return Ipvm::Point32s2(brX, brY);
}

BOOL PI_RECT::SetLTPoint(const Ipvm::Point32s2& ptLT)
{
    ltX = ptLT.m_x;
    ltY = ptLT.m_y;
    return TRUE;
}

BOOL PI_RECT::SetRTPoint(const Ipvm::Point32s2& ptRT)
{
    rtX = ptRT.m_x;
    rtY = ptRT.m_y;

    return TRUE;
}

BOOL PI_RECT::SetLBPoint(const Ipvm::Point32s2& ptLB)
{
    blX = ptLB.m_x;
    blY = ptLB.m_y;

    return TRUE;
}

BOOL PI_RECT::SetRBPoint(const Ipvm::Point32s2& ptRB)
{
    brX = ptRB.m_x;
    brY = ptRB.m_y;
    return TRUE;
}
//}}

Ipvm::Rect32s PI_RECT::CreateExternalMinimumRect() const
{
    int32_t left = min(blX, ltX);
    int32_t top = min(ltY, rtY);
    int32_t right = max(brX, rtX);
    int32_t bottom = max(blY, brY);

    return Ipvm::Rect32s(left, top, right + 1, bottom + 1);
}