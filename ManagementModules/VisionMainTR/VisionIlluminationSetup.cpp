//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionIlluminationSetup.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionIllumSetup2D.h"
#include "DlgVisionIllumSetup3D.h"
#include "DlgVisionIllumSetupTR.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionIlluminationSetup::VisionIlluminationSetup(
    VisionMainTR& main, VisionUnit& visionUnit, HWND hwndParent, const enSideVisionModule i_eSideVisionModule)
    : m_ui2D(nullptr)
    , m_ui3D(nullptr)
    , m_uiTR(nullptr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWnd* parentWnd = CWnd::FromHandle(hwndParent);

    int iVisionType = SystemConfig::GetInstance().GetVisionType();

    switch (iVisionType)
    {
        case VISIONTYPE_2D_INSP:
            m_ui2D = new DlgVisionIllumSetup2D(main, visionUnit, parentWnd, i_eSideVisionModule);
            m_ui2D->Create(DlgVisionIllumSetup2D::IDD, parentWnd);
            break;

        case VISIONTYPE_SIDE_INSP:
            m_ui2D = new DlgVisionIllumSetup2D(main, visionUnit, parentWnd, i_eSideVisionModule);
            m_ui2D->Create(DlgVisionIllumSetup2D::IDD_SIDE, parentWnd);
            break;

        case VISIONTYPE_3D_INSP:
            m_ui3D = new DlgVisionIllumSetup3D(main, parentWnd);
            m_ui3D->Create(DlgVisionIllumSetup3D::IDD, parentWnd);
            break;

        case VISIONTYPE_TR:
            m_uiTR = new DlgVisionIllumSetupTR(main, visionUnit, parentWnd, i_eSideVisionModule);
            m_uiTR->Create(DlgVisionIllumSetupTR::IDD, parentWnd);
            break;

        default:
            break;
    }
}

VisionIlluminationSetup::~VisionIlluminationSetup()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_ui2D->GetSafeHwnd())
        m_ui2D->DestroyWindow();
    if (m_ui3D->GetSafeHwnd())
        m_ui3D->DestroyWindow();
    if (m_uiTR->GetSafeHwnd())
        m_uiTR->DestroyWindow();

    delete m_ui2D;
    delete m_ui3D;
    delete m_uiTR;
}

BOOL VisionIlluminationSetup::isJobOpenEnable()
{
    if (m_ui2D || m_uiTR)
        return true;
    return false;
}

BOOL VisionIlluminationSetup::isJobSaveEnable()
{
    if (m_ui2D || m_uiTR)
        return true;
    return false;
}

BOOL VisionIlluminationSetup::callJobOpen()
{
    if (m_ui2D)
    {
        return m_ui2D->callJobOpen();
    }
    else if (m_uiTR)
    {
        return m_uiTR->callJobOpen();
    }

    return false;
}

BOOL VisionIlluminationSetup::callJobSave()
{
    if (m_ui2D)
    {
        return m_ui2D->callJobSave();
    }
    else if (m_uiTR)
    {
        return m_uiTR->callJobSave();
    }

    return false;
}
