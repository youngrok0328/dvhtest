#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Object.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
typedef int int32_t;

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class __INTEKPLUS_SHARED_SUPPORTCSV_CLASS__ TableWrapper : public Object
{
public:
    virtual void SetFilePath(LPCTSTR filePath) = 0;
    virtual void SetItemValue(size_t row, size_t columnKey, double value) = 0;
    virtual void SetItemValue(size_t row, size_t columnKey, float value) = 0;
    virtual void SetItemValue(size_t row, size_t columnKey, int32_t value) = 0;
    virtual void SetItemValue(size_t row, size_t columnKey, LPCTSTR value) = 0;
    virtual void SetItemFmt(size_t row, size_t columnKey, LPCTSTR fmt, ...) = 0;

    virtual bool Save() = 0;
    virtual size_t GetRowCount() const = 0;
    virtual LPCTSTR GetSaveFilePath() const = 0;
};
} // namespace SupportCSV
