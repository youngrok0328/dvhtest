#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Object.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace SupportCSV
{
class Row;
class File_Impl;
} // namespace SupportCSV

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class __INTEKPLUS_SHARED_SUPPORTCSV_CLASS__ File : public Object
{
public:
    File();
    ~File() override;

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
    File_Impl* m_impl;
};
} // namespace SupportCSV
