#include "stdafx.h"
#include "SpecRoiList.h"

#include "../../VisionHostCommon/DBObject.h"
#include "../../dA_Modules/dA_Base/iDataType.h"

#include "Base/Polygon.h"
#include "Base/Point_32f_C2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SpecRoiList::SpecRoiList()
	: m_type(RoiType::Rect)
	, m_name(_T("Unknown"))
	, m_polygonPointCount(3)
{
}

SpecRoiList::~SpecRoiList()
{
}

void SpecRoiList::Init()
{
	m_rois.clear();
}

BOOL SpecRoiList::LinkDataBase(BOOL bSave, CiDataBase &db)
{
	long count = long(m_rois.size());

	if (!db[_T("name")].Link(bSave, m_name)) m_name = _T("Unknown");
	if (!db[_T("type")].Link(bSave, (long &)m_type)) m_type = RoiType::Rect;
	if (!db[_T("polygonPointCount")].Link(bSave, m_polygonPointCount)) m_polygonPointCount = 3;
	if (!db[_T("count")].Link(bSave, count)) count = 0;

	if (!bSave)
	{
		m_rois.resize(count);
	}

	for (long index = 0; index < count; index++)
	{
		auto& sub_db = db.GetSubDBFmt(_T("roi%d"), index + 1);
		m_rois[index].LinkDataBase(bSave, sub_db);
	}

	return TRUE;
}

const SpecRoi& SpecRoiList::operator [](long index) const
{
	return m_rois[index];
}

SpecRoi& SpecRoiList::operator [](long index)
{
	return m_rois[index];
}

long SpecRoiList::count() const
{
	return long(m_rois.size());
}

void SpecRoiList::setCount(long count)
{
	m_rois.resize(count);
	setPolygonPointCount(m_polygonPointCount);
}

RoiType SpecRoiList::getType() const
{
	return m_type;
}

void SpecRoiList::setType(RoiType type)
{
	m_type = type;
}

long SpecRoiList::getPolygonPointCount() const
{
	return m_polygonPointCount;
}

void SpecRoiList::setPolygonPointCount(long count)
{
	m_polygonPointCount = max(3, count);

	for (long index = 0; index < long(m_rois.size()); index++)
	{
		auto& polygon = m_rois[index].m_polygon;

		long insertVertexStart = polygon.GetVertexNum();
		polygon.SetVertexNum(m_polygonPointCount);

		for (long point = insertVertexStart; point < m_polygonPointCount; point++)
		{
			float angle = 360.f * point / m_polygonPointCount;

			float co = (float)cos(angle * DEF_DEG_TO_RAD);
			float si = (float)sin(angle * DEF_DEG_TO_RAD);

			float x = 1000.f;
			float y = 1000.f;

			polygon.GetVertices()[point].m_x = (x*co - y * si);
			polygon.GetVertices()[point].m_y = (x*si + y * co);
		}
	}
}
