//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Row.h"

//CPP_2_________________________________ This project's headers
#include "Row_Impl.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
using namespace SupportCSV;

Row::Row(size_t itemCount)
{
    m_impl = new Row_Impl(itemCount);
}

Row::~Row()
{
    delete m_impl;
}

size_t Row::AddTxt(LPCTSTR text)
{
    return m_impl->AddTxt(text);
}

void Row::SetTxt(size_t nItem, LPCTSTR szText)
{
    return m_impl->SetTxt(nItem, szText);
}

void Row::SetFmt(size_t nItem, LPCTSTR szFmt, ...)
{
    va_list argptr;
    va_start(argptr, szFmt);
    m_impl->SetFmt(nItem, szFmt, argptr);
    va_end(argptr);
}

size_t Row::GetCount() const
{
    return m_impl->GetCount();
}

const TCHAR* Row::GetTxt(size_t nItem) const
{
    return m_impl->GetTxt(nItem);
}

void Row::AppendOut(CString& strText) const
{
    m_impl->AppendOut(strText);
}
