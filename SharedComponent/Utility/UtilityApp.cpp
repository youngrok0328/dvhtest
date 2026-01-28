//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "UtilityApp.h"

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
BEGIN_MESSAGE_MAP(UtilityApp, CWinApp)
END_MESSAGE_MAP()

// UtilityApp 생성
namespace Ipsc
{
void InitializeAsyncWindowMessage();
void CleanupAsyncWindowMessage();

void InitializeDirectoryPaths();
} // namespace Ipsc

UtilityApp::UtilityApp()
{
    Ipsc::InitializeAsyncWindowMessage();
    Ipsc::InitializeDirectoryPaths();
}

UtilityApp::~UtilityApp()
{
    Ipsc::CleanupAsyncWindowMessage();
}

// 유일한 UtilityApp 개체입니다.

UtilityApp theApp;

// UtilityApp 초기화

BOOL UtilityApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}
