#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_PERSISTENCE_API__ RegistryHelper
{
public:
    static void Read(
        LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, BYTE* nValue, BYTE nDefValue = 0);
    static void Write(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, BYTE* nValue);
    static void Read(
        LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, long* nValue, long nDefValue = 0);
    static void Write(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, long* nValue);
    static void Read(
        LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, int* nValue, int nDefValue = 0);
    static void Write(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, int* nValue);
    static void Read(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, CString* strValue,
        CString strDefValue = _T(""));
    static void Write(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, CString* strValue);
    static void Read(
        LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, float* fValue, float fDefValue = 0);
    static void Write(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, float* fValue);
    static void Read(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, double* lfValue,
        double lfDefValue = 0.f);
    static void Write(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, double* lfValue);
    static void Read(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, CPoint* ptValue,
        CPoint ptDefValue = CPoint(0, 0));
    static void Write(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, CPoint* ptValue);
    static void Read(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, CRect* rtValue,
        CRect rtDefValue = CRect(0, 0, 0, 0));
    static void Write(LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, CRect* rtValue);

    template<class T>
    static void Link(bool bSave, LPCTSTR szCompany, LPCTSTR szAppName, LPCTSTR szCategory, LPCTSTR szKey, T* value)
    {
        if (bSave)
            Write(szCompany, szAppName, szCategory, szKey, value);
        else
            Read(szCompany, szAppName, szCategory, szKey, value);
    }
};
