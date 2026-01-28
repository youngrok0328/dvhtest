//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceSetMaskNameDlg.h"

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
CSurfaceSetMaskNameDlg::CSurfaceSetMaskNameDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CSurfaceSetMaskNameDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSurfaceSetMaskNameDlg)
    m_strName = _T("");
    //}}AFX_DATA_INIT
}

void CSurfaceSetMaskNameDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSurfaceSetMaskNameDlg)
    DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSurfaceSetMaskNameDlg, CDialog)
//{{AFX_MSG_MAP(CSurfaceSetMaskNameDlg)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurfaceSetMaskNameDlg message handlers
