//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInlineUIApp.h"

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
BEGIN_MESSAGE_MAP(CVisionInlineUIApp, CWinApp)
END_MESSAGE_MAP()

CVisionInlineUIApp::CVisionInlineUIApp()
{
}

CVisionInlineUIApp theApp;

BOOL CVisionInlineUIApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}

int CVisionInlineUIApp::ExitInstance()
{
    TRACE0("VisionInlineUI.DLL Terminating!\n");

    return CWinApp::ExitInstance();
}
