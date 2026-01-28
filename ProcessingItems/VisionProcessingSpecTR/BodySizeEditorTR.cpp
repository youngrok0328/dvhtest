//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BodySizeEditorTR.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BodySizeEditorTR::BodySizeEditorTR(CPackageSpec& packageSpec, CWnd* pParent /*=NULL*/)
    : CDialog(BodySizeEditorTR::IDD, pParent)
    , m_packageSpec(packageSpec)
{
    //{{AFX_DATA_INIT(BodySizeEditorTR)
    m_fBodySizeX = m_packageSpec.m_bodyInfoMaster->fBodySizeX * 0.001f;
    m_fBodySizeY = m_packageSpec.m_bodyInfoMaster->fBodySizeY * 0.001f;
    m_fBodySizeZ = m_packageSpec.m_bodyInfoMaster->fBodyThickness * 0.001f;

    m_nUnit = 0;

    m_bDeadBug = m_packageSpec.m_deadBug; //kircheis_20160622
    m_bChangedDeadBug = FALSE;
    //}}AFX_DATA_INIT
}

void BodySizeEditorTR::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(BodySizeEditorTR)
    DDX_Text(pDX, IDC_EDIT_BODY_SIZE_X, m_fBodySizeX);
    DDX_Text(pDX, IDC_EDIT_BODY_SIZE_Y, m_fBodySizeY);
    DDX_Text(pDX, IDC_EDIT_BODY_SIZE_Z, m_fBodySizeZ);
    DDX_Radio(pDX, IDC_RADIO_UNIT_MM, m_nUnit);
    //}}AFX_DATA_MAP
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(BodySizeEditorTR, CDialog)
//{{AFX_MSG_MAP(BodySizeEditorTR)
ON_BN_CLICKED(IDC_RADIO_UNIT_MIL, OnRadioUnitMil)
ON_BN_CLICKED(IDC_RADIO_UNIT_MM, OnRadioUnitMm)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BodySizeEditorTR message handlers

BOOL BodySizeEditorTR::OnInitDialog()
{
    CDialog::OnInitDialog();

    GetDlgItem(IDC_RADIO_UNIT_MIL)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_UNIT_MM)->EnableWindow(FALSE);

    ((CButton*)GetDlgItem(IDC_CHECK_DEAD_BUG))->SetCheck(m_bDeadBug); //kircheis_20160622

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void BodySizeEditorTR::OnOK()
{
    UpdateData();

    // TODO: Add extra validation here
    if (m_fBodySizeX <= 0.f || m_fBodySizeY <= 0.f || m_fBodySizeZ <= 0.f || m_fBodySizeX > 120.f
        || m_fBodySizeY > 240.f) // SDY Package Size의 최댓값 옵션 추가
    {
        ::SimpleMessage(_T("Invalid Body Size!"), MB_OK);
        return;
    }
    CDialog::OnOK();

    if (m_nUnit == 0) // mm
    {
        m_packageSpec.m_bodyInfoMaster->fBodySizeX = m_fBodySizeX / 0.001f;
        m_packageSpec.m_bodyInfoMaster->fBodySizeY = m_fBodySizeY / 0.001f;
        m_packageSpec.m_bodyInfoMaster->fBodyThickness = m_fBodySizeZ / 0.001f;
    }
    else // mil
    {
        m_packageSpec.m_bodyInfoMaster->fBodySizeX = m_fBodySizeX * 25.4f;
        m_packageSpec.m_bodyInfoMaster->fBodySizeY = m_fBodySizeY * 25.4f;
        m_packageSpec.m_bodyInfoMaster->fBodyThickness = m_fBodySizeZ * 25.4f;
    }

    m_bChangedDeadBug = (m_packageSpec.m_deadBug != m_bDeadBug);
    m_packageSpec.m_deadBug = m_bDeadBug; //kircheis_20160622
}

void BodySizeEditorTR::OnRadioUnitMil()
{
    UpdateData();

    m_nUnit = 1;

    GetDlgItem(IDC_RADIO_UNIT_MIL)->EnableWindow(FALSE);
    GetDlgItem(IDC_RADIO_UNIT_MM)->EnableWindow(TRUE);

    m_fBodySizeX /= 0.0254f;
    m_fBodySizeY /= 0.0254f;
    m_fBodySizeZ /= 0.0254f;

    UpdateData(FALSE);
}

void BodySizeEditorTR::OnRadioUnitMm()
{
    UpdateData();

    m_nUnit = 0;

    GetDlgItem(IDC_RADIO_UNIT_MIL)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_UNIT_MM)->EnableWindow(FALSE);

    m_fBodySizeX *= 0.0254f;
    m_fBodySizeY *= 0.0254f;
    m_fBodySizeZ *= 0.0254f;

    UpdateData(FALSE);
}
