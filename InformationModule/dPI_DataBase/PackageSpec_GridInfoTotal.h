#pragma once

class CiDataBase;

struct SGridInfoTotal
{
	SGridInfoTotal();

	void Init();

	BOOL LinkDataBase(BOOL bSave, CiDataBase &db);

	long nGridNum;
	long nBallNum;	// 이 녀석은 호스트에서 사용해야 하므로 시리얼라이즈해야 함.
	float fBallWidthMin;
	float fBallWidthMax;
	float fBallHeightMin;
	float fBallHeightMax;
	float fGridPitchXMax;
	float fGridPitchYMax;
};

CArchive &operator<<(CArchive &ar, const SGridInfoTotal &Src);
CArchive &operator>>(CArchive &ar, SGridInfoTotal &Dst);