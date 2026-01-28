#include "stdafx.h"

#include "PackageSpec_GridInfoTotal.h"
#include "../../VisionHostCommon/DBObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SGridInfoTotal::SGridInfoTotal()
: nGridNum(0)
, nBallNum(0)
, fBallWidthMin(0.f)
, fBallWidthMax(0.f)
, fBallHeightMin(0.f)
, fBallHeightMax(0.f)
, fGridPitchXMax(0.f)
, fGridPitchYMax(0.f)
{
}

void SGridInfoTotal::Init()
{
	nGridNum = 0;
	nBallNum = 0;
	fBallWidthMin = 0.f;
	fBallWidthMin = 0.f;
	fBallWidthMax = 0.f;
	fBallHeightMin = 0.f;
	fBallHeightMax = 0.f;
	fGridPitchXMax = 0.f;
	fGridPitchYMax = 0.f;
}

// Save & Load...
BOOL SGridInfoTotal::LinkDataBase(BOOL bSave, CiDataBase &db)
{
	if(!db[_T("GridInfoTotal_Grid_Num")].Link(bSave, nGridNum)) nGridNum = 0;
	if(!db[_T("GridInfoTotal_Ball_Num")].Link(bSave, nBallNum)) nBallNum = 0;
	if(!db[_T("GridInfoTotal_Ball_Width_Min")].Link(bSave, fBallWidthMin)) fBallWidthMin = 0.f;
	if(!db[_T("GridInfoTotal_Ball_Width_Min")].Link(bSave, fBallWidthMin)) fBallWidthMin = 0.f;
	if(!db[_T("GridInfoTotal_Ball_Width_Max")].Link(bSave, fBallWidthMax)) fBallWidthMax = 0.f;
	if(!db[_T("GridInfoTotal_Ball_Height_Min")].Link(bSave, fBallHeightMin)) fBallHeightMin = 0.f;
	if(!db[_T("GridInfoTotal_Ball_Height_Max")].Link(bSave, fBallHeightMax)) fBallHeightMax = 0.f;
	if(!db[_T("GridInfoTotal_Grid_Pitch_X_Max")].Link(bSave, fGridPitchXMax)) fGridPitchXMax = 0.f;
	if(!db[_T("GridInfoTotal_Grid_Pitch_Y_Max")].Link(bSave, fGridPitchYMax)) fGridPitchYMax = 0.f;

	return TRUE;
}

CArchive &operator<<(CArchive &ar, const SGridInfoTotal &Src)
{
	float fGridPitchXMin = 0.f;
	float fGridPitchYMin = 0.f;

	long nVersion = 20140620;

	ar << nVersion;

	switch(nVersion)
	{
	case 20140620:
		ar << Src.nBallNum;
		ar << Src.fBallWidthMin;
		ar << Src.fBallWidthMin;
		ar << Src.fBallWidthMax;
		ar << Src.fBallHeightMin;
		ar << Src.fBallHeightMax;
		ar << fGridPitchXMin;
		ar << Src.fGridPitchXMax;
		ar << fGridPitchYMin;
		ar << Src.fGridPitchYMax;
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Grid Info Total"));
		break;
	}

	return ar;
}

CArchive &operator>>(CArchive &ar, SGridInfoTotal &Dst)
{
	float fGridPitchXMin;
	float fGridPitchYMin;

	long nVersion = 20140620;

	ar >> nVersion;

	switch(nVersion)
	{
	case 20140620:
		ar >> Dst.nBallNum;
		ar >> Dst.fBallWidthMin;
		ar >> Dst.fBallWidthMin;
		ar >> Dst.fBallWidthMax;
		ar >> Dst.fBallHeightMin;
		ar >> Dst.fBallHeightMax;
		ar >> fGridPitchXMin;
		ar >> Dst.fGridPitchXMax;
		ar >> fGridPitchYMin;
		ar >> Dst.fGridPitchYMax;
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Grid Info Total"));
		break;
	}

	return ar;
}