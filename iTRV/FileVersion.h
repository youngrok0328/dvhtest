#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class FileVersion
{
public:
    FileVersion();
    ~FileVersion();

public:
    BOOL Open(LPCTSTR lpszModuleName);
    void Close();

    CString QueryValue(LPCTSTR lpszValueName, DWORD dwLangCharset = 0);
    CString GetFileDescription()
    {
        return QueryValue(_T("FileDescription"));
    };
    CString GetFileVersion()
    {
        return QueryValue(_T("FileVersion"));
    };
    CString GetFileVersion2();
    CString GetInternalName()
    {
        return QueryValue(_T("InternalName"));
    };
    CString GetCompanyName()
    {
        return QueryValue(_T("CompanyName"));
    };
    CString GetLegalCopyright()
    {
        return QueryValue(_T("LegalCopyright"));
    };
    CString GetOriginalFilename()
    {
        return QueryValue(_T("OriginalFilename"));
    };
    CString GetProductName()
    {
        return QueryValue(_T("ProductName"));
    };
    CString GetProductVersion()
    {
        return QueryValue(_T("ProductVersion"));
    };
    CString GetSpecialBuild()
    {
        return QueryValue(_T("SpecialBuild"));
    };

    BOOL GetFixedInfo(VS_FIXEDFILEINFO& vsffi);
    CString GetFixedFileVersion();
    CString GetFixedProductVersion();

    // Attributes

protected:
    LPBYTE m_lpVersionData;
    DWORD m_dwLangCharset;
};
