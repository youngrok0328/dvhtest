#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_PACKAGE_SPEC_COSTOM_API __declspec(dllexport)
#else
#define DPI_PACKAGE_SPEC_COSTOM_API __declspec(dllimport)
#endif

class CFigure;
class CiDataBase;

#include <vector>

struct DPI_PACKAGE_SPEC_COSTOM_API SCustomInfoMaster
{
	SCustomInfoMaster();
	SCustomInfoMaster(const SCustomInfoMaster &Src);
	~SCustomInfoMaster();

	SCustomInfoMaster &operator=(const SCustomInfoMaster &Src);

	void Init();
	void Copy(const SCustomInfoMaster &Src);

	BOOL LinkDataBase(BOOL bSave, CiDataBase &db, long nCount);

	void ImportGerberTxt(LPCTSTR szPathName);

	static LPCTSTR GetDefault_strLayerName();

	std::vector<CFigure*>	vecElement;

	CString strLayerName;

	float fOffsetX;
	float fOffsetY;
};

CArchive &operator<<(CArchive &ar, const SCustomInfoMaster &Src);
CArchive &operator>>(CArchive &ar, SCustomInfoMaster &Dst);
