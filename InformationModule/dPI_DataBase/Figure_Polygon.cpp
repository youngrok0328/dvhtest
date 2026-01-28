// Figure_Polygon.cpp: implementation of the CPolygon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Figure_Polygon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GUID CPolygon::guid = { 0x19b165f4, 0x3641, 0x4f12, { 0xb4, 0xc1, 0x76, 0x6, 0x62, 0xbc, 0x30, 0x77 } };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPolygon::CPolygon()
{
}

CPolygon::CPolygon(const CPolygon &Src)
: vecx(Src.vecx)
, vecy(Src.vecy)
{
}

CPolygon::~CPolygon()
{
}

CPolygon &CPolygon::operator=(const CPolygon &Src)
{
	vecx = Src.vecx;
	vecy = Src.vecy;

	return *this;
}

CFigure *CPolygon::Clone() const
{
	return new CPolygon(*this);
}

const GUID &CPolygon::GetGUID() const
{
	return GetGUIDStatic();
}

const GUID &CPolygon::GetGUIDStatic()
{
	return CPolygon::guid;
}

CArchive &CPolygon::Load(CArchive &ar)
{
	long nVersion = 20140620;

	ar >> nVersion;

	long nSize = 0;
	switch(nVersion)
	{
	case 20140620:
		ar >> nSize;
		vecx.resize(nSize);
		for(long n=0 ; n<nSize ; n++)
		{
			ar >> vecx[n];		// mil
		}

		vecy.resize(nSize);
		for(long n=0 ; n<nSize ; n++)
		{
			ar >> vecy[n];		// mil
		}
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Figure Polygon"));
		break;
	}

	return ar;
}

CArchive &CPolygon::Store(CArchive &ar) const
{
	long nVersion = 20140620;

	ar << nVersion;

	long nSize = 0;
	switch(nVersion)
	{
	case 20140620:
		nSize = (long)vecx.size();
		ar << nSize;
		for(long n=0 ; n<nSize ; n++)
		{
			ar << vecx[n];		// mil
		}

		nSize = (long)vecy.size();
		ar << nSize;
		for(long n=0 ; n<nSize ; n++)
		{
			ar << vecy[n];		// mil
		}
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Figure Polygon"));
		break;
	}

	return ar;
}

BOOL CPolygon::IsPointIn(float x, float y) const
{
	return FALSE;
}