#pragma once

#ifdef __DA_BASE_MODULE_EXPORT__
#define _DA_BASE_MODULE_API __declspec(dllexport)
#else
#define _DA_BASE_MODULE_API __declspec(dllimport)
#endif

class _DA_BASE_MODULE_API ChipDB
{
public:
	ChipDB();
	ChipDB& operator=(const ChipDB &Dst) = default;

	CString strModelName;
	float fChipWidth;
	float fChipLength;
	COLORREF GroupColor;
};

extern _DA_BASE_MODULE_API CArchive& operator<<(CArchive& ar, const ChipDB& Dst);
extern _DA_BASE_MODULE_API CArchive& operator>>(CArchive& ar, ChipDB& Src);
