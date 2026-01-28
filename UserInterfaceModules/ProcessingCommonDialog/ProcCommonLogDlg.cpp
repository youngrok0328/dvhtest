//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ProcCommonLogDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(ProcCommonLogDlg, CDialog)

ProcCommonLogDlg::ProcCommonLogDlg(CWnd* pParent, const CRect& rtPositionOnParent)
    : CDialog(IDD_DIALOG_TEXT_LOG, pParent)
    , m_rtInitPosition(rtPositionOnParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    AfxInitRichEdit();

    VERIFY(Create(IDD_DIALOG_TEXT_LOG, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

ProcCommonLogDlg::~ProcCommonLogDlg()
{
}

void ProcCommonLogDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT22, m_RE_Watch);
}

BEGIN_MESSAGE_MAP(ProcCommonLogDlg, CDialog)
ON_WM_SIZE()
END_MESSAGE_MAP()

// ProcCommonLogDlg 메시지 처리기입니다.

BOOL ProcCommonLogDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);

    CHARFORMAT cf;
    m_RE_Watch.GetDefaultCharFormat(cf);
    cf.dwMask = CFM_FACE;
    _tcscpy_s(cf.szFaceName, 20, _T("FixedSys"));
    m_RE_Watch.SetDefaultCharFormat(cf);
    return TRUE; // return TRUE unless you set the focus to a control
}

void ProcCommonLogDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (m_RE_Watch.GetSafeHwnd())
    {
        m_RE_Watch.MoveWindow(0, 0, cx, cy);
    }
}

void ProcCommonLogDlg::SetLogBoxText(LPCTSTR text)
{
    m_RE_Watch.SetWindowText(_T(""));
    m_RE_Watch.ReplaceSel(text);
}
