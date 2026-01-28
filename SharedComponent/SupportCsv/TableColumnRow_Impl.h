#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Row.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class TableColumnRow_Impl
{
public:
    TableColumnRow_Impl();
    ~TableColumnRow_Impl();

    void AddColumn(size_t nColumnKey, LPCTSTR szName, int32_t nDecimalPlaces = -1);
    size_t GetColumnIndex(size_t nColumnKey) const;
    Row& GetRow();
    const Row& GetRow() const;
    LPCTSTR GetFloatFmt(size_t nColumnKey) const;

    void AppendOut(CString& strText) const;

private:
    Row m_Row;

    std::vector<CString> m_vecColumn_FloatFormat;
    std::vector<size_t> m_vecColumnKey;
};
} // namespace SupportCSV
