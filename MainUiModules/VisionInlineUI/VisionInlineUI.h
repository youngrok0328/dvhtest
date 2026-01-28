#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u3.h> //kircheis_NGRVINLINE

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CVisionMain;
class VisionUnit;
class CDlgVisionInlineUI;
class VisionInspectionSpec;
class VisionInspectionResult;
class VisionDebugInfo;
class VisionMainAgent;

//HDR_6_________________________________ Header body
//
class __VISION_PRIMARY_UI_CLASS__ CVisionInlineUI
{
public:
    CVisionInlineUI(void);
    ~CVisionInlineUI(void);

    long OnThreadInspect(bool manualInsp);
    void ShowVisionPrimaryDlg();

    void SetVisionModules(VisionUnit* pVisionUnit);
    void SetVisionModules(CVisionMain* pVisionMain, VisionUnit* pVisionUnit, HWND hwndParent);

    CWnd* GetDlg()
    {
        return (CWnd*)m_pDlg;
    }

    VisionMainAgent* m_pVisionMainAgent;
    VisionUnit* m_pVisionUnit;

public:
    void ShowWindow(int nCmdShow);
    BOOL IsWindowVisible();

    void SetColorImageForNGRV(Ipvm::Image8u3 image); //kircheis_NGRVINLINE

private:
    CDlgVisionInlineUI* m_pDlg;
};
