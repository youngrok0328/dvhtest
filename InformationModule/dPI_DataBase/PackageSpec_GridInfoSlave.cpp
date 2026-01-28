#include "stdafx.h"

#include "PackageSpec_GridInfoSlave.h"
#include "../../VisionHostCommon/DBObject.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SGridInfoSlave::SGridInfoSlave()
: nGridNum(0)
, nBallNum(0)
{
}

void SGridInfoSlave::Init()
{
	vecnGridToReal.clear();
	vecnRealToGrid.clear();
	vecfSpecGridPosX.clear();
	vecfSpecGridPosY.clear();
	vecfSpecBallPosX.clear();
	vecfSpecBallPosY.clear();
	vecfSpecBallPosX_mm.clear();
	vecfSpecBallPosY_mm.clear();
}

// Save & Load...
BOOL SGridInfoSlave::LinkDataBase(BOOL bSave, CiDataBase &db, long nCount)
{
	long nSize(0);
	CString strTemp;
	strTemp.Format(_T("GridInfoSlave_%d_"), nCount);

	if(!db[strTemp + _T("Spec_Gravity_Center_Offset_X")].Link(bSave, fSpecGravityCenterOffsetX)) fSpecGravityCenterOffsetX = 0.f;
	if(!db[strTemp + _T("Spec_Gravity_Center_Offset_Y")].Link(bSave, fSpecGravityCenterOffsetY)) fSpecGravityCenterOffsetY = 0.f;


	// vecnGridToReal...
	if(!bSave)
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		if(!db[strTemp + _T("GridToReal_Size")].Link(bSave, nSize)) nSize = 0;
		vecnGridToReal.resize(nSize);
	}
	else
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		nSize = (long)(vecnGridToReal.size());
		if(!db[strTemp + _T("GridToReal_Size")].Link(bSave, nSize)) nSize = 0;
	}
	for(long i = 0 ; i < nSize ; i++)
	{
		strTemp.Format(_T("GridInfoSlave_%d_GridToReal_%d"), nCount, i);
		if(!db[strTemp].Link(bSave, vecnGridToReal[i])) vecnGridToReal[i] = 0;
	}

	// vecnRealToGrid...
	if(!bSave)
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		if(!db[strTemp + _T("RealToGrid_Size")].Link(bSave, nSize)) nSize = 0;
		vecnRealToGrid.resize(nSize);
	}
	else
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		nSize = (long)(vecnRealToGrid.size());
		if(!db[strTemp + _T("RealToGrid_Size")].Link(bSave, nSize)) nSize = 0;
	}
	for(long i = 0 ; i < nSize ; i++)
	{
		strTemp.Format(_T("GridInfoSlave_%d_RealToGrid_%d"), nCount, i);
		if(!db[strTemp].Link(bSave, vecnRealToGrid[i])) vecnRealToGrid[i] = 0;
	}

	// vecfSpecGridPosX...
	if(!bSave)
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		if(!db[strTemp + _T("SpecGridPosX_Size")].Link(bSave, nSize)) nSize = 0;
		vecfSpecGridPosX.resize(nSize);
	}
	else
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		nSize = (long)(vecfSpecGridPosX.size());
		if(!db[strTemp + _T("GridInfoSlave_SpecGridPosX_Size")].Link(bSave, nSize)) nSize = 0;
	}
	for(long i = 0 ; i < nSize ; i++)
	{
		strTemp.Format(_T("GridInfoSlave_%d_SpecGridPosX_%d"), nCount, i);
		if(!db[strTemp].Link(bSave, vecfSpecGridPosX[i])) vecfSpecGridPosX[i] = 0;
	}

	// vecfSpecGridPosY...
	if(!bSave)
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		if(!db[strTemp + _T("SpecGridPosY_Size")].Link(bSave, nSize)) nSize = 0;
		vecfSpecGridPosY.resize(nSize);
	}
	else
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		nSize = (long)(vecfSpecGridPosY.size());
		if(!db[strTemp + _T("SpecGridPosY_Size")].Link(bSave, nSize)) nSize = 0;
	}
	for(long i = 0 ; i < nSize ; i++)
	{
		strTemp.Format(_T("GridInfoSlave_%d_SpecGridPosY_%d"), nCount, i);
		if(!db[strTemp].Link(bSave, vecfSpecGridPosY[i])) vecfSpecGridPosY[i] = 0;
	}

	// vecfSpecBallPosX mm ...
	if (!bSave)
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		if (!db[strTemp + _T("SpecBallPosX_mm_Size")].Link(bSave, nSize)) nSize = 0;
		vecfSpecBallPosX_mm.resize(nSize);
	}
	else
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		nSize = (long)(vecfSpecBallPosX_mm.size());
		if (!db[strTemp + _T("SpecBallPosX_mm_Size")].Link(bSave, nSize)) nSize = 0;
	}
	nBallNum = nSize;
	for (long i = 0; i < nSize; i++)
	{
		strTemp.Format(_T("GridInfoSlave_%d_SpecBallPosX_mm_%d"), nCount, i);
		if (!db[strTemp].Link(bSave, vecfSpecBallPosX_mm[i])) vecfSpecBallPosX_mm[i] = 0;
	}

	// vecfSpecBallPosY mm ...
	if (!bSave)
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		if (!db[strTemp + _T("SpecBallPosY_mm_Size")].Link(bSave, nSize)) nSize = 0;
		vecfSpecBallPosY_mm.resize(nSize);
	}
	else
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		nSize = (long)(vecfSpecBallPosY_mm.size());
		if (!db[strTemp + _T("SpecBallPosY_mm_Size")].Link(bSave, nSize)) nSize = 0;
	}
	nBallNum = nSize;
	for (long i = 0; i < nSize; i++)
	{
		strTemp.Format(_T("GridInfoSlave_%d_SpecBallPosY_mm_%d"), nCount, i);
		if (!db[strTemp].Link(bSave, vecfSpecBallPosY_mm[i])) vecfSpecBallPosY_mm[i] = 0;
	}

	// vecfSpecBallPosX...
	if(!bSave)
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		if(!db[strTemp + _T("SpecBallPosX_Size")].Link(bSave, nSize)) nSize = 0;
		vecfSpecBallPosX.resize(nSize);
	}
	else
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		nSize = (long)(vecfSpecBallPosX.size());
		if(!db[strTemp + _T("SpecBallPosX_Size")].Link(bSave, nSize)) nSize = 0;
	}
	for(long i = 0 ; i < nSize ; i++)
	{
		strTemp.Format(_T("GridInfoSlave_%d_SpecBallPosX_%d"), nCount, i);
		if(!db[strTemp].Link(bSave, vecfSpecBallPosX[i])) vecfSpecBallPosX[i] = 0;
	}

	// vecfSpecBallPosY...
	if(!bSave)
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		if(!db[strTemp + _T("SpecBallPosY_Size")].Link(bSave, nSize)) nSize = 0;
		vecfSpecBallPosY.resize(nSize);
	}
	else
	{
		strTemp.Format(_T("GridInfoSlave_%d_"), nCount);
		nSize = (long)(vecfSpecBallPosY.size());
		if(!db[strTemp + _T("SpecBallPosY_Size")].Link(bSave, nSize)) nSize = 0;
	}
	for(long i = 0 ; i < nSize ; i++)
	{
		strTemp.Format(_T("GridInfoSlave_%d_SpecBallPosY_%d"), nCount, i);
		if(!db[strTemp].Link(bSave, vecfSpecBallPosY[i])) vecfSpecBallPosY[i] = 0;
	}

	return TRUE;
}

