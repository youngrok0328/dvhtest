//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "TableStatic_Impl.h"

//CPP_2_________________________________ This project's headers
#include "FileHelper.h"
#include "Row.h"
#include "TableColumnRow.h"
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

TableStatic_Impl::TableStatic_Impl(const TableColumnRow& columnRow, size_t rowCount)
    : m_columnRow(columnRow)
    , m_memory(nullptr)
    , m_colSize(sizeof(size_t) + CSV_FILED_MAX_SIZE * sizeof(TCHAR))
    , m_rowSize(columnRow.GetRow().GetCount() * m_colSize)
    , m_rowCount(rowCount)
{
    size_t memorySize = m_rowSize * m_rowCount;
    m_memory = new BYTE[memorySize];
    memset(m_memory, 0, memorySize);

    m_memoryLinkF = m_memory;
    m_memoryRowF = m_memory + sizeof(size_t);
}

TableStatic_Impl::~TableStatic_Impl()
{
    delete[] m_memory;
}

void TableStatic_Impl::SetFilePath(LPCTSTR filePath)
{
    m_filePath = filePath;
}

void TableStatic_Impl::SetItemValue(size_t row, size_t columnKey, double value)
{
    size_t col = m_columnRow.GetColumnIndex(columnKey);
    TCHAR* rowData = (TCHAR*)(m_memoryRowF + m_rowSize * row + col * m_colSize);
    _sntprintf_s(rowData, CSV_FILED_MAX_SIZE, CSV_FILED_MAX_SIZE - 1, m_columnRow.GetFloatFmt(columnKey), value);
}

void TableStatic_Impl::SetItemValue(size_t row, size_t columnKey, float value)
{
    size_t col = m_columnRow.GetColumnIndex(columnKey);
    TCHAR* rowData = (TCHAR*)(m_memoryRowF + m_rowSize * row + col * m_colSize);
    _sntprintf_s(rowData, CSV_FILED_MAX_SIZE, CSV_FILED_MAX_SIZE - 1, m_columnRow.GetFloatFmt(columnKey), value);
}

void TableStatic_Impl::SetItemValue(size_t row, size_t columnKey, int32_t value)
{
    size_t col = m_columnRow.GetColumnIndex(columnKey);
    TCHAR* rowData = (TCHAR*)(m_memoryRowF + m_rowSize * row + col * m_colSize);
    _sntprintf_s(rowData, CSV_FILED_MAX_SIZE, CSV_FILED_MAX_SIZE - 1, _T("%d"), value);
}

void TableStatic_Impl::SetItemValue(size_t row, size_t columnKey, LPCTSTR value)
{
    size_t col = m_columnRow.GetColumnIndex(columnKey);
    TCHAR* rowData = (TCHAR*)(m_memoryRowF + m_rowSize * row + col * m_colSize);
    _sntprintf_s(rowData, CSV_FILED_MAX_SIZE, CSV_FILED_MAX_SIZE - 1, value);
}

void TableStatic_Impl::SetItemFmt(size_t row, size_t columnKey, LPCTSTR fmt, va_list& argptr)
{
    size_t col = m_columnRow.GetColumnIndex(columnKey);
    TCHAR* rowData = (TCHAR*)(m_memoryRowF + m_rowSize * row + col * m_colSize);

    _vsntprintf_s(rowData, CSV_FILED_MAX_SIZE, CSV_FILED_MAX_SIZE - 1, fmt, argptr);
}

bool TableStatic_Impl::Save()
{
    CFileHelper resFile;

    if (!resFile.Open(m_filePath, CFileHelper::modeNoTruncate | CFileHelper::modeWrite))
    {
        // 파일이 없으면 UniCode 정보를 먼저 써주자
        if (!resFile.Open(m_filePath, CFileHelper::modeCreate | CFileHelper::modeNoTruncate | CFileHelper::modeWrite))
        {
            return false;
        }

        CArchive ar(&resFile, CArchive::store);

        BYTE wd[] = {0xef, 0xbb, 0xbf};
        ar.Write(wd, sizeof(wd));
    }

    CArchive ar(&resFile, CArchive::store);
    resFile.SeekToEnd();

    CString text;
    text.Preallocate(static_cast<int>((m_rowCount + 1) * 255));

    m_columnRow.AppendOut(text);

    size_t columnCount = m_columnRow.GetRow().GetCount();

    for (size_t row = 0; row < m_rowCount; row++)
    {
        text += _T("\r\n");

        size_t offset = m_rowSize * row;

        for (size_t col = 0; col < columnCount; col++, offset += m_colSize)
        {
            const TCHAR* rowData = (TCHAR*)(m_memoryRowF + offset);

            if (col == 0)
            {
                text += rowData;
            }
            else
            {
                text.AppendFormat(_T(",%s"), rowData);
            }
        }
    }

    // UTF8로 컨버팅
    int allocLen = WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)text, -1, NULL, 0, NULL, NULL);
    CHAR* utf8Text = new CHAR[allocLen];
    memset(utf8Text, 0, allocLen);
    WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)text, -1, utf8Text, allocLen, NULL, NULL);
    ar.Write(utf8Text, allocLen - 1);
    delete[] utf8Text;

    ar.Close();
    resFile.Close();

    return true;
}

size_t TableStatic_Impl::GetRowCount() const
{
    return m_rowCount;
}

LPCTSTR TableStatic_Impl::GetSaveFilePath() const
{
    return m_filePath;
}
