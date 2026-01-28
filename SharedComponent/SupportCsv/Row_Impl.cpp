//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Row_Impl.h"

//CPP_2_________________________________ This project's headers
#include "defineHeader.h"

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

Row_Impl::Row_Impl(size_t itemCount)
    : m_itemCount(itemCount)
{
    m_text = new TCHAR[CSV_FILED_MAX_SIZE * itemCount];

    if (m_itemCount > 0)
    {
        memset(m_text, 0, CSV_FILED_MAX_SIZE * m_itemCount * sizeof(TCHAR));
    }
}

Row_Impl::~Row_Impl()
{
    delete[] m_text;
}

size_t Row_Impl::AddTxt(LPCTSTR text)
{
    TCHAR* oldText = m_text;

    m_text = new TCHAR[CSV_FILED_MAX_SIZE * (m_itemCount + 1)];

    memcpy(m_text, oldText, CSV_FILED_MAX_SIZE * m_itemCount * sizeof(TCHAR));

    delete[] oldText;

    _sntprintf_s(CSV_FILED_S_DYNAMIC(m_text, m_itemCount), CSV_FILED_MAX_SIZE, CSV_FILED_MAX_SIZE - 1, text);
    m_itemCount++;

    return m_itemCount - 1;
}

void Row_Impl::SetTxt(size_t nItem, LPCTSTR szText)
{
    _sntprintf_s(CSV_FILED_S_DYNAMIC(m_text, nItem), CSV_FILED_MAX_SIZE, CSV_FILED_MAX_SIZE - 1, szText);
}

void Row_Impl::SetFmt(size_t nItem, LPCTSTR szFmt, va_list& argptr)
{
    _vsntprintf_s(CSV_FILED_S_DYNAMIC(m_text, nItem), CSV_FILED_MAX_SIZE, CSV_FILED_MAX_SIZE - 1, szFmt, argptr);
}

size_t Row_Impl::GetCount() const
{
    return m_itemCount;
}

const TCHAR* Row_Impl::GetTxt(size_t nItem) const
{
    return CSV_FILED_S_DYNAMIC(m_text, nItem);
}

void Row_Impl::AppendOut(CString& strText) const
{
    size_t nFieldCount = GetCount();

    for (size_t nColumn = 0; nColumn < nFieldCount; nColumn++)
    {
        const TCHAR* pFieldText = GetTxt(nColumn);

        if (nColumn == 0)
        {
            strText += pFieldText;
        }
        else
        {
            strText.AppendFormat(_T(",%s"), pFieldText);
        }
    }
}
