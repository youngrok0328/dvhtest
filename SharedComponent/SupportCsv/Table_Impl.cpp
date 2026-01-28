//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Table_Impl.h"

//CPP_2_________________________________ This project's headers
#include "File.h"
#include "FileHelper.h"
#include "Row.h"
#include "TableColumnRow.h"

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

Table_Impl::Table_Impl(const TableColumnRow& columnRow, size_t rowCount)
    : m_columnRow(columnRow)
{
    m_pFile = new File;
    if (rowCount > 0)
        AddRows(rowCount);
}

Table_Impl::~Table_Impl()
{
    delete m_pFile;
}

void Table_Impl::Reset()
{
    m_pFile->Reset();
}

size_t Table_Impl::InsertRow(size_t nRow)
{
    return m_pFile->InsertRow(nRow, m_columnRow.GetRow().GetCount());
}

size_t Table_Impl::AddRow()
{
    return m_pFile->AddRow(m_columnRow.GetRow().GetCount());
}

size_t Table_Impl::AddRows(size_t nRowCount)
{
    return m_pFile->AddRows(nRowCount, m_columnRow.GetRow().GetCount());
}

void Table_Impl::SetFilePath(LPCTSTR filePath)
{
    m_pFile->SetFilePath(filePath);
}

bool Table_Impl::SaveAndRowDelete()
{
    return Save(true);
}

void Table_Impl::SetItemValue(size_t nRow, size_t nColumnKey, double fValue)
{
    size_t nColumn = m_columnRow.GetColumnIndex(nColumnKey);
    m_pFile->GetRow(nRow).SetFmt(nColumn, m_columnRow.GetFloatFmt(nColumnKey), fValue);
}

void Table_Impl::SetItemValue(size_t nRow, size_t nColumnKey, float fValue)
{
    size_t nColumn = m_columnRow.GetColumnIndex(nColumnKey);
    m_pFile->GetRow(nRow).SetFmt(nColumn, m_columnRow.GetFloatFmt(nColumnKey), fValue);
}

void Table_Impl::SetItemValue(size_t nRow, size_t nColumnKey, int32_t nValue)
{
    size_t nColumn = m_columnRow.GetColumnIndex(nColumnKey);
    m_pFile->GetRow(nRow).SetFmt(nColumn, _T("%d"), nValue);
}

void Table_Impl::SetItemValue(size_t nRow, size_t nColumnKey, LPCTSTR szText)
{
    size_t nColumn = m_columnRow.GetColumnIndex(nColumnKey);
    m_pFile->GetRow(nRow).SetTxt(nColumn, szText);
}

bool Table_Impl::Save()
{
    return Save(false);
}

bool Table_Impl::Save(bool bSaveRowDelete)
{
    CString strFilePath = m_pFile->GetSaveFilePath();
    CFileHelper resFile;
    if (!resFile.Open(strFilePath, CFileHelper::modeNoTruncate | CFileHelper::modeWrite))
    {
        // 파일이 없으면 Table Header를 먼저 써주자
        if (!resFile.Open(strFilePath, CFileHelper::modeCreate | CFileHelper::modeNoTruncate | CFileHelper::modeWrite))
        {
            return false;
        }

        CArchive ar(&resFile, CArchive::store);

        BYTE wd[] = {0xef, 0xbb, 0xbf};
        ar.Write(wd, sizeof(wd));

        CString text;
        m_columnRow.AppendOut(text);

        int allocLen = WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)text, -1, NULL, 0, NULL, NULL);
        CHAR* utf8Text = new CHAR[allocLen];
        memset(utf8Text, 0, allocLen);
        WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)text, -1, utf8Text, allocLen, NULL, NULL);
        ar.Write(utf8Text, allocLen - 1);
        delete[] utf8Text;

        ar.Close();
    }

    resFile.Close();

    if (bSaveRowDelete)
    {
        return m_pFile->SaveAndRowDelete();
    }

    return m_pFile->Save();
}

size_t Table_Impl::GetRowCount() const
{
    return m_pFile->GetRowCount();
}

LPCTSTR Table_Impl::GetSaveFilePath() const
{
    return m_pFile->GetSaveFilePath();
}
