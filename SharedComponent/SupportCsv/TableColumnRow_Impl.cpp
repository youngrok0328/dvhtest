//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "TableColumnRow_Impl.h"

//CPP_2_________________________________ This project's headers
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

TableColumnRow_Impl::TableColumnRow_Impl()
    : m_Row()
{
}

TableColumnRow_Impl::~TableColumnRow_Impl()
{
}

void TableColumnRow_Impl::AddColumn(size_t nColumnKey, LPCTSTR szName, int32_t nDecimalPlaces)
{
    m_vecColumnKey.resize(max(m_vecColumnKey.size(), nColumnKey + 1), -1);
    m_vecColumn_FloatFormat.resize(max(m_vecColumnKey.size(), nColumnKey + 1), _T(""));

    m_vecColumnKey[nColumnKey] = m_Row.GetCount();

    CString strFloatFormat;
    if (nDecimalPlaces >= 0)
    {
        strFloatFormat.Format(_T("%%.%df"), nDecimalPlaces);
    }
    else
    {
        strFloatFormat.Format(_T("%%f"));
    }

    m_Row.AddTxt(szName);
    m_vecColumn_FloatFormat[nColumnKey] = strFloatFormat;
}

size_t TableColumnRow_Impl::GetColumnIndex(size_t nColumnKey) const
{
    return m_vecColumnKey[nColumnKey];
}

Row& TableColumnRow_Impl::GetRow()
{
    return m_Row;
}

const Row& TableColumnRow_Impl::GetRow() const
{
    return m_Row;
}

LPCTSTR TableColumnRow_Impl::GetFloatFmt(size_t nColumnKey) const
{
    return m_vecColumn_FloatFormat[nColumnKey];
}

void TableColumnRow_Impl::AppendOut(CString& strText) const
{
    m_Row.AppendOut(strText);
}
