// Figure_Rectangle.cpp: implementation of the CRectangle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Figure_Rectangle.h"

//#include "../../PI_Modules/dPI_Serialize\JobInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GUID CRectangle::guid = { 0x141f6670, 0x42d5, 0x4498, { 0xbe, 0xfe, 0x53, 0xda, 0xbf, 0x32, 0xf7, 0x76 } };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRectangle::CRectangle()
: fCenX(0.f)			// mil
, fCenY(0.f)			// mil
, fAxisLenX(0.f)		// mil
, fAxisLenY(0.f)		// mil
, fHeight(0.f)			// mil
{
}

CRectangle::CRectangle(const CRectangle &Src)
: fCenX(Src.fCenX)
, fCenY(Src.fCenY)
, fAxisLenX(Src.fAxisLenX)
, fAxisLenY(Src.fAxisLenY)
, fHeight(Src.fHeight)
{
}

CRectangle::~CRectangle()
{
}

CRectangle &CRectangle::operator=(const CRectangle &Src)
{
	fCenX = Src.fCenX;
	fCenY = Src.fCenY;
	fAxisLenX = Src.fAxisLenX;
	fAxisLenY = Src.fAxisLenY;
	fHeight = Src.fHeight;

	return *this;
}

CFigure *CRectangle::Clone() const
{
	return new CRectangle(*this);
}

const GUID &CRectangle::GetGUID() const
{
	return GetGUIDStatic();
}

const GUID &CRectangle::GetGUIDStatic()
{
	return CRectangle::guid;
}

CRectangle CRectangle::Intersect(const CRectangle &Src1, const CRectangle &Src2)
{
	CRectangle Result;

	float fSrc1_left = Src1.fCenX - 0.5f * Src1.fAxisLenX;
	float fSrc1_right = Src1.fCenX + 0.5f * Src1.fAxisLenX;
	float fSrc1_top = Src1.fCenY - 0.5f * Src1.fAxisLenY;
	float fSrc1_bottom = Src1.fCenY + 0.5f * Src1.fAxisLenY;

	float fSrc2_left = Src2.fCenX - 0.5f * Src2.fAxisLenX;
	float fSrc2_right = Src2.fCenX + 0.5f * Src2.fAxisLenX;
	float fSrc2_top = Src2.fCenY - 0.5f * Src2.fAxisLenY;
	float fSrc2_bottom = Src2.fCenY + 0.5f * Src2.fAxisLenY;

	if(fSrc1_left >= fSrc2_right)	return Result;
	if(fSrc1_right <= fSrc2_left)	return Result;
	if(fSrc1_top >= fSrc2_bottom)	return Result;
	if(fSrc1_bottom <= fSrc2_top)	return Result;

	float fRes_left = max(fSrc1_left, fSrc2_left);
	float fRes_right = min(fSrc1_right, fSrc2_right);
	float fRes_top = max(fSrc1_top, fSrc2_top);
	float fRes_bottom = min(fSrc1_bottom, fSrc2_bottom);

	Result.fCenX = 0.5f * (fRes_left + fRes_right);
	Result.fCenY = 0.5f * (fRes_top + fRes_bottom);
	Result.fAxisLenX = fRes_right - fRes_left;
	Result.fAxisLenY = fRes_bottom - fRes_top;

	return Result;
}

BOOL CRectangle::IsEmpty() const
{
	return fAxisLenX <= 0.f || fAxisLenY <= 0.f;
}

CArchive &CRectangle::Load(CArchive &ar)
{
	long nVersion = 20140620;

	ar >> nVersion;

	switch(nVersion)
	{
	case 20140620:
		ar >> fCenX;			// mil	: Layer Center 기준
		ar >> fCenY;			// mil	: Layer Center 기준
		ar >> fAxisLenX;		// mil
		ar >> fAxisLenY;		// mil
		ar >> fHeight;			// mil
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Figure Rectangle"));
		break;
	}

	return ar;
}

CArchive &CRectangle::Store(CArchive &ar) const
{
	long nVersion = 20140620;

	ar << nVersion;

	switch(nVersion)
	{
	case 20140620:
		ar << fCenX;			// mil	: Layer Center 기준
		ar << fCenY;			// mil	: Layer Center 기준
		ar << fAxisLenX;		// mil
		ar << fAxisLenY;		// mil
		ar << fHeight;			// mil
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Figure Rectangle"));
		break;
	}

	return ar;
}

BOOL CRectangle::IsPointIn(float x, float y) const
{
	return FALSE;
}