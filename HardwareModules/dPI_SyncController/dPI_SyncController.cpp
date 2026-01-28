//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "dPI_SyncController.h"

//CPP_2_________________________________ This project's headers
#include "VirtualVisionIO.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BEGIN_MESSAGE_MAP(CdPI_SyncControllerApp, CWinApp)
END_MESSAGE_MAP()

CdPI_SyncControllerApp::CdPI_SyncControllerApp()
{
}

CdPI_SyncControllerApp theApp;

BOOL CdPI_SyncControllerApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}

int CdPI_SyncControllerApp::ExitInstance()
{
    VirtualVisionIO::DelInstance();

    return CWinApp::ExitInstance();
}
