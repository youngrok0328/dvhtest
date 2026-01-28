#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CFileHelper : public CFile
{
public:
    CFileHelper();
    CFileHelper(HANDLE hFile);
    CFileHelper(LPCTSTR lpszFileName, UINT nOpenFlags) = delete;
    ~CFileHelper(void);

    BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL) override;
};
