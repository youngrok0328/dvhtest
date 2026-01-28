#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_DATABASE_API __declspec(dllexport)
#else
#define DPI_DATABASE_API __declspec(dllimport)
#endif

#include "Figure.h"

class DPI_DATABASE_API CRectangle : public CFigure  
{
public:
	CRectangle();
	CRectangle(const CRectangle &Src);
	virtual ~CRectangle();

	CRectangle &operator=(const CRectangle &Src);

	virtual CFigure *Clone() const;
	virtual const GUID &GetGUID() const;

	static const GUID &GetGUIDStatic();

	virtual CArchive &Load(CArchive &ar);
	virtual CArchive &Store(CArchive &ar) const;

	virtual BOOL IsPointIn(float x, float y) const;

	static CRectangle Intersect(const CRectangle &Src1, const CRectangle &Src2);

	BOOL IsEmpty() const;

	float fCenX;
	float fCenY;
	float fAxisLenX;
	float fAxisLenY;
	float fHeight;

private:
	static GUID guid;
};