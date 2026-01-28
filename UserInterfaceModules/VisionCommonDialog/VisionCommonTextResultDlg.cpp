//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionCommonTextResultDlg.h"

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
IMPLEMENT_DYNAMIC(VisionCommonTextResultDlg, CDialog)

VisionCommonTextResultDlg::VisionCommonTextResultDlg(CWnd* pParent, const CRect& rtPositionOnParent)
    : CDialog(IDD_DIALOG_TEXT_RESULT, pParent)
    , m_rtInitPosition(rtPositionOnParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    AfxInitRichEdit();

    VERIFY(Create(IDD_DIALOG_TEXT_RESULT, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

VisionCommonTextResultDlg::~VisionCommonTextResultDlg()
{
}

void VisionCommonTextResultDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT22, m_RE_Watch);
}

BEGIN_MESSAGE_MAP(VisionCommonTextResultDlg, CDialog)
ON_WM_SIZE()
END_MESSAGE_MAP()

// VisionCommonTextResultDlg 메시지 처리기입니다.

BOOL VisionCommonTextResultDlg::OnInitDialog()
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

void VisionCommonTextResultDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (m_RE_Watch.GetSafeHwnd())
    {
        m_RE_Watch.MoveWindow(0, 0, cx, cy);
    }
}

void VisionCommonTextResultDlg::SetLogBoxText(LPCTSTR text)
{
    m_RE_Watch.SetWindowText(_T(""));
    m_RE_Watch.ReplaceSel(text);
}
