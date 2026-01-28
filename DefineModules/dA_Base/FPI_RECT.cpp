//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FPI_RECT.h"

//CPP_2_________________________________ This project's headers
#include "PI_RECT.h"
#include "iDataType.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
//
//{{ FPI_RECT
FPI_RECT::FPI_RECT()
{
    fptLT = Ipvm::Point32r2(0.f, 0.f);
    fptRT = Ipvm::Point32r2(0.f, 0.f);
    fptRB = Ipvm::Point32r2(0.f, 0.f);
    fptLB = Ipvm::Point32r2(0.f, 0.f);
}

FPI_RECT::FPI_RECT(Ipvm::Point32r2 _fptLT, Ipvm::Point32r2 _fptRT, Ipvm::Point32r2 _fptLB, Ipvm::Point32r2 _fptRB)
{
    fptLT = _fptLT;
    fptRT = _fptRT;
    fptLB = _fptLB;
    fptRB = _fptRB;
}

FPI_RECT& FPI_RECT::operator=(const FPI_RECT& sfpiRect)
{
    fptLT = sfpiRect.fptLT;
    fptRT = sfpiRect.fptRT;
    fptLB = sfpiRect.fptLB;
    fptRB = sfpiRect.fptRB;

    return *this;
}

PI_RECT FPI_RECT::GetSPI_RECT() const
{
    PI_RECT srt;
    srt.ltX = (long)(fptLT.m_x + .5f);
    srt.ltY = (long)(fptLT.m_y + .5f);
    srt.rtX = (long)(fptRT.m_x + .5f);
    srt.rtY = (long)(fptRT.m_y + .5f);
    srt.blX = (long)(fptLB.m_x + .5f);
    srt.blY = (long)(fptLB.m_y + .5f);
    srt.brX = (long)(fptRB.m_x + .5f);
    srt.brY = (long)(fptRB.m_y + .5f);

    return srt;
}

void FPI_RECT::SetSPI_RECT(PI_RECT spiRect) //kircheis_EdgePeeling
{
    fptLT = Ipvm::Point32r2((float)spiRect.ltX, (float)spiRect.ltY);
    fptRT = Ipvm::Point32r2((float)spiRect.rtX, (float)spiRect.rtY);
    fptLB = Ipvm::Point32r2((float)spiRect.blX, (float)spiRect.blY);
    fptRB = Ipvm::Point32r2((float)spiRect.brX, (float)spiRect.brY);
}

FPI_RECT& FPI_RECT::Move(float x, float y)
{
    fptLT.m_x += x;
    fptLT.m_y += y;
    fptRT.m_x += x;
    fptRT.m_y += y;
    fptLB.m_x += x;
    fptLB.m_y += y;
    fptRB.m_x += x;
    fptRB.m_y += y;

    return *this;
}

Ipvm::Rect32r FPI_RECT::GetFRect() const
{
    float dx_top, dy_top;
    dx_top = fptLT.m_x - fptRT.m_x;
    dy_top = fptLT.m_y - fptRT.m_y;

    float dx_left, dy_left;
    dx_left = fptLT.m_x - fptLB.m_x;
    dy_left = fptLT.m_y - fptLB.m_y;

    float xdist = static_cast<float>(sqrt(dx_top * dx_top + dy_top * dy_top));
    float ydist = static_cast<float>(sqrt(dx_left * dx_left + dy_left * dy_left));

    float xcen = (fptLT.m_x + fptRT.m_x + fptLB.m_x + fptRB.m_x) / 4.0f;
    float ycen = (fptLT.m_y + fptRT.m_y + fptLB.m_y + fptRB.m_y) / 4.0f;

    Ipvm::Rect32r frt;
    frt.m_left = xcen - (xdist / 2.0f);
    frt.m_right = xcen + (xdist / 2.0f);
    frt.m_top = ycen - (ydist / 2.0f);
    frt.m_bottom = ycen + (ydist / 2.0f);
    frt.NormalizeRect();

    return frt;
}

