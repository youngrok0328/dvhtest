//HDR_0_________________________________ Configuration header
#include "stdafx.h"

//HDR_1_________________________________ This project's headers
#include "VisionInspectionGlassSubstrate2DApp.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//HDR_6_________________________________ Header body
//
BEGIN_MESSAGE_MAP(VisionInspectionGlassSubstrate2DApp, CWinApp)
END_MESSAGE_MAP()

VisionInspectionGlassSubstrate2DApp::VisionInspectionGlassSubstrate2DApp()
{
}

VisionInspectionGlassSubstrate2DApp theApp;

BOOL VisionInspectionGlassSubstrate2DApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}
