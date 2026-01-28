//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionMainMenuAccess.h"

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
VisionMainMenuAccess::VisionMainMenuAccess()
{
}

VisionMainMenuAccess::~VisionMainMenuAccess()
{
}

BOOL VisionMainMenuAccess::isInspectionEnable()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::isGrabEnable()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::isJobOpenEnable()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::isJobSaveEnable()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::isImageOpenEnable()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::isImageSaveEnable()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::callInspection()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::callInspection(
    long nRunMode) // Automation에서 Batch inspection의 All inspection에 접근하기 위한 함수
{
    UNREFERENCED_PARAMETER(nRunMode);

    return FALSE;
}

BOOL VisionMainMenuAccess::callGrab()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::callJobOpen()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::callJobSave()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::callImageOpen()
{
    return FALSE;
}

BOOL VisionMainMenuAccess::callImagesOpen(
    std::vector<CString> ImagePaths) // Automation에서 Batch inspection에서 여러장의 이미지를 open 하기 위한 함수
{
    return FALSE;
}

BOOL VisionMainMenuAccess::callImageSave()
{
    return FALSE;
}

void VisionMainMenuAccess::callQuit()
{
}
