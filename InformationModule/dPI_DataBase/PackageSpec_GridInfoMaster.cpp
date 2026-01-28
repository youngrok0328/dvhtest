#include "stdafx.h"

#include "PackageSpec_GridInfoMaster.h"
#include "../../VisionHostCommon/DBObject.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SGridInfoMaster::SGridInfoMaster()
: nGridNumX(0)
, nGridNumY(0)
, fGridOffsetX(0.f)
, fGridOffsetY(0.f)
, fGridPitchX(0.f)
, fGridPitchY(0.f)
, fBallWidth(0.f)
, fBallHeight(0.f)
, vecnGridLayout(1)
, nPitchDefinition(0)
, strLayerName("Untitled Layer")
{
}

void SGridInfoMaster::Init()
{
	nGridNumX = 0;
	nGridNumY = 0;
	fGridOffsetX = 0.f;
	fGridOffsetY = 0.f;
	fGridPitchX = 0.f;
	fGridPitchY = 0.f;
	fBallWidth = 0.f;
	fBallHeight = 0.f;
	vecnGridLayout.resize(1);
	nPitchDefinition = 0;
	strLayerName = _T("Untitled Layer");
}

// Save & Load...
BOOL SGridInfoMaster::LinkDataBase(BOOL bSave, CiDataBase &db, long nCount)
{
	long nSize(0);
	CString strTemp;
	strTemp.Format(_T("GridInfoMaster_%d_"), nCount);

	if(!db[strTemp + _T("Grid Num X")].Link(bSave, nGridNumX)) nGridNumX = 0.f;
	if(!db[strTemp + _T("Grid Num Y")].Link(bSave, nGridNumY)) nGridNumY = 0.f;
	if(!db[strTemp + _T("Grid Offset X")].Link(bSave, fGridOffsetX)) fGridOffsetX = 0.f;
	if(!db[strTemp + _T("Grid Offset Y")].Link(bSave, fGridOffsetY)) fGridOffsetY = 0.f;
	if(!db[strTemp + _T("Grid Pitch X")].Link(bSave, fGridPitchX)) fGridPitchX = 0.f;
	if(!db[strTemp + _T("Grid Pitch Y")].Link(bSave, fGridPitchY)) fGridPitchY = 0.f;
	if(!db[strTemp + _T("Ball Width")].Link(bSave, fBallWidth)) fBallWidth = 0.f;
	if(!db[strTemp + _T("Ball Height")].Link(bSave, fBallHeight)) fBallHeight = 0.f;
	if(!db[strTemp + _T("Pitch Definition")].Link(bSave, nPitchDefinition)) nPitchDefinition = 0;
	if(!db[strTemp + _T("Layer Name")].Link(bSave, strLayerName)) strLayerName = _T("Untitled Layer");

	// vecnGridLayout ¿˙¿Â
	if(!bSave)
	{
		strTemp.Format(_T("GridInfoMaster_%d_GridLayout_Size"), nCount);
		if(!db[strTemp].Link(bSave, nSize)) nSize = 0;
		vecnGridLayout.resize(nSize);
	}
	else
	{
		nSize = (long)(vecnGridLayout.size());
		strTemp.Format(_T("GridInfoMaster_%d_GridLayout_Size"), nCount);
		if(!db[strTemp].Link(bSave, nSize)) nSize = 0;
	}
	for(long i = 0 ; i < nSize ; i++)
	{
		strTemp.Format(_T("GridInfoMaster_%d_GridLayout_%d"), nCount, i);
		if(!db[strTemp].Link(bSave, vecnGridLayout[i])) vecnGridLayout[i] = 0;
	}

	return TRUE;
}

CArchive &operator<<(CArchive &ar, const SGridInfoMaster &Src)
{
	long nVersion = 20140620;

	ar << nVersion;

	long nSize = 0;
	switch(nVersion)
	{
	case 20140620:
		ar << Src.nGridNumX;	
		ar << Src.nGridNumY;	
		ar << Src.fGridOffsetX;
		ar << Src.fGridOffsetY;
		ar << Src.fGridPitchX;
		ar << Src.fGridPitchY;
		ar << Src.fBallWidth;
		ar << Src.fBallHeight;
		ar << Src.nPitchDefinition;
		ar << Src.strLayerName;

		nSize = (long)Src.vecnGridLayout.size();
		ar << nSize;
		for(long n=0 ; n<nSize ; n++)
		{
			ar << Src.vecnGridLayout[n];
		}
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Grid Info Master"));
		break;
	}

	return ar;
}

CArchive &operator>>(CArchive &ar, SGridInfoMaster &Dst)
{
	long nVersion = 20140620;

	ar >> nVersion;

	long nSize = 0;
	switch(nVersion)
	{
	case 20140620:
		ar >> Dst.nGridNumX;	
		ar >> Dst.nGridNumY;	
		ar >> Dst.fGridOffsetX;
		ar >> Dst.fGridOffsetY;
		ar >> Dst.fGridPitchX;
		ar >> Dst.fGridPitchY;
		ar >> Dst.fBallWidth;
		ar >> Dst.fBallHeight;
		ar >> Dst.nPitchDefinition;
		ar >> Dst.strLayerName;

		ar >> nSize;
		Dst.vecnGridLayout.resize(nSize);
		for(long n=0 ; n<nSize ; n++)
		{
			ar >> Dst.vecnGridLayout[n];
		}
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Grid Info Master"));
		break;
	}

	return ar;
}
