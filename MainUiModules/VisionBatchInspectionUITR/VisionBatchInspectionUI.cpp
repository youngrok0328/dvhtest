//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionBatchInspectionUI.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionBatchInspectionUI.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionBatchInspectionUI::VisionBatchInspectionUI(
    VisionMainAgent& visionMainAgent, VisionUnit& visionUnit, HWND hwndParent)
    : m_pDlg(nullptr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWnd* parentWnd = CWnd::FromHandle(hwndParent);

    m_pDlg = new DlgVisionBatchInspectionUI(visionMainAgent, visionUnit, parentWnd);
    m_pDlg->Create(DlgVisionBatchInspectionUI::IDD, parentWnd);
    m_pDlg->OnJobChanged();
    m_pDlg->ShowWindow(SW_SHOW);
}

VisionBatchInspectionUI::~VisionBatchInspectionUI(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pDlg->GetSafeHwnd())
    {
        m_pDlg->DestroyWindow();
    }

    delete m_pDlg;
}

BOOL VisionBatchInspectionUI::isInspectionEnable()
{
    return m_pDlg->isInspectionEnable();
}

BOOL VisionBatchInspectionUI::isImageOpenEnable()
{
    return m_pDlg->isImageOpenEnable();
}

BOOL VisionBatchInspectionUI::callInspection()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_pDlg->call_inspection();
    return TRUE;
}

BOOL VisionBatchInspectionUI::callInspection(long nRunMode)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_pDlg->call_inspection(nRunMode);
    return TRUE;
}

BOOL VisionBatchInspectionUI::callImageOpen()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_pDlg->call_openImageFiles();
    return TRUE;
}

BOOL VisionBatchInspectionUI::callImagesOpen(std::vector<CString> ImagePaths)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_pDlg->call_openImageFiles(ImagePaths);
    return TRUE;
}

void VisionBatchInspectionUI::callQuit()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_pDlg->call_quit();
}
