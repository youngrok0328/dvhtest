//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgLogin.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgLogin, CDialog)

CDlgLogin::CDlgLogin(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgLogin::IDD, pParent)
{
    m_nMode = 0;
    m_bHiddenView = FALSE;
    m_bSystemSaveMode = FALSE;
}

CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgLogin, CDialog)
ON_BN_CLICKED(IDOK, &CDlgLogin::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgLogin::OnBnClickedCancel)
ON_BN_CLICKED(IDC_Operator, &CDlgLogin::OnBnClickedOperator)
ON_BN_CLICKED(IDC_Engineer, &CDlgLogin::OnBnClickedEngineer)
ON_BN_CLICKED(IDC_IntekPlus, &CDlgLogin::OnBnClickedIntekplus)
END_MESSAGE_MAP()

// CDlgLogin 메시지 처리기입니다.

void CDlgLogin::SetAccessMode(long nAccessLevel)
{
    m_nMode = nAccessLevel;
}

void CDlgLogin::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_SHIFT))
    {
        m_bSystemSaveMode = TRUE;
    }

    OnOK();
}

void CDlgLogin::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnCancel();
}

void CDlgLogin::OnBnClickedOperator()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    m_nMode = 0;
}

void CDlgLogin::OnBnClickedEngineer()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    m_nMode = 1;
}

void CDlgLogin::OnBnClickedIntekplus()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    m_nMode = 2;
}

BOOL CDlgLogin::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_nMode >= _INTEKPLUS)
        m_nMode = _INTEKPLUS;

    switch (m_nMode)
    {
        case _OPERATOR:
            if (GetDlgItem(IDC_Operator) != NULL || GetDlgItem(IDC_Operator)->GetSafeHwnd() != NULL)
            {
                ((CButton*)GetDlgItem(IDC_Operator))->SetCheck(TRUE);
            }
            break;
        case _ENGINEER:
            if (GetDlgItem(IDC_Engineer) != NULL || GetDlgItem(IDC_Engineer)->GetSafeHwnd() != NULL)
            {
                ((CButton*)GetDlgItem(IDC_Engineer))->SetCheck(TRUE);
            }
            break;
        case _INTEKPLUS:
            if (GetDlgItem(IDC_IntekPlus) != NULL || GetDlgItem(IDC_IntekPlus)->GetSafeHwnd() != NULL)
            {
                ((CButton*)GetDlgItem(IDC_IntekPlus))->SetCheck(TRUE);
            }
            break;
    }

    if (GetDlgItem(IDC_IntekPlus) != NULL || GetDlgItem(IDC_IntekPlus)->GetSafeHwnd() != NULL)
    {
        ((CButton*)GetDlgItem(IDC_IntekPlus))->ShowWindow(SW_SHOW);
    }
    if (GetDlgItem(IDC_HiddenMode) != NULL || GetDlgItem(IDC_HiddenMode)->GetSafeHwnd() != NULL)
    {
        ((CButton*)GetDlgItem(IDC_HiddenMode))->ShowWindow(SW_HIDE);
    }

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CDlgLogin::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->wParam == VK_SHIFT && GetAsyncKeyState(VK_CONTROL))
    {
        m_bHiddenView = TRUE;
        ShowControl();
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CDlgLogin::ShowControl()
{
    if (m_bHiddenView)
    {
        if (GetDlgItem(IDC_IntekPlus) != NULL || GetDlgItem(IDC_IntekPlus)->GetSafeHwnd() != NULL)
        {
            ((CButton*)GetDlgItem(IDC_IntekPlus))->ShowWindow(SW_HIDE);
        }

        if (GetDlgItem(IDC_HiddenMode) != NULL || GetDlgItem(IDC_HiddenMode)->GetSafeHwnd() != NULL)
        {
            ((CButton*)GetDlgItem(IDC_HiddenMode))->ShowWindow(SW_SHOW);
        }
    }
}
