//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "dlgPassword.h"

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
IMPLEMENT_DYNAMIC(CdlgPassword, CDialog)

CdlgPassword::CdlgPassword(CWnd* pParent /*=NULL*/)
    : CDialog(CdlgPassword::IDD, pParent)
{
}

CdlgPassword::~CdlgPassword()
{
}

void CdlgPassword::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CdlgPassword, CDialog)
ON_BN_CLICKED(IDOK, &CdlgPassword::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CdlgPassword::OnBnClickedCancel)
END_MESSAGE_MAP()

// CdlgPassword 메시지 처리기입니다.

void CdlgPassword::OnBnClickedOk()
{
    ((CEdit*)GetDlgItem(IDC_EDIT_PASSWORD))->GetWindowText(m_strPassword);

    OnOK();
}

void CdlgPassword::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnCancel();
}

BOOL CdlgPassword::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    ((CEdit*)GetDlgItem(IDC_EDIT_PASSWORD))->SetFocus();

    return FALSE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
