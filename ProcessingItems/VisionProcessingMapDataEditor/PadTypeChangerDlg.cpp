//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PadTypeChangerDlg.h"

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
IMPLEMENT_DYNAMIC(CPadTypeChangerDlg, CDialog)

CPadTypeChangerDlg::CPadTypeChangerDlg(CWnd* pParent /*=nullptr*/, long& nSelectedType)
    : CDialog(IDD_DLG_MAPDATA_PADTYPE_CHANGER, pParent)
    , m_nSelectedPadType(&nSelectedType)
{
}

CPadTypeChangerDlg::~CPadTypeChangerDlg()
{
}

void CPadTypeChangerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PADTYPE, m_cmbPadType);
}

BEGIN_MESSAGE_MAP(CPadTypeChangerDlg, CDialog)
ON_CBN_SELCHANGE(IDC_COMBO_PADTYPE, &CPadTypeChangerDlg::OnCbnSelchangeComboPadtype)
ON_BN_CLICKED(IDOK, &CPadTypeChangerDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CPadTypeChangerDlg 메시지 처리기
BOOL CPadTypeChangerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_cmbPadType.AddString(_T("Rectangle_32f"));
    m_cmbPadType.AddString(_T("Circle"));
    m_cmbPadType.AddString(_T("Triangle"));
    m_cmbPadType.AddString(_T("Pin Index"));

    m_cmbPadType.SetCurSel(0);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CPadTypeChangerDlg::OnCbnSelchangeComboPadtype()
{
    *m_nSelectedPadType = m_cmbPadType.GetCurSel();
}

void CPadTypeChangerDlg::OnBnClickedOk()
{
    long curSelectedVal = m_cmbPadType.GetCurSel();
    if (curSelectedVal == 0)
        *m_nSelectedPadType = 2;
    else if (curSelectedVal == 1)
        *m_nSelectedPadType = 1;
    else if (curSelectedVal == 2)
        *m_nSelectedPadType = 3;
    else if (curSelectedVal == 3)
        *m_nSelectedPadType = 0;

    CDialog::OnOK();
}
