#include "stdafx.h"
#include "ChipDB.h"


ChipDB::ChipDB()
	: fChipWidth(0.f)
	, fChipLength(0.f)
	, GroupColor(RGB(255, 255, 128))
{
}

CArchive& operator<<(CArchive& ar, const ChipDB& Dst)
{
	long nVesrion = 20140526;

	ar << nVesrion;

	switch (nVesrion)
	{
	case 20140526:
	{
		ar << Dst.strModelName;
		ar << Dst.fChipWidth;
		ar << Dst.fChipLength;
		ar << Dst.GroupColor;
	}
	break;
	default:
		AfxMessageBox(_T("Archive Error ChipDB.h"));
		break;
	}

	return ar;
}

CArchive& operator>>(CArchive& ar, ChipDB& Src)
{
	long nVesrion;

	ar >> nVesrion;

	switch (nVesrion)
	{
	case 20140526:
	{
		ar >> Src.strModelName;
		ar >> Src.fChipWidth;
		ar >> Src.fChipLength;
		ar >> Src.GroupColor;
	}
	break;
	default:
		AfxMessageBox(_T("Archive Error ChipDB.h"));
		break;
	}
	return ar;
}
