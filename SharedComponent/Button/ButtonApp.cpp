//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ButtonApp.h"

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
BEGIN_MESSAGE_MAP(CButtonApp, CWinApp)
END_MESSAGE_MAP()

// CButtonApp 생성

CButtonApp::CButtonApp()
{
    // InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 CButtonApp 개체입니다.

CButtonApp theApp;

// CButtonApp 초기화

BOOL CButtonApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}
