#include "stdafx.h"
#include "ResultDataRoi.h"
#include "SpecRoi.h"

#include "Base/Point_32f_C2.h"
#include "Base/Rect_32f.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ResultDataRoi::ResultDataRoi()
	: m_type(RoiType::Rect)
{
}

ResultDataRoi::~ResultDataRoi()
{
}

void ResultDataRoi::Set(RoiType type, const IPVM::Point_32f_C2& px2um, const IPVM::Point_32f_C2& imageCenter, const SpecRoi& roi)
{
	m_type = type;

	switch (type)
	{
	case RoiType::Ellipse:
	case RoiType::Rect:
		if (1)
		{
			IPVM::Point_32f_C2 center;
			center.m_x = imageCenter.m_x + roi.m_rect.CenterPoint().m_x / px2um.m_x;
			center.m_y = imageCenter.m_y + roi.m_rect.CenterPoint().m_y / px2um.m_y;

			IPVM::Rect_32f rect;
			rect.m_left = center.m_x - roi.m_rect.Width() / px2um.m_x * 0.5f;
			rect.m_top = center.m_y - roi.m_rect.Height() / px2um.m_y * 0.5f;
			rect.m_right = center.m_x + roi.m_rect.Width() / px2um.m_x * 0.5f;
			rect.m_bottom = center.m_y + roi.m_rect.Height() / px2um.m_y * 0.5f;

			m_rect = rect;
		}
		
	case RoiType::Polygon:
		m_polygon.clear();
		for (long vertex = 0; vertex < roi.m_polygon.GetVertexNum(); vertex++)
		{
			auto& specPoint = roi.m_polygon.GetVertices()[vertex];
			IPVM::Point_32f_C2 newPoint;
			newPoint.m_x = imageCenter.m_x + specPoint.m_x / px2um.m_x;
			newPoint.m_y = imageCenter.m_y + specPoint.m_y / px2um.m_y;

			m_polygon.push_back(newPoint);
		}
		break;

	default:
		ASSERT(!_T("??"));
	}
}

bool ResultDataRoi::operator == (const ResultDataRoi& object) const
{
	if (m_type != object.m_type) return false;
	if (m_type == RoiType::Polygon)
	{
		if (m_polygon != object.m_polygon) return false;
	}
	else
	{
		if (m_rect != object.m_rect) return false;
	}

	return true;
}

bool ResultDataRoi::operator != (const ResultDataRoi& object) const
{
	return !(*this == object);
}
