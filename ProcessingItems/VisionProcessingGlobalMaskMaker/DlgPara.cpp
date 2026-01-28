//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgPara.h"

//CPP_2_________________________________ This project's headers
#include "DlgParaSingle.h"
#include "DlgSetupUI.h"
#include "VisionProcessingGlobalMaskMaker.h"
#include "VisionProcessingGlobalMaskMakerPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgPara, CDialogEx)

DlgPara::DlgPara(const CRect& dialogArea, VisionProcessingGlobalMaskMaker* pVisionInsp, CDlgSetupUI* parent)
    : CDialogEx(IDD_DIALOG_PARA, parent)
    , m_parent(parent)
    , m_dialogArea(dialogArea)
    , m_pVisionInsp(pVisionInsp)
    , m_pVisionPara(pVisionInsp->m_VisionPara)
    , m_displayOtherLayerMask(FALSE)
    , m_subSingle(nullptr)
{
}

DlgPara::~DlgPara()
{
    delete m_subSingle;
}

void DlgPara::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_LAYER_NAME, m_editLayerName);
}

BEGIN_MESSAGE_MAP(DlgPara, CDialogEx)
ON_WM_SIZE()
ON_WM_DESTROY()
END_MESSAGE_MAP()

// DlgPara message handlers

BOOL DlgPara::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_subSingle = new DlgParaSingle(m_pVisionInsp, *this, this);
    m_subSingle->Create(IDD_DIALOG_PARA_SINGLE, this);
    m_subSingle->ShowWindow(SW_SHOW);

    MoveWindow(m_dialogArea, FALSE);

    UpdateLayerMaskList();
    m_parent->SetDisplayImage();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgPara::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_subSingle == NULL)
        return;

    {
        CRect window;

        m_editLayerName.GetWindowRect(window);
        ScreenToClient(window);

        m_editLayerName.MoveWindow(0, window.top, cx, window.Height());
    }

    CRect rtStep;
    m_editLayerName.GetWindowRect(rtStep);
    ScreenToClient(rtStep);

    rtStep.top = rtStep.bottom;
    rtStep.bottom = cy;

    m_subSingle->MoveWindow(rtStep);
    GetDlgItem(IDC_SELECT_VIEW)->MoveWindow(rtStep);
}

void DlgPara::OnDestroy()
{
    CDialogEx::OnDestroy();

    m_subSingle->DestroyWindow();
}

void DlgPara::UpdateLayerMaskList()
{
    long selectLayer = m_parent->GetCurrentLayer();
    if (selectLayer < 0 || selectLayer >= m_pVisionPara->getLayerCount())
    {
        m_editLayerName.SetWindowText(_T(""));
        m_subSingle->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SELECT_VIEW)->ShowWindow(SW_SHOW);
    }
    else
    {
        m_editLayerName.SetWindowText(m_pVisionPara->getLayer(selectLayer).m_name);
        m_subSingle->UpdateLayerMaskList();
        m_subSingle->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_SELECT_VIEW)->ShowWindow(SW_HIDE);
    }

    m_parent->SetROI();
}

long DlgPara::GetSelectLayerIndex() const
{
    return m_parent->GetCurrentLayer();
}

SpecLayer* DlgPara::GetSelectSpecLayer()
{
    long selectLayer = m_parent->GetCurrentLayer();
    if (selectLayer < 0 || selectLayer >= m_pVisionPara->getLayerCount())
    {
        return nullptr;
    }

    return &m_pVisionPara->getLayer(selectLayer);
}
