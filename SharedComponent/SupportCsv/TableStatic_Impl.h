#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace SupportCSV
{
class TableColumnRow;
}

typedef int int32_t;

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class TableStatic_Impl
{
public:
    TableStatic_Impl(const TableColumnRow& columnRow, size_t rowCount);
    ~TableStatic_Impl();

    void SetFilePath(LPCTSTR filePath);
    void SetItemValue(size_t row, size_t columnKey, double value);
    void SetItemValue(size_t row, size_t columnKey, float value);
    void SetItemValue(size_t row, size_t columnKey, int32_t value);
    void SetItemValue(size_t row, size_t columnKey, LPCTSTR value);
    void SetItemFmt(size_t row, size_t columnKey, LPCTSTR fmt, va_list& argptr);

    bool Save();
    size_t GetRowCount() const;
    LPCTSTR GetSaveFilePath() const;

private:
    BYTE* m_memory;
    BYTE* m_memoryRowF;
    BYTE* m_memoryLinkF;

    size_t m_colSize;
    size_t m_rowSize;
    size_t m_rowCount;
    CString m_filePath;

    const TableColumnRow& m_columnRow;
};
} // namespace SupportCSV