Ipvm::Rect32s FPI_RECT::GetRoughRect() const
{
    Ipvm::Rect32s RoughRect;

    RoughRect.m_left = (int)min(min(fptLT.m_x, fptLB.m_x), min(fptRT.m_x, fptRB.m_x));
    RoughRect.m_top = (int)min(min(fptLT.m_y, fptRT.m_y), min(fptLB.m_y, fptRB.m_y));
    RoughRect.m_right = (int)max(max(fptLT.m_x, fptLB.m_x), max(fptRT.m_x, fptRB.m_x));
    RoughRect.m_bottom = (int)max(max(fptLT.m_y, fptRT.m_y), max(fptLB.m_y, fptRB.m_y));

    return RoughRect;
}

Ipvm::Rect32s FPI_RECT::GetCRect() const
{
    return Ipvm::Conversion::ToRect32s(GetFRect());
}

Ipvm::Rect32r FPI_RECT::GetExtFRect() const
{
    float fLeft = fptLT.m_x;
    if (fptLB.m_x < fLeft)
        fLeft = fptLB.m_x;
    if (fptRB.m_x < fLeft)
        fLeft = fptRB.m_x;
    if (fptRT.m_x < fLeft)
        fLeft = fptRT.m_x;

    float fTop = fptLT.m_y;
    if (fptLB.m_y < fTop)
        fTop = fptLB.m_y;
    if (fptRB.m_y < fTop)
        fTop = fptRB.m_y;
    if (fptRT.m_y < fTop)
        fTop = fptRT.m_y;

    float fRight = fptLT.m_x;
    if (fptLB.m_x > fRight)
        fRight = fptLB.m_x;
    if (fptRB.m_x > fRight)
        fRight = fptRB.m_x;
    if (fptRT.m_x > fRight)
        fRight = fptRT.m_x;

    float fBottom = fptLT.m_y;
    if (fptLB.m_y > fBottom)
        fBottom = fptLB.m_y;
    if (fptRB.m_y > fBottom)
        fBottom = fptRB.m_y;
    if (fptRT.m_y > fBottom)
        fBottom = fptRT.m_y;

    return Ipvm::Rect32r(fLeft, fTop, fRight, fBottom);
}

Ipvm::Rect32s FPI_RECT::GetExtCRect() const
{
    return Ipvm::Conversion::ToRect32s(GetExtFRect());
}

Ipvm::Point32r2 FPI_RECT::GetCenter() const
{
    Ipvm::Point32r2 fptCenter;
    fptCenter.m_x = (fptLT.m_x + fptRT.m_x + fptRB.m_x + fptLB.m_x) / 4.0f;
    fptCenter.m_y = (fptLT.m_y + fptRT.m_y + fptRB.m_y + fptLB.m_y) / 4.0f;

    return fptCenter;
}

FPI_RECT FPI_RECT::Rotate(double dRadian) const
{
    FPI_RECT srtTemp(fptLT, fptRT, fptLB, fptRB);
    //1) 원점 이동.
    //srtTemp.Move((int)(-GetCenter().m_x), (int)(-GetCenter().m_y)); // 원점 이동.
    srtTemp.Move((float)(-GetCenter().m_x), (float)(-GetCenter().m_y)); // 원점 이동.

    //2) 회전.
    double cosine = cos(-dRadian);
    double sine = sin(-dRadian);

    FPI_RECT srtTemp2 = srtTemp;
    srtTemp2.fptLT.m_x = (float)(srtTemp.fptLT.m_x * cosine - srtTemp.fptLT.m_y * sine);
    srtTemp2.fptLT.m_y = (float)(srtTemp.fptLT.m_x * sine + srtTemp.fptLT.m_y * cosine);
    srtTemp2.fptRT.m_x = (float)(srtTemp.fptRT.m_x * cosine - srtTemp.fptRT.m_y * sine);
    srtTemp2.fptRT.m_y = (float)(srtTemp.fptRT.m_x * sine + srtTemp.fptRT.m_y * cosine);
    srtTemp2.fptLB.m_x = (float)(srtTemp.fptLB.m_x * cosine - srtTemp.fptLB.m_y * sine);
    srtTemp2.fptLB.m_y = (float)(srtTemp.fptLB.m_x * sine + srtTemp.fptLB.m_y * cosine);
    srtTemp2.fptRB.m_x = (float)(srtTemp.fptRB.m_x * cosine - srtTemp.fptRB.m_y * sine);
    srtTemp2.fptRB.m_y = (float)(srtTemp.fptRB.m_x * sine + srtTemp.fptRB.m_y * cosine);

    //3) 다시 본래위치로 이동.
    //srtTemp2.Move((int)(GetCenter().m_x), (int)(GetCenter().m_y));
    srtTemp2.Move((float)(GetCenter().m_x), (float)(GetCenter().m_y));

    return srtTemp2;
}

