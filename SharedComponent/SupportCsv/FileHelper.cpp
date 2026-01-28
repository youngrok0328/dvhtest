//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FileHelper.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GET_PATHNAME(x) ((x).Left((x).ReverseFind('\\')))

//CPP_7_________________________________ Implementation body
//
BOOL CreateDirectorys(CString strNewDirectory)
{
    int nPos = strNewDirectory.ReverseFind('\\');
    if (2 < nPos)
    {
        CString strParentDirectory = strNewDirectory.Left(nPos);
        if (GetFileAttributes(strParentDirectory) == -1)
        {
            CreateDirectorys(strParentDirectory);
        }
    }
    // 마지막 자기꺼만 성공하면 된거지.
    BOOL bOK = CreateDirectory(strNewDirectory, NULL);
    return bOK;
}

CFileHelper::CFileHelper()
    : CFile()
{
}
CFileHelper::CFileHelper(HANDLE hFile)
    : CFile(hFile)
{
}
CFileHelper::~CFileHelper(void)
{
}

BOOL CFileHelper::Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pException)
{
    CreateDirectorys(GET_PATHNAME(CString(lpszFileName)));
    return CFile::Open(lpszFileName, nOpenFlags, pException);
}
