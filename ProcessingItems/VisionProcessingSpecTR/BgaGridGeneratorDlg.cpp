//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BgaGridGeneratorDlg.h"

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
BgaGridGeneratorDlg::BgaGridGeneratorDlg(CWnd* pParent /*=NULL*/)
    : CDialog(BgaGridGeneratorDlg::IDD, pParent)
    , m_gridCountX(0)
    , m_gridCountY(0)
    , m_gridPitchX_um(0)
    , m_gridPitchY_um(0)
    , m_ballDiameter_um(0)
    , m_ballHeight_um(0)
    , m_bIgnore(false)
{
}

void BgaGridGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_GRID_COUNT_X, m_gridCountX);
    DDX_Text(pDX, IDC_EDIT_GRID_COUNT_Y, m_gridCountY);
    DDX_Text(pDX, IDC_EDIT_GRID_PITCH_X, m_gridPitchX_um);
    DDX_Text(pDX, IDC_EDIT_GRID_PITCH_Y, m_gridPitchY_um);
    DDX_Text(pDX, IDC_EDIT_BALL_DIAMETER, m_ballDiameter_um);
    DDX_Text(pDX, IDC_EDIT_BALL_HEIGHT, m_ballHeight_um);
}

BEGIN_MESSAGE_MAP(BgaGridGeneratorDlg, CDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BgaGridGeneratorDlg message handlers

BOOL BgaGridGeneratorDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void BgaGridGeneratorDlg::OnOK()
{
    UpdateData();

    __super::OnOK();
}
