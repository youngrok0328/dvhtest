//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PersistenceApp.h"

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
BEGIN_MESSAGE_MAP(CPersistenceApp, CWinApp)
END_MESSAGE_MAP()

// CPersistenceApp 생성

CPersistenceApp::CPersistenceApp()
{
    // InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 CPersistenceApp 개체입니다.

CPersistenceApp theApp;

// CPersistenceApp 초기화

BOOL CPersistenceApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}
