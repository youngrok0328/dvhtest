#pragma once

#include <vector>
class CiDataBase;

struct SGridInfoSlave	// Master 스펙으로부터 계산되는 값의 모음
{
	SGridInfoSlave();

	void Init();

	BOOL LinkDataBase(BOOL bSave, CiDataBase &db, long nCount);

	long nGridNum;
	long nBallNum;

	std::vector<CString> vecstrID;

	std::vector<long>	vecnGridToReal;
	std::vector<long>	vecnRealToGrid;

	std::vector<float> vecfSpecGridPosX;		// Grid Position X : mil 단위, x grid center 가 원점
	std::vector<float> vecfSpecGridPosY;		// Grid Position Y : mil 단위, y grid center 가 원점
	std::vector<float> vecfSpecBallPosX;		// Ball Position X : Image Coord, According to Real Ball Count
	std::vector<float> vecfSpecBallPosY;		// Ball Position Y : Image Coord, According to Real Ball Count

	std::vector<float> vecfSpecBallPosX_mm;		// Ball Position X : mm
	std::vector<float> vecfSpecBallPosY_mm;		// Ball Position Y : mm

	float fSpecGravityCenterOffsetX;		// Dist From Body Center to Balls' Gravity Center : Image Coord
	float fSpecGravityCenterOffsetY;		// Dist From Body Center to Balls' Gravity Center : Image Coord

};

CArchive &operator<<(CArchive &ar, const SGridInfoSlave &Src);
CArchive &operator>>(CArchive &ar, SGridInfoSlave &Dst);