CArchive &operator<<(CArchive &ar, const SGridInfoSlave &Src)
{
	//{{ 이 녀석들은 호스트에서 사용해야 하므로 시리얼라이즈해야 함.
	float fBallPitchX = 0.f;	// Staggered 냐 아니냐에 따라서 값이 달라짐.
	float fBallPitchY = 0.f;	// Staggered 냐 아니냐에 따라서 값이 달라짐.
						//}}
	long nVersion = 20140620;

	ar << nVersion;

	switch(nVersion)
	{
	case 20140620:
		ar << fBallPitchX;
		ar << fBallPitchY;
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Grid Info Slave"));
		break;
	}

	return ar;
}

CArchive &operator>>(CArchive &ar, SGridInfoSlave &Dst)
{
	//{{ 이 녀석들은 호스트에서 사용해야 하므로 시리얼라이즈해야 함.
	float fBallPitchX;	// Staggered 냐 아니냐에 따라서 값이 달라짐.
	float fBallPitchY;	// Staggered 냐 아니냐에 따라서 값이 달라짐.
						//}}
	long nVersion = 20140620;

	ar >> nVersion;

	switch(nVersion)
	{
	case 20140620:
		ar >> fBallPitchX;
		ar >> fBallPitchY;
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Grid Info Slave"));
		break;
	}

	return ar;
}