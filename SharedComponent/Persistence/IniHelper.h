#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <stdint.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_PERSISTENCE_API__ IniHelper
{
public:
    static CString MakeKeyName(LPCTSTR szNameHeader, LPCTSTR szKeyname, int nKeyPostNum);

    static bool LoadBOOL(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, bool defValue = false);
    static void SaveBOOL(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, bool value);
    static uint8_t LoadBYTE(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint8_t defValue = 0);
    static void SaveBYTE(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint8_t value);
    static int16_t LoadSHORT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, int16_t defValue = 0);
    static void SaveSHORT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, int16_t value);
    static int LoadINT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, int defValue = 0);
    static void SaveINT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, int value);
    static CString LoadSTRING(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CString defValue = _T(""));
    static void SaveSTRING(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CString value);
    static float LoadFLOAT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, float defValue = 0.f);
    static void SaveFLOAT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, float value);
    static double LoadDOUBLE(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, double defValue = 0.);
    static void SaveDOUBLE(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, double value);
    static CPoint LoadPOINT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CPoint defValue = CPoint(0, 0));
    static void SavePOINT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CPoint value);
    static CRect LoadRECT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CRect defValue = CRect(0, 0, 0, 0));
    static void SaveRECT(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CRect value);
    static uint64_t LoadULONGLONG(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint64_t defValue = 0);
    static void SaveULONGLONG(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint64_t value);

    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, bool* value, bool defValue = false);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, bool* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint8_t* value, uint8_t defValue = 0);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint8_t* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, int16_t* value, int16_t defValue = 0);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, int16_t* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint16_t* value, uint16_t defValue = 0);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint16_t* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, int32_t* value, int32_t defValue = 0);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, int32_t* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint32_t* value, uint32_t defValue = 0);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint32_t* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint64_t* value, uint64_t defValue = 0);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, uint64_t* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, long* value, long defValue = 0);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, long* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CString* strValue, CString defValue = _T(""));
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CString* strValue);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, float* value, float defValue = 0);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, float* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, double* value, double defValue = 0.f);
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, double* value);
    static void Read(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CPoint* value, CPoint defValue = CPoint(0, 0));
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CPoint* value);
    static void Read(
        LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CRect* value, CRect rtDefValue = CRect(0, 0, 0, 0));
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CRect* value);
    static void Read(
        LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CTimeSpan* value, CTimeSpan defValue = CTimeSpan());
    static void Write(LPCTSTR pathName, LPCTSTR category, LPCTSTR key, CTimeSpan* value);
    static void Delete(LPCTSTR pathName, LPCTSTR category);

    template<class T>
    static void Link(bool bSave, LPCTSTR pathName, LPCTSTR category, LPCTSTR key, T* value)
    {
        if (bSave)
            Write(pathName, category, key, value);
        else
            Read(pathName, category, key, value, *value);
    }
};
