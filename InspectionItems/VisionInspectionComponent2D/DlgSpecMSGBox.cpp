//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSpecMSGBox.h"

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
IMPLEMENT_DYNAMIC(CDlgSpecMSGBox, CDialog)

CDlgSpecMSGBox::CDlgSpecMSGBox(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgSpecMSGBox::IDD, pParent)
    , m_strSpecName(_T(""))
{
}

CDlgSpecMSGBox::~CDlgSpecMSGBox()
{
}

void CDlgSpecMSGBox::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPECDB_ALGO, m_strSpecName);
}

BEGIN_MESSAGE_MAP(CDlgSpecMSGBox, CDialog)
ON_WM_SHOWWINDOW()

END_MESSAGE_MAP()

void CDlgSpecMSGBox::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    if (bShow)
    {
        SetDlgItemText(IDC_EDIT_SPECDB_ALGO, m_strSpecName);
    }
    else
    {
    }
}
