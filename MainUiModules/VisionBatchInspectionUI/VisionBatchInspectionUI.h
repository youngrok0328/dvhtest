#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionMainMenuAccess.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionMainAgent;
class VisionUnit;
class DlgVisionBatchInspectionUI;

//HDR_6_________________________________ Header body
//
class __VISION_BATCH_INSPECTION_UI_CLASS__ VisionBatchInspectionUI : public VisionMainMenuAccess
{
public:
    VisionBatchInspectionUI(void) = delete;
    VisionBatchInspectionUI(VisionMainAgent& visionMainAgent, VisionUnit& visionUnit, HWND hwndParent);
    virtual ~VisionBatchInspectionUI(void);

    virtual BOOL isInspectionEnable() override;
    virtual BOOL isImageOpenEnable() override;

    virtual BOOL callInspection() override;
    virtual BOOL callInspection(long nRunMode) override;
    virtual BOOL callImageOpen() override;
    virtual BOOL callImagesOpen(std::vector<CString> ImagePaths) override;
    virtual void callQuit() override;

private:
    DlgVisionBatchInspectionUI* m_pDlg;
};
