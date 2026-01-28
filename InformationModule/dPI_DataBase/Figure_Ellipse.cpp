// Figure_Ellipse.cpp: implementation of the CEllipse class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Figure_Ellipse.h"

//#include "../../PI_Modules/dPI_Serialize\JobInterface.h"
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GUID CEllipse::guid = { 0x36b0aca2, 0x48c0, 0x4e30, { 0xa8, 0xd6, 0x3d, 0x2c, 0x88, 0x0, 0xb0, 0x68 } };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEllipse::CEllipse()
: fAxisLenX(0.f)		// mil
, fAxisLenY(0.f)		// mil
, fRotationAngle(0.f)	// Degree
, fCenX(0.f)			// mil
, fCenY(0.f)			// mil
, fHeight(0.f)			// mil
, fArcAngle(0.f)		// Degree
{
}

CEllipse::CEllipse(const CEllipse &Src)
: fAxisLenX(Src.fAxisLenX)
, fAxisLenY(Src.fAxisLenY)
, fRotationAngle(Src.fRotationAngle)
, fCenX(Src.fCenX)
, fCenY(Src.fCenY)
, fHeight(Src.fHeight)
, fArcAngle(Src.fArcAngle)
{
}

CEllipse::~CEllipse()
{
}

CEllipse &CEllipse::operator=(const CEllipse &Src)
{
	fAxisLenX = fAxisLenX;
	fAxisLenY = fAxisLenY;
	fRotationAngle = fRotationAngle;
	fCenX = fCenX;
	fCenY = fCenY;
	fHeight = fHeight;
	fArcAngle = fArcAngle;

	return *this;
}

CFigure *CEllipse::Clone() const
{
	return new CEllipse(*this);
}

const GUID &CEllipse::GetGUID() const
{
	return GetGUIDStatic();
}

const GUID &CEllipse::GetGUIDStatic()
{
	return CEllipse::guid;
}

CArchive &CEllipse::Load(CArchive &ar)
{
	long nVersion = 20140620;

	ar >> nVersion;

	switch(nVersion)
	{
	case 20140620:
		ar >> fAxisLenX;		// mil
		ar >> fAxisLenY;		// mil
		ar >> fRotationAngle;	// Degree
		ar >> fCenX;			// mil	: Layer Center 기준
		ar >> fCenY;			// mil	: Layer Center 기준
		ar >> fHeight;			// mil
		ar >> fArcAngle;		// Degree
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Figure Ellipse"));
		break;
	}

	return ar;
}

CArchive &CEllipse::Store(CArchive &ar) const
{
	long nVersion = 20140620;

	ar << nVersion;

	switch(nVersion)
	{
	case 20140620:
		ar << fAxisLenX;		// mil
		ar << fAxisLenY;		// mil
		ar << fRotationAngle;	// Degree
		ar << fCenX;			// mil	: Layer Center 기준
		ar << fCenY;			// mil	: Layer Center 기준
		ar << fHeight;			// mil
		ar << fArcAngle;		// Degree
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Figure Ellipse"));
		break;
	}

	return ar;
}

BOOL CEllipse::IsPointIn(float x, float y) const
{
	double lfCos = cos(fRotationAngle);
	double lfSin = sin(fRotationAngle);
	double lfA = (x*lfCos - y*lfSin - fCenX)/(0.5 * fAxisLenX);
	double lfB = (x*lfSin + y*lfCos - fCenY)/(0.5 * fAxisLenY);

	if(lfA*lfA + lfB*lfB - 1 < 0.f)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}