#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionMainMenuAccess.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionMainTR;
class VisionUnit;
class DlgVisionIllumSetup2D;
class DlgVisionIllumSetup3D;
class DlgVisionIllumSetupTR;

//HDR_6_________________________________ Header body
//
class VisionIlluminationSetup : public VisionMainMenuAccess
{
public:
    VisionIlluminationSetup(VisionMainTR& main, VisionUnit& visionUnit, HWND hwndParent,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    ~VisionIlluminationSetup();

    virtual BOOL isJobOpenEnable() override;
    virtual BOOL isJobSaveEnable() override;

    virtual BOOL callJobOpen() override;
    virtual BOOL callJobSave() override;

private:
    DlgVisionIllumSetup2D* m_ui2D;
    DlgVisionIllumSetup3D* m_ui3D;
    DlgVisionIllumSetupTR* m_uiTR;
};
