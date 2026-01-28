#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_DATABASE_API __declspec(dllexport)
#else
#define DPI_DATABASE_API __declspec(dllimport)
#endif

#include "Figure.h"
#include <vector>

class DPI_DATABASE_API CPolygon : public CFigure  
{
public:
	CPolygon();
	CPolygon(const CPolygon &Src);
	virtual ~CPolygon();

	CPolygon &operator=(const CPolygon &Src);

	virtual CFigure *Clone() const;
	virtual const GUID &GetGUID() const;

	static const GUID &GetGUIDStatic();

	virtual CArchive &Load(CArchive &ar);
	virtual CArchive &Store(CArchive &ar) const;

	virtual BOOL IsPointIn(float x, float y) const;

	std::vector<float> vecx;
	std::vector<float> vecy;

private:
	static GUID guid;
};