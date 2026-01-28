//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceLayerRoi.h"

//CPP_2_________________________________ This project's headers
#include "VisionReusableMemory.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SurfaceLayerRoi::SurfaceLayerRoi(VisionReusableMemory* memory)
    : m_memory(memory)
{
}

SurfaceLayerRoi::~SurfaceLayerRoi()
{
}

bool SurfaceLayerRoi::Ready(const Ipvm::Point32r2& bodyCenter)
{
    m_bodyCenter = bodyCenter;

    return true;
}

void SurfaceLayerRoi::Reset()
{
    m_objects_ellipse.clear();
    m_objects_quardrangle.clear();
}

void SurfaceLayerRoi::Add(const Ipvm::EllipseEq32r& object)
{
    m_objects_ellipse.push_back(object);
}

void SurfaceLayerRoi::Add(const Ipvm::Quadrangle32r& object)
{
    m_objects_quardrangle.push_back(object);
}

void SurfaceLayerRoi::Add(const Ipvm::Rect32s& object)
{
    Add(Ipvm::Conversion::ToQuadrangle32r(object));
}

bool SurfaceLayerRoi::AddToMask(long dilateX, long dilateY, Ipvm::Image8u& io_mask) const
{
    for (auto object : m_objects_ellipse)
    {
        object.m_xradius += dilateX;
        object.m_yradius += dilateY;

        if (Ipvm::ImageProcessing::Fill(object, 255, io_mask) != Ipvm::Status::e_ok)
        {
            return false;
        }
    }

    for (auto object : m_objects_quardrangle)
    {
        object.m_ltX -= dilateX;
        object.m_ltY -= dilateY;
        object.m_rtX += dilateX;
        object.m_rtY -= dilateY;
        object.m_lbX -= dilateX;
        object.m_lbY += dilateY;
        object.m_rbX += dilateX;
        object.m_rbY += dilateY;

        if (Ipvm::ImageProcessing::Fill(object, 255, io_mask) != Ipvm::Status::e_ok)
        {
            return false;
        }
    }

    return true;
}

bool SurfaceLayerRoi::SubtractToMask(long dilateX, long dilateY, Ipvm::Image8u& io_mask) const
{
    for (auto object : m_objects_ellipse)
    {
        object.m_xradius += dilateX;
        object.m_yradius += dilateY;

        if (Ipvm::ImageProcessing::Fill(object, 0, io_mask) != Ipvm::Status::e_ok)
        {
            return false;
        }
    }

    for (auto object : m_objects_quardrangle)
    {
        object.m_ltX -= dilateX;
        object.m_ltY -= dilateY;
        object.m_rtX += dilateX;
        object.m_rtY -= dilateY;
        object.m_lbX -= dilateX;
        object.m_lbY += dilateY;
        object.m_rbX += dilateX;
        object.m_rbY += dilateY;

        if (Ipvm::ImageProcessing::Fill(object, 0, io_mask) != Ipvm::Status::e_ok)
        {
            return false;
        }
    }

    return true;
}

Ipvm::Point32r2 SurfaceLayerRoi::makeSurfacePos(
    const Ipvm::Point32r2& point, const Ipvm::Point32r2& bodyCenter, float bodyAngle_deg)
{
    float screenCenX = m_memory->GetInspImageSizeX() * 0.5f;
    float screenCenY = m_memory->GetInspImageSizeY() * 0.5f;

    float co = (float)cos(bodyAngle_deg * DEF_DEG_TO_RAD);
    float si = (float)sin(bodyAngle_deg * DEF_DEG_TO_RAD);

    float x = point.m_x - bodyCenter.m_x;
    float y = point.m_y - bodyCenter.m_y;

    Ipvm::Point32r2 dst;
    dst.m_x = (x * co - y * si) + screenCenX;
    dst.m_y = (x * si + y * co) + screenCenY;

    return dst;
}
