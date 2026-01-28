//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ArchiveAllType.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CArchive& operator<<(CArchive& ar, const Ipvm::Rect32s& object)
{
    ar << object.m_left;
    ar << object.m_top;
    ar << object.m_right;
    ar << object.m_bottom;

    return ar;
}
CArchive& operator>>(CArchive& ar, Ipvm::Rect32s& object)
{
    ar >> object.m_left;
    ar >> object.m_top;
    ar >> object.m_right;
    ar >> object.m_bottom;

    return ar;
}

void ArchiveAllType::Serialize_Element(CString& istr)
{
    try
    {
        int nValid = istr.IsEmpty() ? -1 : 1;

        Serialize_Element(nValid);

        if (nValid == 1)
        {
            CStringW strWBlockName;

            strWBlockName = istr;

            if (IsStoring())
            {
                (*this) << strWBlockName;
            }
            else
            {
                (*this) >> strWBlockName;
                istr = strWBlockName;
            }
        }
    }
    catch (CArchiveException* ae)
    {
        throw ae;
    }
}

void ArchiveAllType::Serialize_Element(CPoint& pt)
{
    if (IsStoring())
    {
        (*this) << pt.x;
        (*this) << pt.y;
    }
    else
    {
        (*this) >> pt.x;
        (*this) >> pt.y;
    }
}

void ArchiveAllType::Serialize_Element(CSize& sz)
{
    if (IsStoring())
    {
        (*this) << sz.cx;
        (*this) << sz.cy;
    }
    else
    {
        (*this) >> sz.cx;
        (*this) >> sz.cy;
    }
}

void ArchiveAllType::Serialize_Element(Ipvm::Rect32s& rt)
{
    if (IsStoring())
    {
        (*this) << rt.m_left;
        (*this) << rt.m_top;
        (*this) << rt.m_right;
        (*this) << rt.m_bottom;
    }
    else
    {
        (*this) >> rt.m_left;
        (*this) >> rt.m_top;
        (*this) >> rt.m_right;
        (*this) >> rt.m_bottom;
    }
}

void ArchiveAllType::Serialize_Binary(BYTEPTR& pData, ULONG& nDataNum)
{
    if (IsStoring())
    {
        (*this) << nDataNum;
        Write(pData, nDataNum);
    }
    else
    {
        (*this) >> nDataNum;

        delete[] pData;
        pData = new BYTE[nDataNum];
        Read(pData, nDataNum);
    }
}

void ArchiveAllType::Serialize_Illum(short* Illum, ULONG nChannelNum)
{
    int nIllum = nChannelNum, n;
    short nReadLED;

    if (IsStoring())
    {
        (*this) << nIllum;

        for (n = 0; n < nIllum; n++)
        {
            (*this) << Illum[n];
        }
    }
    else
    {
        (*this) >> nIllum;
        for (n = 0; n < nIllum; n++)
        {
            (*this) >> nReadLED;
            if (n < (int)nChannelNum)
                Illum[n] = nReadLED;
        }

        for (; n < (int)nChannelNum; n++)
            Illum[n] = 0;
    }
}

long ArchiveAllType::__GetStringBufferSize(CString& strData, WCHAR** pStrBuffer)
{
#ifdef _UNICODE
    if (pStrBuffer)
    {
        *pStrBuffer = strData.GetBuffer();
    }
    return strData.GetLength() + 1;
#else
    long nStringLength = (long)(strData.GetLength());
    static std::vector<WCHAR> vecwStrData;
    vecwStrData.resize(strData.GetLength() + 1);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strData, nStringLength + 1, &vecwStrData[0], nStringLength);

    vecwStrData[nStringLength] = 0;

    if (pStrBuffer)
    {
        *pStrBuffer = &vecwStrData[0];
    }

    return nStringLength + 1;
#endif
}
