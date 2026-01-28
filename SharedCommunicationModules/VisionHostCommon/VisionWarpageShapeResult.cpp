//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionWarpageShapeResult.h"

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
VisionWarpageShapeResult::VisionWarpageShapeResult()
{
    Init();
}

VisionWarpageShapeResult::VisionWarpageShapeResult(
    CString strSign, CString strShape, double R, double B4_um, double B5_um, double angle_deg)
{
    SetResult(strSign, strShape, R, B4_um, B5_um, angle_deg);
}

VisionWarpageShapeResult::~VisionWarpageShapeResult()
{
}

void VisionWarpageShapeResult::Init()
{
    m_sign.Empty();
    m_shape.Empty();
    m_R = 0.;
    m_B4_um = 0.;
    m_B5_um = 0.;
    m_angle_deg = 0.;
}

void VisionWarpageShapeResult::GetResult(
    CString& strSign, CString& strShape, double& R, double& B4_um, double& B5_um, double& angle_deg)
{
    strSign = m_sign;
    strShape = m_shape;
    R = m_R;
    B4_um = m_B4_um;
    B5_um = m_B5_um;
    angle_deg = m_angle_deg;
}

void VisionWarpageShapeResult::SetResult(
    CString strSign, CString strShape, double R, double B4_um, double B5_um, double angle_deg)
{
    m_sign = strSign;
    m_shape = strShape;
    m_R = R;
    m_B4_um = B4_um;
    m_B5_um = B5_um;
    m_angle_deg = angle_deg;
}

void VisionWarpageShapeResult::SetResult(VisionWarpageShapeResult visionWarpageShapeResult)
{
    m_sign = visionWarpageShapeResult.m_sign;
    m_shape = visionWarpageShapeResult.m_shape;
    m_R = visionWarpageShapeResult.m_R;
    m_B4_um = visionWarpageShapeResult.m_B4_um;
    m_B5_um = visionWarpageShapeResult.m_B5_um;
    m_angle_deg = visionWarpageShapeResult.m_angle_deg;
}

CArchive& operator<<(CArchive& ar, const VisionWarpageShapeResult& src)
{
    return ar << src.m_sign << src.m_shape << src.m_R << src.m_B4_um << src.m_B5_um << src.m_angle_deg;
}

CArchive& operator>>(CArchive& ar, VisionWarpageShapeResult& dst)
{
    return ar >> dst.m_sign >> dst.m_shape >> dst.m_R >> dst.m_B4_um >> dst.m_B5_um >> dst.m_angle_deg;
}
