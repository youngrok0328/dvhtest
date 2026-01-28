//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SharedMemForIO.h"

//CPP_2_________________________________ This project's headers
#include "HandlerInputOutput.h"
#include "VisionInputOutput.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CSharedMemForIOApp, CWinApp)
END_MESSAGE_MAP()

namespace Ipvm
{
void InitializeStaticResourceSharedMemoryResourceHandleMap();
void TerminateStaticResourceSharedMemoryResourceHandleMap();
} // namespace Ipvm

CSharedMemForIOApp::CSharedMemForIOApp()
{
    Ipvm::InitializeStaticResourceSharedMemoryResourceHandleMap();
}

CSharedMemForIOApp::~CSharedMemForIOApp()
{
    Ipvm::TerminateStaticResourceSharedMemoryResourceHandleMap();
}

CSharedMemForIOApp theApp;

BOOL CSharedMemForIOApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}
