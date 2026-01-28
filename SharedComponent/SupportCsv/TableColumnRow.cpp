//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "TableColumnRow.h"

//CPP_2_________________________________ This project's headers
#include "TableColumnRow_Impl.h"

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

TableColumnRow::TableColumnRow()
{
    m_impl = new TableColumnRow_Impl;
}

TableColumnRow::~TableColumnRow()
{
    delete m_impl;
}

void TableColumnRow::AddColumn(size_t nColumnKey, LPCTSTR szName, int32_t nDecimalPlaces)
{
    m_impl->AddColumn(nColumnKey, szName, nDecimalPlaces);
}

size_t TableColumnRow::GetColumnIndex(size_t nColumnKey) const
{
    return m_impl->GetColumnIndex(nColumnKey);
}

Row& TableColumnRow::GetRow()
{
    return m_impl->GetRow();
}

const Row& TableColumnRow::GetRow() const
{
    return m_impl->GetRow();
}

LPCTSTR TableColumnRow::GetFloatFmt(size_t nColumnKey) const
{
    return m_impl->GetFloatFmt(nColumnKey);
}

void TableColumnRow::AppendOut(CString& strText) const
{
    m_impl->AppendOut(strText);
}