FPI_RECT FPI_RECT::Rotate(double dRadian, Ipvm::Point32r2 fptCenter) const
{
    Ipvm::Point32r2 srcfpt[4];
    Ipvm::Point32r2 dstfpt[4];
    double co, si;

    // 상대적인 거리 계산.
    srcfpt[0].m_x = fptLT.m_x - fptCenter.m_x;
    srcfpt[0].m_y = fptLT.m_y - fptCenter.m_y;

    srcfpt[1].m_x = fptRT.m_x - fptCenter.m_x;
    srcfpt[1].m_y = fptRT.m_y - fptCenter.m_y;

    srcfpt[2].m_x = fptLB.m_x - fptCenter.m_x;
    srcfpt[2].m_y = fptLB.m_y - fptCenter.m_y;

    srcfpt[3].m_x = fptRB.m_x - fptCenter.m_x;
    srcfpt[3].m_y = fptRB.m_y - fptCenter.m_y;

    co = cos(-dRadian);
    si = sin(-dRadian);

    for (int i = 0; i < 4; i++)
    {
        dstfpt[i].m_x = (float)((srcfpt[i].m_x * co - srcfpt[i].m_y * si) + fptCenter.m_x);
        dstfpt[i].m_y = (float)((srcfpt[i].m_x * si + srcfpt[i].m_y * co) + fptCenter.m_y);
        //dstfpt[i].m_x = (srcfpt[i].m_x*co - srcfpt[i].m_y*si) + fptCenter.m_x;
        //dstfpt[i].m_y = (srcfpt[i].m_x*si + srcfpt[i].m_y*co) + fptCenter.m_y;
    }

    return FPI_RECT(dstfpt[0], dstfpt[1], dstfpt[2], dstfpt[3]);
}

void SwapPoint(Ipvm::Point32r2& fpt1, Ipvm::Point32r2& fpt2)
{
    Ipvm::Point32r2 fptTemp = fpt1;
    fpt1 = fpt2;
    fpt2 = fptTemp;
}

void FPI_RECT::NormalizeRect()
{
    // LeftTop
    if (fptLT.m_x > fptRT.m_x)
        SwapPoint(fptLT, fptRT);
    if (fptLT.m_x > fptRB.m_x && fptLT.m_y > fptRB.m_y)
        SwapPoint(fptLT, fptRB);
    if (fptLT.m_y > fptLB.m_y)
        SwapPoint(fptLT, fptLB);

    // RightTop
    if (fptRT.m_x < fptLT.m_x)
        SwapPoint(fptRT, fptLT);
    if (fptRT.m_x < fptLB.m_x && fptRT.m_y > fptLB.m_y)
        SwapPoint(fptRT, fptLB);
    if (fptRT.m_y > fptRB.m_y)
        SwapPoint(fptRT, fptRB);

    // RigthBottom
    if (fptRB.m_x < fptLT.m_x && fptRB.m_y < fptLT.m_y)
        SwapPoint(fptRB, fptLT);
    if (fptRB.m_x < fptLB.m_x)
        SwapPoint(fptRB, fptLB);
    if (fptRB.m_y < fptRT.m_y)
        SwapPoint(fptRB, fptRT);

    // LeftBottom
    if (fptLB.m_x > fptRT.m_x && fptLB.m_y < fptRT.m_y)
        SwapPoint(fptLB, fptRT);
    if (fptLB.m_x > fptRB.m_x)
        SwapPoint(fptLB, fptRB);
    if (fptLB.m_y < fptLT.m_y)
        SwapPoint(fptLB, fptLT);
}
//}}
