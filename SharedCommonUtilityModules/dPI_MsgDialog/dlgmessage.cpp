//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "dlgMessage.h"

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
CdlgMessage::CdlgMessage(CWnd* pParent /*=NULL*/, LPCTSTR lpStr)
    : CDialog(CdlgMessage::IDD, pParent)
{
    //{{AFX_DATA_INIT(CdlgMessage)
    //}}AFX_DATA_INIT
    m_strMsg = lpStr;
}

void CdlgMessage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CdlgMessage)
    DDX_Control(pDX, IDC_STATIC_Msg, m_lblMsg);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CdlgMessage, CDialog)
//{{AFX_MSG_MAP(CdlgMessage)
ON_BN_CLICKED(IDNO, OnNo)
ON_BN_CLICKED(IDYES, OnYes)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CdlgMessage message handlers
// MB_YESNOCANCEL, MB_OKCANCEL, MB_OK 3가지를 구분하여 사용한다.
// m_nMBType의 기본값은 MB_OK이다.

BOOL CdlgMessage::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtOrder1;
    GetDlgItem(IDC_STATIC_ORDER1)->GetWindowRect(rtOrder1);
    ScreenToClient(rtOrder1);
    CRect rtOrder2;
    GetDlgItem(IDC_STATIC_ORDER2)->GetWindowRect(rtOrder2);
    ScreenToClient(rtOrder2);
    CRect rtOrder3;
    GetDlgItem(IDC_STATIC_ORDER3)->GetWindowRect(rtOrder3);
    ScreenToClient(rtOrder3);
    CRect rtOrder4;
    GetDlgItem(IDC_STATIC_ORDER4)->GetWindowRect(rtOrder4);
    ScreenToClient(rtOrder4);
    CRect rtOrder5;
    GetDlgItem(IDC_STATIC_ORDER5)->GetWindowRect(rtOrder5);
    ScreenToClient(rtOrder5);

    if (m_nMBType == MB_YESNOCANCEL)
    {
        GetDlgItem(IDYES)->SetWindowPos(
            &CWnd::wndTopMost, rtOrder1.left, rtOrder1.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDYES)->ShowWindow(SW_SHOW);
        GetDlgItem(IDNO)->SetWindowPos(&CWnd::wndTopMost, rtOrder3.left, rtOrder3.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDNO)->ShowWindow(SW_SHOW);
        GetDlgItem(IDCANCEL)->SetWindowPos(
            &CWnd::wndTopMost, rtOrder5.left, rtOrder5.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
    }
    else if (m_nMBType == MB_OK)
    {
        GetDlgItem(IDOK)->SetWindowPos(&CWnd::wndTopMost, rtOrder3.left, rtOrder3.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
    }
    else if (m_nMBType == MB_YESNO)
    {
        GetDlgItem(IDYES)->SetWindowPos(
            &CWnd::wndTopMost, rtOrder2.left, rtOrder2.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDYES)->ShowWindow(SW_SHOW);
        GetDlgItem(IDNO)->SetWindowPos(&CWnd::wndTopMost, rtOrder4.left, rtOrder4.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDNO)->ShowWindow(SW_SHOW);
    }
    else if (m_nMBType == MB_OKCANCEL)
    {
        GetDlgItem(IDOK)->SetWindowPos(&CWnd::wndTopMost, rtOrder2.left, rtOrder2.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
        GetDlgItem(IDCANCEL)->SetWindowPos(
            &CWnd::wndTopMost, rtOrder4.left, rtOrder4.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
    }
    else if (m_nMBType == MB_RETRYCANCEL)
    {
        GetDlgItem(IDOK)->SetWindowPos(&CWnd::wndTopMost, rtOrder2.left, rtOrder2.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDOK)->SetWindowText(_T("Retry"));
        GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
        GetDlgItem(IDCANCEL)->SetWindowPos(
            &CWnd::wndTopMost, rtOrder4.left, rtOrder4.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
    }
    else
        ASSERT(FALSE);

    m_strMsg = CString("\n") + m_strMsg;

    m_lblMsg.SetBkColor(RGB(255, 200, 128));
    m_lblMsg.SetTextColor(RGB(0, 0, 0));
    m_lblMsg.SetText(m_strMsg);

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

int CdlgMessage::DoModal(int nType)
{
    m_nMBType = nType;

    switch (m_nMBType)
    {
        case MB_YESNO:
            m_nResult = IDNO;
            break;

        case MB_OK:
            m_nResult = IDOK;
            break;

        case MB_YESNOCANCEL:
        case MB_OKCANCEL:
        case MB_RETRYCANCEL:
            m_nResult = IDCANCEL;
            break;

        default:
            ASSERT(FALSE);
    }

    CDialog::DoModal();

    return m_nResult;
}
void CdlgMessage::OnNo()
{
    m_nResult = IDNO;
    CDialog::OnOK();
}

void CdlgMessage::OnOK()
{
    if (m_nMBType == MB_RETRYCANCEL)
        m_nResult = IDRETRY;
    else
        m_nResult = IDOK;
    CDialog::OnOK();
}

void CdlgMessage::OnCancel()
{
    m_nResult = IDCANCEL;
    CDialog::OnOK();
}

void CdlgMessage::OnYes()
{
    m_nResult = IDYES;
    CDialog::OnOK();
}
