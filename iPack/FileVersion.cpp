//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FileVersion.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "version")

FileVersion::FileVersion()
{
    m_lpVersionData = NULL;
    m_dwLangCharset = 0;
}

FileVersion::~FileVersion()
{
    Close();
}

void FileVersion::Close()
{
    delete[] m_lpVersionData;
    m_lpVersionData = NULL;
    m_dwLangCharset = 0;
}

BOOL FileVersion::Open(LPCTSTR lpszModuleName)
{
    ASSERT(_tcslen(lpszModuleName) > 0);
    ASSERT(m_lpVersionData == NULL);

    // Get the version information size for allocate the buffer
    DWORD dwHandle;
    DWORD dwDataSize = ::GetFileVersionInfoSize((LPTSTR)lpszModuleName, &dwHandle);
    if (dwDataSize == 0)
        return FALSE;

    // Allocate buffer and retrieve version information
    m_lpVersionData = new BYTE[dwDataSize];
    if (!::GetFileVersionInfo((LPTSTR)lpszModuleName, dwHandle, dwDataSize, (void**)m_lpVersionData))
    {
        Close();
        return FALSE;
    }

    // Retrieve the first language and character-set identifier
    UINT nQuerySize;
    DWORD* pTransTable;
    if (!::VerQueryValue(m_lpVersionData, _T("\\VarFileInfo\\Translation"), (void**)&pTransTable, &nQuerySize))
    {
        Close();
        return FALSE;
    }

    // Swap the words to have lang-charset in the correct format
    m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));

    return TRUE;
}

CString FileVersion::QueryValue(LPCTSTR lpszValueName, DWORD dwLangCharset /* = 0*/)
{
    // Must call Open() first
    ASSERT(m_lpVersionData != NULL);
    if (m_lpVersionData == NULL)
        return (CString) _T("");

    // If no lang-charset specified use default
    if (dwLangCharset == 0)
        dwLangCharset = m_dwLangCharset;

    // Query version information value
    UINT nQuerySize;
    LPVOID lpData;
    CString strValue, strBlockName;
    strBlockName.Format(_T("\\StringFileInfo\\%08lx\\%s"), dwLangCharset, lpszValueName);
    if (::VerQueryValue((void**)m_lpVersionData, strBlockName.GetBuffer(0), &lpData, &nQuerySize))
        strValue = (LPCTSTR)lpData;

    strBlockName.ReleaseBuffer();

    return strValue;
}

BOOL FileVersion::GetFixedInfo(VS_FIXEDFILEINFO& vsffi)
{
    // Must call Open() first
    ASSERT(m_lpVersionData != NULL);
    if (m_lpVersionData == NULL)
        return FALSE;

    UINT nQuerySize;
    VS_FIXEDFILEINFO* pVsffi;
    if (::VerQueryValue((void**)m_lpVersionData, _T("\\"), (void**)&pVsffi, &nQuerySize))
    {
        vsffi = *pVsffi;
        return TRUE;
    }

    return FALSE;
}

CString FileVersion::GetFixedFileVersion()
{
    CString sVersion;
    VS_FIXEDFILEINFO vsffi;

    if (GetFixedInfo(vsffi))
    {
        sVersion.Format(_T("%u,%u,%u,%u"), HIWORD(vsffi.dwFileVersionMS), LOWORD(vsffi.dwFileVersionMS),
            HIWORD(vsffi.dwFileVersionLS), LOWORD(vsffi.dwFileVersionLS));
    }
    return sVersion;
}

CString FileVersion::GetFixedProductVersion()
{
    CString sVersion;
    VS_FIXEDFILEINFO vsffi;

    if (GetFixedInfo(vsffi))
    {
        sVersion.Format(_T("%u,%u,%u,%u"), HIWORD(vsffi.dwProductVersionMS), LOWORD(vsffi.dwProductVersionMS),
            HIWORD(vsffi.dwProductVersionLS), LOWORD(vsffi.dwProductVersionLS));
    }
    return sVersion;
}

CString FileVersion::GetFileVersion2()
{
    CString strFileVersion = GetFileVersion();

    strFileVersion.Remove(' ');
    strFileVersion.Replace(',', '.');

    // Detail Version Information
    int pos0 = 0;
    int pos1 = strFileVersion.Find('.', pos0);
    int iVer1 = _ttoi(strFileVersion.Mid(pos0, pos1 - pos0));

    pos0 = pos1 + 1;
    pos1 = strFileVersion.Find('.', pos0);
    int iVer2 = _ttoi(strFileVersion.Mid(pos0, pos1 - pos0));

    pos0 = pos1 + 1;
    pos1 = strFileVersion.Find('.', pos0);
    int iVer3 = _ttoi(strFileVersion.Mid(pos0, pos1 - pos0));

    int iVer4 = _ttoi(strFileVersion.Right(strFileVersion.GetLength() - pos1 - 1));

#ifdef _DEBUG
    if (IsDebuggerPresent())
        strFileVersion.Format(_T("%d.%02d.%02d.%04dD"), iVer1, iVer2, iVer3, iVer4);
    else
        strFileVersion.Format(_T("%d.%02d.%02d.%04d"), iVer1, iVer2, iVer3, iVer4);
#else
    if (IsDebuggerPresent())
        strFileVersion.Format(_T("%d.%02d.%02d.%04dR"), iVer1, iVer2, iVer3, iVer4);
    else
        strFileVersion.Format(_T("%d.%02d.%02d.%04d"), iVer1, iVer2, iVer3, iVer4);
#endif

    return strFileVersion;
}
