#pragma once
#include <vector>
#include "SpecRoi.h"

class CiDataBase;
class CImageDisplay;

class SpecRoiList
{
public:
	SpecRoiList();
	~SpecRoiList();

	void Init();
	BOOL LinkDataBase(BOOL bSave, CiDataBase &db);

	const SpecRoi& operator [](long index) const;
	SpecRoi& operator [](long index);

	long	count() const;
	void	setCount(long count);
	RoiType getType() const;
	void	setType(RoiType type);
	long	getPolygonPointCount() const;
	void	setPolygonPointCount(long count);

	CString	m_name;

private:
	RoiType m_type;
	long	m_polygonPointCount;

	std::vector<SpecRoi> m_rois;
};
