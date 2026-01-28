//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgChipNameEdit.h"

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
IMPLEMENT_DYNAMIC(CDlgChipNameEdit, CDialog)

CDlgChipNameEdit::CDlgChipNameEdit(CWnd* pParent /*=NULL*/)
    : CDialog(IDD_DIALOG_CHIP_NAME, pParent)
{
}

CDlgChipNameEdit::~CDlgChipNameEdit()
{
}

void CDlgChipNameEdit::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgChipNameEdit, CDialog)
ON_BN_CLICKED(IDOK, &CDlgChipNameEdit::OnBnClickedOk)
END_MESSAGE_MAP()

// CDlgChipNameEdit 메시지 처리기입니다.

void CDlgChipNameEdit::OnBnClickedOk()
{
    GetDlgItemText(IDC_EDIT_CHIP_NAME, m_strChipName);

    CDialog::OnOK();
}
