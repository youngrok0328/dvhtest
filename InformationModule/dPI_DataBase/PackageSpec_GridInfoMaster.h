#pragma once

#include <vector>
class CArchive;
class CiDataBase;

struct SGridInfoMaster	// 사용자가 직접 지정하는 값의 모음
{										// 이 값을 변경시키는 것은 반드시 해단 UI 에서만 진행할 것
	SGridInfoMaster();

	void Init();

	BOOL LinkDataBase(BOOL bSave, CiDataBase &db, long nCount);

	long nGridNumX;		
	long nGridNumY;		

	float fGridOffsetX;
	float fGridOffsetY;

	float fGridPitchX;
	float fGridPitchY;

	float fBallWidth;
	float fBallHeight;

	std::vector<long>				vecnGridLayout;

	long nPitchDefinition;			// 0 : Normal
									// 1 : Staggered

	CString strLayerName;

};

CArchive &operator<<(CArchive &ar, const SGridInfoMaster &Src);
CArchive &operator>>(CArchive &ar, SGridInfoMaster &Dst);
