//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingAlign3DApp.h"

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
BEGIN_MESSAGE_MAP(CVisionProcessingAlign3DApp, CWinApp)
END_MESSAGE_MAP()

CVisionProcessingAlign3DApp::CVisionProcessingAlign3DApp()
{
}

CVisionProcessingAlign3DApp theApp;

BOOL CVisionProcessingAlign3DApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}
