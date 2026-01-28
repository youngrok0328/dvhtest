//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ImageFileParser.h"

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
CString ImageFileParser::GetFileName(LPCTSTR filePath)
{
    CString fullFilename = filePath;
    long nIndex = fullFilename.ReverseFind(_T('\\'));
    if (nIndex == -1)
    {
        return fullFilename;
    }

    return fullFilename.Mid(nIndex + 1);
}

CString ImageFileParser::GetFolder(LPCTSTR filePath)
{
    CString fullFilename = filePath;
    long nIndex = fullFilename.ReverseFind(_T('\\'));
    if (nIndex == -1)
    {
        return _T("");
    }

    return fullFilename.Mid(0, nIndex);
}

long ImageFileParser::GetTrayID(LPCTSTR filePath)
{
    CString fileName = GetFileName(filePath);
    long trayID = 0;

    long trayID_S = fileName.Find(_T("Tray["));
    long trayID_E = fileName.Find(_T("]"), trayID_S + 5);

    if (trayID_S >= 0 && trayID_E >= 0)
    {
        trayID = _ttoi(fileName.Mid(trayID_S + 5, trayID_E - trayID_S - 5));
    }

    return trayID;
}

long ImageFileParser::GetScanID(LPCTSTR filePath)
{
    CString fileName = GetFileName(filePath);

    long scanIndex = 0;
    long scanAreaS = fileName.Find(_T("ScanArea["));
    long scanAreaE = fileName.Find(_T("]"), scanAreaS + 9);

    if (scanAreaS >= 0 && scanAreaE >= 0)
    {
        CString scanAreaText = fileName.Mid(scanAreaS + 9, scanAreaE - scanAreaS - 9);
        scanIndex = _ttoi(scanAreaText);
    }

    return scanIndex;
}
