//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Para_PolygonRects.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL Para_PolygonRects::SPolygonRectNamePair::IsRectIntersectWithPolygon(
    const VisionScale& scale, const Ipvm::Point32r2& imageCenter, const Ipvm::Rect32s& rtObj) const
{
    if (vecptPolygon_BCU.size() == 0)
    {
        return FALSE;
    }

    if (nPolygonType < 2)
    {
        const Ipvm::Point32r2* pPt_BCU = &(vecptPolygon_BCU[0]);

        Ipvm::Rect32s rtDA = scale.convert_BCUToPixel(
            Ipvm::Rect32r(pPt_BCU[0].m_x, pPt_BCU[0].m_y, pPt_BCU[1].m_x, pPt_BCU[1].m_y), imageCenter);
        Ipvm::Rect32s rtInter;

        if (rtInter.IntersectRect(rtDA, rtObj))
        {
            return TRUE;
        }
    }
    else
    {
        std::vector<Ipvm::Point32s2> points;
        points.resize(vecptPolygon_BCU.size());

        for (long n = 0; n < long(vecptPolygon_BCU.size()); n++)
        {
            points[n] = scale.convert_BCUToPixel(vecptPolygon_BCU[n], imageCenter);
        }
        Ipvm::Point32s2 ptObj[4]
            = {Ipvm::Point32s2(rtObj.m_left, rtObj.m_top), Ipvm::Point32s2(rtObj.m_right, rtObj.m_top),
                Ipvm::Point32s2(rtObj.m_left, rtObj.m_bottom), Ipvm::Point32s2(rtObj.m_right, rtObj.m_bottom)};
        for (int i = 0; i < 4; i++)
        {
            if (CPI_Geometry::bPtIsInsidePolygon(&points[0], nPolygonType, ptObj[i]))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

Para_PolygonRects::Para_PolygonRects()
{
}

Para_PolygonRects::Para_PolygonRects(const Para_PolygonRects& object)
{
    *this = object;
}

Para_PolygonRects::~Para_PolygonRects()
{
}

void Para_PolygonRects::Init()
{
    m_items.clear();
}

bool Para_PolygonRects::IsValid(LPCTSTR maskName) const
{
    for (auto item : m_items)
    {
        if (item.strName == maskName)
        {
            return true;
        }
    }

    return false;
}

long Para_PolygonRects::FindIndex(LPCTSTR maskName) const
{
    for (long index = 0; index < long(m_items.size()); index++)
    {
        if (m_items[index].strName == maskName)
        {
            return index;
        }
    }

    return -1;
}

BOOL Para_PolygonRects::LinkDataBase(BOOL save, CiDataBase& db)
{
    long version = 1;
    long count = (long)m_items.size();

    if (!db[_T("Version")].Link(save, version))
        version = 0;
    if (!db[_T("count")].Link(save, count))
        count = 0;

    if (!save)
    {
        m_items.resize(count);
    }

    for (int index = 0; index < count; index++)
    {
        auto& info = m_items[index];
        auto& sub_db = db.GetSubDBFmt(_T("Item%d"), index);

        long nSize = (long)info.vecptPolygon_BCU.size();

        if (!sub_db[_T("Name")].Link(save, info.strName))
            info.strName = "";
        if (!sub_db[_T("PolygonType")].Link(save, info.nPolygonType))
            info.nPolygonType = 0;
        if (!sub_db[_T("PolygonSize")].Link(save, nSize))
            nSize = 0;

        if (!save)
        {
            info.vecptPolygon_BCU.resize(nSize);
        }

        for (int polygonIndex = 0; polygonIndex < nSize; polygonIndex++)
        {
            auto& polygon_db = sub_db.GetSubDBFmt(_T("Polygon%d"), polygonIndex);

            if (version == 0)
            {
                // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
                return FALSE;
            }
            else
            {
                if (!LinkEx(save, polygon_db, info.vecptPolygon_BCU[polygonIndex]))
                {
                    info.vecptPolygon_BCU[polygonIndex] = Ipvm::Point32r2{};
                }
            }
        }
    }

    return TRUE;
}

void Para_PolygonRects::Add(const SPolygonRectNamePair& maskInfo)
{
    m_items.push_back(maskInfo);
}

bool Para_PolygonRects::Del(long orderIndex)
{
    if (orderIndex < 0 || orderIndex >= long(m_items.size()))
    {
        return false;
    }

    m_items.erase(m_items.begin() + orderIndex);

    return true;
}

const Para_PolygonRects::SPolygonRectNamePair& Para_PolygonRects::GetAt(long orderIndex) const
{
    return m_items[orderIndex];
}

Para_PolygonRects::SPolygonRectNamePair& Para_PolygonRects::GetAt(long orderIndex)
{
    return m_items[orderIndex];
}

long Para_PolygonRects::GetCount() const
{
    return long(m_items.size());
}

Para_PolygonRects& Para_PolygonRects::operator=(const Para_PolygonRects& object)
{
    m_items = object.m_items;

    return *this;
}
