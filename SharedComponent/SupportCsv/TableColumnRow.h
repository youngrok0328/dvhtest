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
class TableColumnRow_Impl;
} // namespace SupportCSV
typedef int int32_t;

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class __INTEKPLUS_SHARED_SUPPORTCSV_CLASS__ TableColumnRow : public Object
{
public:
    TableColumnRow();
    ~TableColumnRow() override;

    void AddColumn(size_t nColumnKey, LPCTSTR szName, int32_t nDecimalPlaces = -1);
    size_t GetColumnIndex(size_t nColumnKey) const;
    Row& GetRow();
    const Row& GetRow() const;
    LPCTSTR GetFloatFmt(size_t nColumnKey) const;

    void AppendOut(CString& strText) const;

private:
    TableColumnRow_Impl* m_impl;
};
} // namespace SupportCSV
