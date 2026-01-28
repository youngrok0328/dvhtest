#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
namespace SupportCSV
{
class Row;
}

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class File_Impl
{
public:
    File_Impl();
    ~File_Impl();

    void Reset();
    void SetFilePath(LPCTSTR filePath);
    size_t InsertRow(size_t nRow, size_t nItemCount = 0);
    size_t AddRow(size_t nItemCount = 0);
    size_t AddRows(size_t nRowCount, size_t nItemCount = 0);

    bool Save();
    bool SaveAndRowDelete();
    Row& GetRow(size_t nRow);
    size_t GetRowCount() const;
    size_t GetRowItemCount(size_t nRow) const;
    LPCTSTR GetSaveFilePath() const;

private:
    std::vector<Row*> m_vecRow;

    size_t m_lastSaveRowIndex;
    CString m_filePath;

    void Free();
    bool Save(bool bSaveRowDelete);
};
} // namespace SupportCSV
