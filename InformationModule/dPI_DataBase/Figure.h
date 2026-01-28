#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_DATABASE_API __declspec(dllexport)
#else
#define DPI_DATABASE_API __declspec(dllimport)
#endif

class CFigure  
{
public:
	CFigure(){};
	virtual ~CFigure(){};

	virtual CFigure *Clone() const = 0;
	virtual const GUID &GetGUID() const = 0;

	virtual CArchive &Load(CArchive &ar) = 0;
	virtual CArchive &Store(CArchive &ar) const = 0;

	virtual BOOL IsPointIn(float x, float y) const = 0;		// x, y : Real Coord (mil) by Body Center.
};