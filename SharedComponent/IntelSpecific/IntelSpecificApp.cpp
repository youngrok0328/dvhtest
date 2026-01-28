//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "IntelSpecificApp.h"

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
BEGIN_MESSAGE_MAP(IntelSpecificApp, CWinApp)
END_MESSAGE_MAP()

// IntelSpecificApp 생성

IntelSpecificApp::IntelSpecificApp()
{
    // InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 IntelSpecificApp 개체입니다.

IntelSpecificApp theApp;

// IntelSpecificApp 초기화

BOOL IntelSpecificApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}
