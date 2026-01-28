//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ResultLayerPre.h"

//CPP_2_________________________________ This project's headers
#include "SpecRoi.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
bool ResultUserROI::operator==(const ResultUserROI& object) const
{
    if (m_operation != object.m_operation)
        return false;
    if (m_type != object.m_type)
        return false;
    if (m_rect != object.m_rect)
        return false;
    if (m_polygon != object.m_polygon)
        return false;
    if (m_preparedObjectName != object.m_preparedObjectName)
        return false;
    if (m_preparedMaskDilateInUm != object.m_preparedMaskDilateInUm)
        return false;
    if (m_preparedROIExpandX_um != object.m_preparedROIExpandX_um)
        return false;
    if (m_preparedROIExpandY_um != object.m_preparedROIExpandY_um)
        return false;

    return true;
}

bool ResultUserROI::operator!=(const ResultUserROI& object) const
{
    return !(*this == object);
}

ResultLayerPre::ResultLayerPre()
{
}

ResultLayerPre::~ResultLayerPre()
{
}

void ResultLayerPre::Add(const Ipvm::Point32r2& px2um, const Ipvm::Point32r2& imageCenter, const SpecRoi& roi)
{
    ResultUserROI addRoi;
    addRoi.m_operation = roi.m_operation;
    addRoi.m_type = roi.getType();

    switch (roi.getType())
    {
        case UserRoiType::PreparedMask:
            addRoi.m_preparedObjectName = roi.m_preparedObjectName;
            addRoi.m_preparedMaskDilateInUm = roi.m_preparedMaskDilateInUm;
            addRoi.m_preparedROIExpandX_um = roi.m_preparedROIExpandX_um;
            addRoi.m_preparedROIExpandY_um = roi.m_preparedROIExpandY_um;
            break;
        case UserRoiType::Ellipse_32f:
        case UserRoiType::Rect:
            if (1)
            {
                Ipvm::Point32r2 center;
                center.m_x = imageCenter.m_x + roi.m_rect.CenterPoint().m_x / px2um.m_x;
                center.m_y = imageCenter.m_y + roi.m_rect.CenterPoint().m_y / px2um.m_y;

                Ipvm::Rect32r rect;
                rect.m_left = center.m_x - roi.m_rect.Width() / px2um.m_x * 0.5f;
                rect.m_top = center.m_y - roi.m_rect.Height() / px2um.m_y * 0.5f;
                rect.m_right = center.m_x + roi.m_rect.Width() / px2um.m_x * 0.5f;
                rect.m_bottom = center.m_y + roi.m_rect.Height() / px2um.m_y * 0.5f;

                addRoi.m_rect = Ipvm::Conversion::ToRect32s(rect);
            }
            break;
        case UserRoiType::Polygon_32f:
            addRoi.m_polygon.clear();
            for (long vertex = 0; vertex < roi.m_polygon.GetVertexNum(); vertex++)
            {
                auto& specPoint = roi.m_polygon.GetVertices()[vertex];
                Ipvm::Point32r2 newPoint;
                newPoint.m_x = imageCenter.m_x + specPoint.m_x / px2um.m_x;
                newPoint.m_y = imageCenter.m_y + specPoint.m_y / px2um.m_y;

                addRoi.m_polygon.push_back(Ipvm::Conversion::ToPoint32s2(newPoint));
            }
            break;

        default:
            ASSERT(!_T("??"));
    }

    m_userROIs.push_back(addRoi);
}

bool ResultLayerPre::operator==(const ResultLayerPre& object) const
{
    if (m_userROIs.size() != object.m_userROIs.size())
        return false;

    for (long index = 0; index < long(m_userROIs.size()); index++)
    {
        if (m_userROIs[index].m_type != object.m_userROIs[index].m_type)
        {
            return false;
        }

        if (m_userROIs[index].m_type == UserRoiType::Polygon_32f)
        {
            if (m_userROIs[index].m_polygon != object.m_userROIs[index].m_polygon)
                return false;
        }
        else
        {
            if (m_userROIs[index].m_rect != object.m_userROIs[index].m_rect)
                return false;
        }
    }

    return true;
}

bool ResultLayerPre::operator!=(const ResultLayerPre& object) const
{
    return !(*this == object);
}
