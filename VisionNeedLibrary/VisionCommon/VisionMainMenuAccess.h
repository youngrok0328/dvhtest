#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionMainMenuAccess
{
public:
    VisionMainMenuAccess();
    virtual ~VisionMainMenuAccess();

    virtual BOOL isInspectionEnable();
    virtual BOOL isGrabEnable();
    virtual BOOL isJobOpenEnable();
    virtual BOOL isJobSaveEnable();
    virtual BOOL isImageOpenEnable();
    virtual BOOL isImageSaveEnable();

    virtual BOOL callInspection();
    virtual BOOL callInspection(long nRunMode);
    virtual BOOL callGrab();
    virtual BOOL callJobOpen();
    virtual BOOL callJobSave();
    virtual BOOL callImageOpen();
    virtual BOOL callImagesOpen(std::vector<CString> ImagePaths);
    virtual BOOL callImageSave();
    virtual void callQuit();
};
