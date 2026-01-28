//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionAlignResult.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionAlignResult::VisionAlignResult()
{
    Init(Ipvm::Point32r2(0.f, 0.f));
}

VisionAlignResult::~VisionAlignResult() //kircheis_ClearBlob
{
    const Ipvm::Point32r2 ptCenter(0.f, 0.f);
    Init(ptCenter);
}

void VisionAlignResult::Init(const Ipvm::Point32r2& center)
{
    m_analysis_packageOffsetX_um = 0.f;
    m_analysis_packageOffsetY_um = 0.f;
    m_analysis_histogramZ.clear();

    bAvailable = FALSE;
    m_center = center;
    m_angle_rad = 0.f;
    fCosVal = 1.f;
    fSinVal = 0.f;
    fptLT = Ipvm::Point32r2(0.f, 0.f);
    fptRT = Ipvm::Point32r2(0.f, 0.f);
    fptLB = Ipvm::Point32r2(0.f, 0.f);
    fptRB = Ipvm::Point32r2(0.f, 0.f);

    vecfParallelismDistLeft.clear();
    vecfParallelismDistRight.clear();
    vecfParallelismDistTop.clear();
    vecfParallelismDistBottom.clear();

    m_ballConstrasts.clear();
    m_ballConstrastROIs.clear();
    m_ballWidths_px.clear();
    m_ballWidthCenterPos_px.clear();
}

void VisionAlignResult::SetAngle(float _fAngle_rad)
{
    bAvailable = TRUE;

    m_angle_rad = _fAngle_rad;

    fCosVal = float(cos(_fAngle_rad));
    fSinVal = float(sin(_fAngle_rad));
}

void VisionAlignResult::SetBodyAlign_AngleZero(const Ipvm::Point32r2& shift)
{
    const Ipvm::Point32r2 origin = m_center;
    float rotateAngle_rad = -m_angle_rad;

    m_center.m_x += shift.m_x;
    m_center.m_y += shift.m_y;
    m_angle_rad = 0.f;
    fCosVal = float(cos(m_angle_rad));
    fSinVal = float(sin(m_angle_rad));

    fptLT = rotate(origin, rotateAngle_rad, fptLT) + shift;
    fptRT = rotate(origin, rotateAngle_rad, fptRT) + shift;
    fptLB = rotate(origin, rotateAngle_rad, fptLB) + shift;
    fptRB = rotate(origin, rotateAngle_rad, fptRB) + shift;
    fptLowtopLT = rotate(origin, rotateAngle_rad, fptLowtopLT) + shift;
    fptLowtopRT = rotate(origin, rotateAngle_rad, fptLowtopRT) + shift;
}

Ipvm::Rect32r VisionAlignResult::getBodyRect() const
{
    //BodyAlign Result
    Ipvm::Rect32r retValue;
    retValue.m_left = (fptLT.m_x + fptLB.m_x) * 0.5f;
    retValue.m_top = (fptLT.m_y + fptRT.m_y) * 0.5f;
    retValue.m_right = (fptRT.m_x + fptRB.m_x) * 0.5f;
    retValue.m_bottom = (fptLB.m_y + fptRB.m_y) * 0.5f;

    return retValue;
}

Ipvm::Rect32s VisionAlignResult::getBodyRect32s() const
{
    return {static_cast<int>(((fptLT.m_x + fptLB.m_x) / 2) + 0.5f),
        static_cast<int>(((fptLT.m_y + fptRT.m_y) / 2) + 0.5f), static_cast<int>(((fptRT.m_x + fptRB.m_x) / 2) + 1.5f),
        static_cast<int>(((fptLB.m_y + fptRB.m_y) / 2) + 1.5f)};
}

Ipvm::Point32r2 VisionAlignResult::rotate(const Ipvm::Point32r2& origin, float angle_rad, const Ipvm::Point32r2& object)
{
    float cosVal = float(cos(angle_rad));
    float sinVal = float(sin(angle_rad));

    float fX = object.m_x - origin.m_x;
    float fY = object.m_y - origin.m_y;

    Ipvm::Point32r2 retValue;
    retValue.m_x = cosVal * fX - sinVal * fY + origin.m_x;
    retValue.m_y = sinVal * fX + cosVal * fY + origin.m_y;

    return retValue;
}
