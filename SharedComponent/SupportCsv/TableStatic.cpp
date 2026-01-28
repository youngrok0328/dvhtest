//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "TableStatic.h"

//CPP_2_________________________________ This project's headers
#include "TableStatic_Impl.h"

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

TableStatic::TableStatic(const TableColumnRow& columnRow, size_t rowCount)
{
    m_impl = new TableStatic_Impl(columnRow, rowCount);
}

TableStatic::~TableStatic()
{
    delete m_impl;
}

void TableStatic::SetFilePath(LPCTSTR filePath)
{
    m_impl->SetFilePath(filePath);
}

void TableStatic::SetItemValue(size_t nRow, size_t nColumnKey, double fValue)
{
    m_impl->SetItemValue(nRow, nColumnKey, fValue);
}

void TableStatic::SetItemValue(size_t nRow, size_t nColumnKey, float fValue)
{
    m_impl->SetItemValue(nRow, nColumnKey, fValue);
}

void TableStatic::SetItemValue(size_t nRow, size_t nColumnKey, int32_t nValue)
{
    m_impl->SetItemValue(nRow, nColumnKey, nValue);
}

void TableStatic::SetItemValue(size_t nRow, size_t nColumnKey, LPCTSTR szText)
{
    m_impl->SetItemValue(nRow, nColumnKey, szText);
}

void TableStatic::SetItemFmt(size_t nRow, size_t nColumnKey, LPCTSTR szFmt, ...)
{
    va_list argptr;
    va_start(argptr, szFmt);
    m_impl->SetItemFmt(nRow, nColumnKey, szFmt, argptr);
    va_end(argptr);
}

bool TableStatic::Save()
{
    return m_impl->Save();
}

size_t TableStatic::GetRowCount() const
{
    return m_impl->GetRowCount();
}

LPCTSTR TableStatic::GetSaveFilePath() const
{
    return m_impl->GetSaveFilePath();
}
