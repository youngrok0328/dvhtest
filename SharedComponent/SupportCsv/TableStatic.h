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
class TableStatic_Impl;
} // namespace SupportCSV

typedef int int32_t;

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class __INTEKPLUS_SHARED_SUPPORTCSV_CLASS__ TableStatic : public TableWrapper
{
public:
    TableStatic(const TableColumnRow& columnRow, size_t rowCount = 0);
    ~TableStatic() override;

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
    TableStatic_Impl* m_impl;
};
} // namespace SupportCSV
