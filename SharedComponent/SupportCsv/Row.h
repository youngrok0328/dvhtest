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
class Row_Impl;
}

//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class __INTEKPLUS_SHARED_SUPPORTCSV_CLASS__ Row : public Object
{
public:
    Row(size_t itemCount = 0);
    ~Row() override;

    size_t AddTxt(LPCTSTR text); // 속도느림. 자주 쓰지 말것
    void SetTxt(size_t nItem, LPCTSTR szText);
    void SetFmt(size_t nItem, LPCTSTR szFmt, ...);

    size_t GetCount() const;
    const TCHAR* GetTxt(size_t nItem) const;

    void AppendOut(CString& strText) const;

private:
    Row_Impl* m_impl;
};
} // namespace SupportCSV
