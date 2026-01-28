#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_DATABASE_API __declspec(dllexport)
#else
#define DPI_DATABASE_API __declspec(dllimport)
#endif

#include <vector>
#include "../../dA_Modules/dA_Base/\iDataType.h"

class CArchive;
class CiDataBase;
class ChipDB;

class DPI_DATABASE_API CPassiveJobInfo
{
public:
	CPassiveJobInfo();

	std::vector<ChipDB> m_vecsChipData;

	void AddType();
	void Save();
	void Open();

public:
	///////////////////////////////////////////////////////////////
	////// Data List///////////////////////////////////////////////
	///////////////////////////////////////////////////////////////
	
	long nType; // 타입
	float fLength; // 길이
	float fWidth; // 너비

public:
	IPVM::Rect rtROI; // LayoutEditor에서의 실제 위치 ( Pixel 좌표계 )
};