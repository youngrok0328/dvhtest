#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_DATABASE_API __declspec(dllexport)
#else
#define DPI_DATABASE_API __declspec(dllimport)
#endif

#include "Figure.h"

class DPI_DATABASE_API CEllipse : public CFigure
{
public:
	CEllipse();
	CEllipse(const CEllipse &Src);
	virtual ~CEllipse();

	CEllipse &operator=(const CEllipse &Src);

	virtual CFigure *Clone() const;
	virtual const GUID &GetGUID() const;

	static const GUID &GetGUIDStatic();

	virtual CArchive &Load(CArchive &ar);
	virtual CArchive &Store(CArchive &ar) const;

	virtual BOOL IsPointIn(float x, float y) const;

	float fAxisLenX;		// mil
	float fAxisLenY;		// mil
//	float fRotationAngle;	// Degree	==> deprecated
	float fCenX;			// mil	: Layer Center 기준
	float fCenY;			// mil	: Layer Center 기준
	float fHeight;			// mil
//	float fArcAngle;		// Degree	==> deprecated

private:
	static GUID guid;

private:	// deprecated
	float fRotationAngle;	// Degree
	float fArcAngle;		// Degree
};