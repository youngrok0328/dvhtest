#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#ifdef _AFXDLL
#include <afx.h>
#endif
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
CArchive& operator<<(CArchive& ar, const Ipvm::Rect32s& object);
CArchive& operator>>(CArchive& ar, Ipvm::Rect32s& object);

class __VISION_HOST_COMMON_API__ ArchiveAllType : public CArchive
{
public:
    ArchiveAllType(CFile* pFile, UINT nMode)
        : CArchive(pFile, nMode) {};
    virtual ~ArchiveAllType() {};

    template<class T>
    void Serialize_Element(T& t)
    {
        if (IsStoring())
            (*this) << t;
        else
            (*this) >> t;
    };

    typedef BYTE* BYTEPTR;

    void Serialize_Element(CString& istr);
    void Serialize_Element(CPoint& pt);
    void Serialize_Element(CSize& sz);
    void Serialize_Element(Ipvm::Rect32s& rt);
    void Serialize_Binary(BYTEPTR& pData, ULONG& nDataNum);
    void Serialize_Illum(short* Illum, ULONG nChannelNum);

private:
    long __GetStringBufferSize(CString& strData, WCHAR** pStrBuffer = NULL);
};
