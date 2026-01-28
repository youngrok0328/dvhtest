//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInlineUI.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInlineUI.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionMainTR/VisionMainTR.h"
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CVisionInlineUI::CVisionInlineUI(void)
    : m_pVisionMainAgent(nullptr)
    , m_pVisionUnit(nullptr)
    , m_pDlg(nullptr)
{
}

CVisionInlineUI::~CVisionInlineUI(void)
{
    if (m_pDlg != NULL)
    {
        m_pDlg->DestroyWindow();
        delete m_pDlg;
        m_pDlg = NULL;
    }
}

long CVisionInlineUI::OnThreadInspect(bool manualInsp)
{
    return m_pDlg->OnThreadInspect(manualInsp);
}

void CVisionInlineUI::ShowVisionPrimaryDlg()
{
    m_pDlg->ShowWindow(SW_SHOW);
}

void CVisionInlineUI::SetVisionModules(VisionUnit* pVisionUnit)
{
    m_pVisionUnit = pVisionUnit;
}

void CVisionInlineUI::SetVisionModules(VisionMainTR* pVisionMain, VisionUnit* pVisionUnit, HWND hwndParent)
{
    m_pVisionMainAgent = pVisionMain;
    m_pVisionUnit = pVisionUnit;

    // 이 코드가 없으면 Dialog 가 보이지 않음
    if (m_pDlg == nullptr)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CWnd* parentWnd = CWnd::FromHandle(hwndParent);

        m_pDlg = new CDlgVisionInlineUI(*this, parentWnd);
        m_pDlg->Create(CDlgVisionInlineUI::IDD, parentWnd);
    }
}

void CVisionInlineUI::ShowWindow(int nCmdShow)
{
    m_pDlg->ShowWindow(nCmdShow);
}

BOOL CVisionInlineUI::IsWindowVisible()
{
    return m_pDlg->IsWindowVisible();
}

void CVisionInlineUI::SetColorImageForNGRV(Ipvm::Image8u3 image) //kircheis_NGRVINLINE
{
    if (IsWindowVisible())
        m_pDlg->SetColorImageForNGRV(image);
}