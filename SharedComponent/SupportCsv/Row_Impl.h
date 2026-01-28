#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace SupportCSV
{
class Row_Impl
{
public:
    Row_Impl(size_t itemCount);
    ~Row_Impl();

    size_t AddTxt(LPCTSTR text); // 속도느림. 자주 쓰지 말것
    void SetTxt(size_t nItem, LPCTSTR szText);
    void SetFmt(size_t nItem, LPCTSTR szFmt, va_list& argptr);

    size_t GetCount() const;
    const TCHAR* GetTxt(size_t nItem) const;

    void AppendOut(CString& strText) const;

private:
    TCHAR* m_text;
    size_t m_itemCount;
};
} // namespace SupportCSV
