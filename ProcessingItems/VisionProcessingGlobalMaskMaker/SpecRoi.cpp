//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SpecRoi.h"

//CPP_2_________________________________ This project's headers
#include "ResultLayerPre.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SpecRoi::SpecRoi()
    : m_type(UserRoiType::Rect)
    , m_polygonPointCount(3)
    , m_preparedMaskDilateInUm(0.f)
    , m_preparedROIExpandX_um(0.f)
    , m_preparedROIExpandY_um(0.f)
    , m_operation(OperationType::Plus)
{
    m_rect = Ipvm::Rect32r(-1000.f, -1000.f, 1000.f, 1000.f);
    setPolygonPointCount(3);
}

SpecRoi::~SpecRoi()
{
}

void SpecRoi::Init()
{
    m_operation = OperationType::Plus;
    m_type = UserRoiType::Rect;
    m_polygonPointCount = 3;
    m_preparedObjectName = _T("");
    m_preparedMaskDilateInUm = 0.f;
    m_preparedROIExpandX_um = 0.f;
    m_preparedROIExpandY_um = 0.f;

    setPolygonPointCount(3);
}

BOOL SpecRoi::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;

    if (!db[_T("Version")].Link(bSave, version))
        version = 0;
    if (!db[_T("{2F01F9B2-F11B-44BE-9B90-371A0FBB6A31}")].Link(bSave, (long&)m_operation))
        m_operation = OperationType::Plus;
    if (!db[_T("{3D02489C-4FFA-4FF5-81A7-90D133C8B0E1}")].Link(bSave, (long&)m_type))
        m_type = UserRoiType::Rect;
    if (!db[_T("{6C36A97D-FCBD-4D74-827D-9617A795C4E4}")].Link(bSave, m_polygonPointCount))
        m_polygonPointCount = 3;
    if (!db[_T("{4B70DF20-1B6C-413A-ACA2-D73F0993D687}")].Link(bSave, m_rect.m_left))
        m_rect.m_left = -1000.f;
    if (!db[_T("{05ABFE86-857C-4A91-9C96-784C019285C6}")].Link(bSave, m_rect.m_top))
        m_rect.m_top = -1000.f;
    if (!db[_T("{92F12DB8-B605-4B60-B828-7EE61753FD89}")].Link(bSave, m_rect.m_right))
        m_rect.m_right = 1000.f;
    if (!db[_T("{A3F8AB55-579C-4E5B-9253-761E4F359880}")].Link(bSave, m_rect.m_bottom))
        m_rect.m_bottom = 1000.f;
    if (!db[_T("{18DACF78-9DB6-4DC8-A320-3D12001B56C8}")].Link(bSave, m_preparedObjectName))
        m_preparedObjectName = _T("");

    if (version < 1)
    {
        // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
        return FALSE;
    }
    else
    {
        if (!db[_T("{954F6339-9333-4754-ABFD-D8C6115C295F}")].Link(bSave, m_preparedROIExpandX_um))
            m_preparedROIExpandX_um = 0.f;
        if (!db[_T("{11EAEF44-04A5-4C35-AC8D-DE4BD45CD9C3}")].Link(bSave, m_preparedROIExpandY_um))
            m_preparedROIExpandY_um = 0.f;
        if (!db[_T("{D49F66F8-264D-4638-9E3E-D5DF8A477323}")].Link(bSave, m_preparedMaskDilateInUm))
            m_preparedMaskDilateInUm = 0.f;
    }

    auto& poly_db = db[_T("{506629FF-884A-4737-B973-FCCAD83A338A}")];
    long vertexCount = static_cast<long>(m_polygon.GetVertexNum());
    if (!poly_db[_T("count")].Link(bSave, vertexCount))
        vertexCount = 0;

    if (!bSave)
    {
        m_polygon.SetVertexNum(vertexCount);
    }

    for (long n = 0; n < vertexCount; n++)
    {
        auto& sub_db = poly_db.GetSubDBFmt(_T("P%d"), n + 1);

        sub_db[_T("X")].Link(bSave, m_polygon.GetVertices()[n].m_x);
        sub_db[_T("Y")].Link(bSave, m_polygon.GetVertices()[n].m_y);
    }

    return TRUE;
}

void SpecRoi::Set(
    const Ipvm::Point32r2& px2um, const Ipvm::Point32r2& imageCenter, const ResultLayerPre& layerPre, long userRoiIndex)
{
    const auto& roi = layerPre.m_userROIs[userRoiIndex];
    switch (roi.m_type)
    {
        case UserRoiType::Ellipse_32f:
        case UserRoiType::Rect:
            if (1)
            {
                Ipvm::Point32r2 center;
                center.m_x = (roi.m_rect.CenterPoint().m_x - imageCenter.m_x) * px2um.m_x;
                center.m_y = (roi.m_rect.CenterPoint().m_y - imageCenter.m_y) * px2um.m_y;

                Ipvm::Rect32r& rect = m_rect;
                rect.m_left = center.m_x - roi.m_rect.Width() * px2um.m_x * 0.5f;
                rect.m_top = center.m_y - roi.m_rect.Height() * px2um.m_y * 0.5f;
                rect.m_right = center.m_x + roi.m_rect.Width() * px2um.m_x * 0.5f;
                rect.m_bottom = center.m_y + roi.m_rect.Height() * px2um.m_y * 0.5f;
            }
            break;
        case UserRoiType::Polygon_32f:
            m_polygon.SetVertexNum(long(roi.m_polygon.size()));
            for (long vertex = 0; vertex < long(roi.m_polygon.size()); vertex++)
            {
                auto& specPoint = roi.m_polygon[vertex];
                Ipvm::Point32r2& newPoint = m_polygon.GetVertices()[vertex];
                newPoint.m_x = (specPoint.m_x - imageCenter.m_x) * px2um.m_x;
                newPoint.m_y = (specPoint.m_y - imageCenter.m_y) * px2um.m_y;
            }
            break;
        default:
            ASSERT(!_T("??"));
    }
}

UserRoiType SpecRoi::getType() const
{
    return m_type;
}

void SpecRoi::setType(UserRoiType type)
{
    m_type = type;
}

long SpecRoi::getPolygonPointCount() const
{
    return m_polygonPointCount;
}

void SpecRoi::setPolygonPointCount(long count)
{
    m_polygonPointCount = max(3, count);

    auto& polygon = m_polygon;

    long insertVertexStart = static_cast<long>(polygon.GetVertexNum());
    polygon.SetVertexNum(m_polygonPointCount);

    for (long point = insertVertexStart; point < m_polygonPointCount; point++)
    {
        float angle = 360.f * point / m_polygonPointCount;

        float co = (float)cos(angle * DEF_DEG_TO_RAD);
        float si = (float)sin(angle * DEF_DEG_TO_RAD);

        float x = 1000.f;
        float y = 1000.f;

        polygon.GetVertices()[point].m_x = (x * co - y * si);
        polygon.GetVertices()[point].m_y = (x * si + y * co);
    }
}
