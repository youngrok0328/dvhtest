#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace SupportCSV
{
class File;
class TableColumnRow;
} // namespace SupportCSV

typedef int int32_t;

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class Table_Impl
{
public:
    Table_Impl(const TableColumnRow& columnRow, size_t rowCount);
    ~Table_Impl();

    void Reset();
    size_t InsertRow(size_t nRow);
    size_t AddRow();
    size_t AddRows(size_t nRowCount);
    bool SaveAndRowDelete();

    void SetFilePath(LPCTSTR filePath);
    void SetItemValue(size_t nRow, size_t nColumnKey, double fValue);
    void SetItemValue(size_t nRow, size_t nColumnKey, float fValue);
    void SetItemValue(size_t nRow, size_t nColumnKey, int32_t nValue);
    void SetItemValue(size_t nRow, size_t nColumnKey, LPCTSTR szText);

    bool Save();
    size_t GetRowCount() const;
    LPCTSTR GetSaveFilePath() const;

private:
    const TableColumnRow& m_columnRow;
    File* m_pFile;

    bool Save(bool bSaveRowDelete);
};
} // namespace SupportCSV
