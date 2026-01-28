#pragma once

#ifndef __AFXWIN_H__
#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CiTRVApp : public CWinApp
{
public:
    CiTRVApp();

    static void UserExceptionHandlerPre(void* callbackData);
    static void UserExceptionHandlerPost(
        void* callbackData, bool success, const wchar_t* dumpPathName, const wchar_t* message);

    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // 구현입니다.

    DECLARE_MESSAGE_MAP()

private:
    ULONG_PTR m_gdiToken;
    Gdiplus::GdiplusStartupInput g_gdiInput;
};

extern CiTRVApp theApp;
