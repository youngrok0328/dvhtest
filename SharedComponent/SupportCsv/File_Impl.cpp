//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "File_Impl.h"

//CPP_2_________________________________ This project's headers
#include "FileHelper.h"
#include "Row.h"

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

File_Impl::File_Impl()
    : m_lastSaveRowIndex(0)
{
    m_vecRow.reserve(100000);
}

File_Impl::~File_Impl()
{
    Free();
}

void File_Impl::Reset()
{
    Free();
}

void File_Impl::SetFilePath(LPCTSTR filePath)
{
    m_filePath = filePath;
}

size_t File_Impl::InsertRow(size_t nRow, size_t nItemCount)
{
    m_vecRow.insert(m_vecRow.begin() + nRow, new Row(nItemCount));

    return nRow;
}

size_t File_Impl::AddRow(size_t nItemCount)
{
    m_vecRow.push_back(new Row(nItemCount));

    return m_vecRow.size() - 1;
}

size_t File_Impl::AddRows(size_t nRowCount, size_t nItemCount)
{
    size_t nStartIndex = m_vecRow.size();

    m_vecRow.resize(nStartIndex + nRowCount);

    for (size_t n = 0; n < nRowCount; n++)
    {
        m_vecRow[nStartIndex + n] = new Row(nItemCount);
    }

    return nStartIndex;
}

bool File_Impl::Save()
{
    return Save(false);
}

bool File_Impl::SaveAndRowDelete()
{
    return Save(true);
}

Row& File_Impl::GetRow(size_t nRow)
{
    return *m_vecRow[nRow];
}

size_t File_Impl::GetRowCount() const
{
    return m_vecRow.size();
}

size_t File_Impl::GetRowItemCount(size_t nRow) const
{
    return m_vecRow[nRow]->GetCount();
}

LPCTSTR File_Impl::GetSaveFilePath() const
{
    return m_filePath;
}

void File_Impl::Free()
{
    for (auto* p : m_vecRow)
    {
        delete p;
    }

    m_vecRow.clear();
    m_lastSaveRowIndex = 0;
}

bool File_Impl::Save(bool bSaveRowDelete)
{
    size_t nRowCount = m_vecRow.size();

    CString text;
    text.Preallocate(static_cast<int32_t>((nRowCount - m_lastSaveRowIndex) * 255));

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
    else
    {
        if (nRowCount - m_lastSaveRowIndex > 0)
        {
            text += _T("\r\n");
        }
    }

    CArchive ar(&resFile, CArchive::store);
    resFile.SeekToEnd();

    for (size_t nRow = m_lastSaveRowIndex; nRow < nRowCount; nRow++)
    {
        Row& row = *m_vecRow[nRow];
        row.AppendOut(text);

        m_lastSaveRowIndex = nRow;
        if (nRow != nRowCount - 1)
            text += _T("\r\n");
    }

    int allocLen = WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)text, -1, NULL, 0, NULL, NULL);
    CHAR* utf8Text = new CHAR[allocLen];
    memset(utf8Text, 0, allocLen);
    WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)text, -1, utf8Text, allocLen, NULL, NULL);
    ar.Write(utf8Text, allocLen - 1);
    delete[] utf8Text;

    ar.Close();
    resFile.Close();

    if (bSaveRowDelete)
    {
        // 기존 Row데이터를 모두 날려버리자
        Reset();
    }

    return true;
}
