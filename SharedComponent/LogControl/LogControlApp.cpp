//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "LogControlApp.h"

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
BEGIN_MESSAGE_MAP(LogControlApp, CWinApp)
END_MESSAGE_MAP()

// LogControlApp 생성

LogControlApp::LogControlApp()
{
    // InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 LogControlApp 개체입니다.

LogControlApp theApp;

// LogControlApp 초기화

BOOL LogControlApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}
