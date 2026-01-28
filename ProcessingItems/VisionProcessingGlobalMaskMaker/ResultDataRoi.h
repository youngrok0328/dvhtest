#pragma once
#include "Predefine.h"
#include "Base/Rect.h"
#include "Base/Point_32s_C2.h"
#include <vector>

class SpecRoi;

class ResultDataRoi
{
public:
	ResultDataRoi();
	~ResultDataRoi();

	void Set(RoiType type, const IPVM::Point_32f_C2& px2um, const IPVM::Point_32f_C2& imageCenter, const SpecRoi& roi);

	bool operator == (const ResultDataRoi& object) const;
	bool operator != (const ResultDataRoi& object) const;

	RoiType m_type;
	IPVM::Rect	m_rect;
	std::vector<IPVM::Point_32s_C2> m_polygon;
};

