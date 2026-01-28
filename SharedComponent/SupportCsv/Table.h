#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "TableWrapper.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace SupportCSV
{
class TableColumnRow;
class Table_Impl;
} // namespace SupportCSV

typedef int int32_t;

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class __INTEKPLUS_SHARED_SUPPORTCSV_CLASS__ Table : public TableWrapper
{
public:
    Table(const TableColumnRow& columnRow, size_t rowCount = 0);
    ~Table() override;

    void Reset();
    size_t InsertRow(size_t nRow);
    size_t AddRow();
    size_t AddRows(size_t nRowCount);
    bool SaveAndRowDelete();

    void SetFilePath(LPCTSTR filePath) override;
    void SetItemValue(size_t nRow, size_t nColumnKey, double fValue) override;
    void SetItemValue(size_t nRow, size_t nColumnKey, float fValue) override;
    void SetItemValue(size_t nRow, size_t nColumnKey, int32_t nValue) override;
    void SetItemValue(size_t nRow, size_t nColumnKey, LPCTSTR szText) override;
    void SetItemFmt(size_t nRow, size_t nColumnKey, LPCTSTR szFmt, ...) override;

    bool Save() override;
    size_t GetRowCount() const override;
    LPCTSTR GetSaveFilePath() const override;

private:
    Table_Impl* m_impl;
};
} // namespace SupportCSV
