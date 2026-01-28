//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Table.h"

//CPP_2_________________________________ This project's headers
#include "Table_Impl.h"

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

Table::Table(const TableColumnRow& columnRow, size_t rowCount)
{
    m_impl = new Table_Impl(columnRow, rowCount);
}

Table::~Table()
{
    delete m_impl;
}

void Table::Reset()
{
    m_impl->Reset();
}

size_t Table::InsertRow(size_t nRow)
{
    return m_impl->InsertRow(nRow);
}

size_t Table::AddRow()
{
    return m_impl->AddRow();
}

size_t Table::AddRows(size_t nRowCount)
{
    return m_impl->AddRows(nRowCount);
}

bool Table::SaveAndRowDelete()
{
    return m_impl->SaveAndRowDelete();
}

void Table::SetFilePath(LPCTSTR filePath)
{
    m_impl->SetFilePath(filePath);
}

void Table::SetItemValue(size_t nRow, size_t nColumnKey, double fValue)
{
    m_impl->SetItemValue(nRow, nColumnKey, fValue);
}

void Table::SetItemValue(size_t nRow, size_t nColumnKey, float fValue)
{
    m_impl->SetItemValue(nRow, nColumnKey, fValue);
}

void Table::SetItemValue(size_t nRow, size_t nColumnKey, int32_t nValue)
{
    m_impl->SetItemValue(nRow, nColumnKey, nValue);
}

void Table::SetItemValue(size_t nRow, size_t nColumnKey, LPCTSTR szText)
{
    m_impl->SetItemValue(nRow, nColumnKey, szText);
}

void Table::SetItemFmt(size_t nRow, size_t nColumnKey, LPCTSTR szFmt, ...)
{
    CString strValue;

    va_list argptr;
    va_start(argptr, szFmt);
    strValue.FormatV(szFmt, argptr);
    va_end(argptr);

    m_impl->SetItemValue(nRow, nColumnKey, strValue);
}

bool Table::Save()
{
    return m_impl->Save();
}

size_t Table::GetRowCount() const
{
    return m_impl->GetRowCount();
}

LPCTSTR Table::GetSaveFilePath() const
{
    return m_impl->GetSaveFilePath();
}
