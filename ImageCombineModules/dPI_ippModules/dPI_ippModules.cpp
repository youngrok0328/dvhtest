//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "dPI_ippModules.h"

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
BEGIN_MESSAGE_MAP(CdPI_ippModulesApp, CWinApp)
END_MESSAGE_MAP()

CdPI_ippModulesApp::CdPI_ippModulesApp()
{
}

CdPI_ippModulesApp theApp;

BOOL CdPI_ippModulesApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}

int CdPI_ippModulesApp::ExitInstance()
{
    TRACE0("dPI_ippModules.DLL Terminating!\n");

    return CWinApp::ExitInstance();
}
