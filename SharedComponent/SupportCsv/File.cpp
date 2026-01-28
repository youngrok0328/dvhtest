//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "File.h"

//CPP_2_________________________________ This project's headers
#include "File_Impl.h"

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

File::File()
{
    m_impl = new File_Impl;
}

File::~File()
{
    delete m_impl;
}

void File::Reset()
{
    m_impl->Reset();
}

void File::SetFilePath(LPCTSTR filePath)
{
    m_impl->SetFilePath(filePath);
}

size_t File::InsertRow(size_t nRow, size_t nItemCount)
{
    return m_impl->InsertRow(nRow, nItemCount);
}

size_t File::AddRow(size_t nItemCount)
{
    return m_impl->AddRow(nItemCount);
}

size_t File::AddRows(size_t nRowCount, size_t nItemCount)
{
    return m_impl->AddRows(nRowCount, nItemCount);
}

bool File::Save()
{
    return m_impl->Save();
}

bool File::SaveAndRowDelete()
{
    return m_impl->SaveAndRowDelete();
}

Row& File::GetRow(size_t nRow)
{
    return m_impl->GetRow(nRow);
}

size_t File::GetRowCount() const
{
    return m_impl->GetRowCount();
}

size_t File::GetRowItemCount(size_t nRow) const
{
    return m_impl->GetRowItemCount(nRow);
}

LPCTSTR File::GetSaveFilePath() const
{
    return m_impl->GetSaveFilePath();
}
