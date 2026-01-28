//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BodySizeEditor.h"

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
CBodySizeEditor::CBodySizeEditor(CPackageSpec& packageSpec, CWnd* pParent /*=NULL*/)
    : CDialog(CBodySizeEditor::IDD, pParent)
    , m_packageSpec(packageSpec)
{
    //{{AFX_DATA_INIT(CBodySizeEditor)
    m_fBodySizeX = m_packageSpec.m_bodyInfoMaster->fBodySizeX * 0.001f;
    m_fBodySizeY = m_packageSpec.m_bodyInfoMaster->fBodySizeY * 0.001f;
    m_fBodySizeZ = m_packageSpec.m_bodyInfoMaster->fBodyThickness * 0.001f;
    m_fDistFromLeft = m_packageSpec.m_bodyInfoMaster->m_fDistFromLeft * 0.001f;
    m_fLowFromTop = m_packageSpec.m_bodyInfoMaster->m_fLowFromTop * 0.001f;
    m_fRoundRadius = m_packageSpec.m_bodyInfoMaster->m_fRoundRadius * 0.001f;
    m_fDistOctagonChamfer = m_packageSpec.m_bodyInfoMaster->m_fDistOctagonChamfer * 0.001f;

    m_bSDCardMode = m_packageSpec.m_bodyInfoMaster->m_bSDCardMode;
    m_bOctagonMode = m_packageSpec.m_bodyInfoMaster->m_bOctagonMode;

    m_nUnit = 0;

    m_bDeadBug = m_packageSpec.m_deadBug; //kircheis_20160622
    m_bChangedDeadBug = FALSE;

    m_nRoundPackageOption = m_packageSpec.m_bodyInfoMaster->m_nRoundPackageOption; //kircheis_RoundPKG
    m_fRoundPackageRadius = m_packageSpec.m_bodyInfoMaster->m_fRoundPackageRadius * 0.001f; //kircheis_RoundPKG

    //{{//kircheis_SideInsp
    m_bCheckGlassCorePackage = m_packageSpec.m_bodyInfoMaster->m_bGlassCorePackage;
    m_fGlassCoreThickness = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreThickness * 0.001f;
    m_fGlassCoreSubstrateSizeX = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeX * 0.001f;
    m_fGlassCoreSubstrateSizeY = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeY * 0.001f;
    m_fGlassCoreTopSubstrateThickness = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreTopSubstrateThickness * 0.001f;
    m_fGlassCoreBottomSubstrateThickness
        = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness * 0.001f;
    if (m_bCheckGlassCorePackage)
        m_fBodySizeZ = m_fGlassCoreThickness + m_fGlassCoreTopSubstrateThickness + m_fGlassCoreBottomSubstrateThickness;
    //}}AFX_DATA_INIT
}

void CBodySizeEditor::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CBodySizeEditor)
    DDX_Text(pDX, IDC_EDIT_BODY_SIZE_X, m_fBodySizeX);
    DDX_Text(pDX, IDC_EDIT_BODY_SIZE_Y, m_fBodySizeY);
    DDX_Text(pDX, IDC_EDIT_BODY_SIZE_Z, m_fBodySizeZ);
    DDX_Text(pDX, IDC_EDIT_DIST_LEFT, m_fDistFromLeft);
    DDX_Text(pDX, IDC_EDIT_LOW_TOP, m_fLowFromTop);
    DDX_Text(pDX, IDC_EDIT_RADIUS, m_fRoundRadius);
    DDX_Text(pDX, IDC_EDIT_DIST_OCTAGON_CHAMFER, m_fDistOctagonChamfer);
    DDX_Radio(pDX, IDC_RADIO_UNIT_MM, m_nUnit);
    //}}AFX_DATA_MAP
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_COMBO_ROUND_PACKAGE_OPTION, m_ctrlCmbRoundOption);
    DDX_Text(pDX, IDC_EDIT_ROUND_PACKAGE_RADIUS, m_fRoundPackageRadius);
    DDX_Check(pDX, IDC_CHECK_GLASS_CORE_PACKAGE, m_bCheckGlassCorePackage);
    DDX_Text(pDX, IDC_EDIT_GLASS_THICKNESS, m_fGlassCoreThickness);
    DDX_Text(pDX, IDC_EDIT_GLASS_SUBSTRATE_SIZE_X, m_fGlassCoreSubstrateSizeX);
    DDX_Text(pDX, IDC_EDIT_GLASS_SUBSTRATE_SIZE_Y, m_fGlassCoreSubstrateSizeY);
    DDX_Text(pDX, IDC_EDIT_GLASS_TOP_SUBSTRATE_THICKNESS, m_fGlassCoreTopSubstrateThickness);
    DDX_Text(pDX, IDC_EDIT_GLASS_BTM_SUBSTRATE_THICKNESS, m_fGlassCoreBottomSubstrateThickness);
}

BEGIN_MESSAGE_MAP(CBodySizeEditor, CDialog)
//{{AFX_MSG_MAP(CBodySizeEditor)
ON_BN_CLICKED(IDC_RADIO_UNIT_MIL, OnRadioUnitMil)
ON_BN_CLICKED(IDC_RADIO_UNIT_MM, OnRadioUnitMm)
//}}AFX_MSG_MAP
ON_BN_CLICKED(IDC_CHECK_SD_CARD, &CBodySizeEditor::OnBnClickedCheckSdCard)
ON_BN_CLICKED(IDC_CHECK_OCTAGON, &CBodySizeEditor::OnBnClickedCheckOctagon)
ON_BN_CLICKED(IDC_CHECK_DEAD_BUG, &CBodySizeEditor::OnBnClickedCheckDeadBug)
ON_CBN_SELCHANGE(IDC_COMBO_ROUND_PACKAGE_OPTION, &CBodySizeEditor::OnCbnSelchangeComboRoundOption)
ON_BN_CLICKED(IDC_CHECK_GLASS_CORE_PACKAGE, &CBodySizeEditor::OnBnClickedCheckGlassCorePackage)
ON_EN_CHANGE(IDC_EDIT_GLASS_THICKNESS, &CBodySizeEditor::OnEnChangeEditGlassThickness)
ON_EN_CHANGE(IDC_EDIT_GLASS_TOP_SUBSTRATE_THICKNESS, &CBodySizeEditor::OnEnChangeEditGlassTopSubstrateThickness)
ON_EN_CHANGE(IDC_EDIT_GLASS_BTM_SUBSTRATE_THICKNESS, &CBodySizeEditor::OnEnChangeEditGlassBottomSubstrateThickness)
ON_EN_CHANGE(IDC_EDIT_GLASS_SUBSTRATE_SIZE_X, &CBodySizeEditor::OnEnChangeEditGlassSubstrateSizeX)
ON_EN_CHANGE(IDC_EDIT_GLASS_SUBSTRATE_SIZE_Y, &CBodySizeEditor::OnEnChangeEditGlassSubstrateSizeY)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBodySizeEditor message handlers

BOOL CBodySizeEditor::OnInitDialog()
{
    CDialog::OnInitDialog();

    GetDlgItem(IDC_RADIO_UNIT_MIL)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_UNIT_MM)->EnableWindow(FALSE);

    ((CButton*)GetDlgItem(IDC_CHECK_SD_CARD))->SetCheck(m_bSDCardMode);
    ((CEdit*)GetDlgItem(IDC_EDIT_DIST_LEFT))->EnableWindow(m_bSDCardMode);
    ((CEdit*)GetDlgItem(IDC_EDIT_LOW_TOP))->EnableWindow(m_bSDCardMode);
    ((CEdit*)GetDlgItem(IDC_EDIT_RADIUS))->EnableWindow(m_bSDCardMode);

    ((CButton*)GetDlgItem(IDC_CHECK_OCTAGON))->SetCheck(m_bOctagonMode);
    ((CEdit*)GetDlgItem(IDC_EDIT_DIST_OCTAGON_CHAMFER))->EnableWindow(m_bOctagonMode);

    ((CButton*)GetDlgItem(IDC_CHECK_DEAD_BUG))->SetCheck(m_bDeadBug); //kircheis_20160622

    //{{ //kircheis_RoundPKG
    m_ctrlCmbRoundOption.SetCurSel(m_nRoundPackageOption);
    BOOL bEnable = (m_nRoundPackageOption != Round_Normal);
    m_fRoundPackageRadius = bEnable ? m_fRoundPackageRadius : 0.f;
    ((CEdit*)GetDlgItem(IDC_EDIT_ROUND_PACKAGE_RADIUS))->EnableWindow(bEnable);
    //}}

    //{{//kircheis_SideInsp
    SetDlgGlassItemEnable(m_packageSpec.m_bodyInfoMaster->m_bGlassCorePackage);
    //}}
    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CBodySizeEditor::OnOK()
{
    UpdateData();

    // TODO: Add extra validation here
    if (m_fBodySizeX <= 0.f || m_fBodySizeY <= 0.f || m_fBodySizeZ <= 0.f || m_fBodySizeX > 120.f
        || m_fBodySizeY > 240.f) // SDY Package Size의 최댓값 옵션 추가
    {
        ::SimpleMessage(_T("Invalid Body Size!"), MB_OK);
        return;
    }

    //{{//kircheis_SideInsp
    if (m_bCheckGlassCorePackage)
    {
        if (IsValidGlassCorePackageInfo() == FALSE)
            return;
    }
    //}}

    CDialog::OnOK();

    if (m_nUnit == 0) // mm
    {
        m_packageSpec.m_bodyInfoMaster->fBodySizeX = m_fBodySizeX / 0.001f;
        m_packageSpec.m_bodyInfoMaster->fBodySizeY = m_fBodySizeY / 0.001f;
        m_packageSpec.m_bodyInfoMaster->fBodyThickness = m_fBodySizeZ / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fDistFromLeft = m_fDistFromLeft / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fLowFromTop = m_fLowFromTop / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fRoundRadius = m_fRoundRadius / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fDistOctagonChamfer = m_fDistOctagonChamfer / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fRoundPackageRadius = m_fRoundPackageRadius / 0.001f; //kircheis_RoundPKG

        //{{//
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreThickness = m_fGlassCoreThickness / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeX = m_fGlassCoreSubstrateSizeX / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeY = m_fGlassCoreSubstrateSizeY / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreTopSubstrateThickness = m_fGlassCoreTopSubstrateThickness / 0.001f;
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness
            = m_fGlassCoreBottomSubstrateThickness / 0.001f;
        //}}
    }
    else // mil
    {
        m_packageSpec.m_bodyInfoMaster->fBodySizeX = m_fBodySizeX * 25.4f;
        m_packageSpec.m_bodyInfoMaster->fBodySizeY = m_fBodySizeY * 25.4f;
        m_packageSpec.m_bodyInfoMaster->fBodyThickness = m_fBodySizeZ * 25.4f;
        m_packageSpec.m_bodyInfoMaster->m_fDistFromLeft = m_fDistFromLeft * 25.4f;
        m_packageSpec.m_bodyInfoMaster->m_fLowFromTop = m_fLowFromTop * 25.4f;
        m_packageSpec.m_bodyInfoMaster->m_fRoundRadius = m_fRoundRadius * 25.4f;
        m_packageSpec.m_bodyInfoMaster->m_fRoundPackageRadius = m_fRoundPackageRadius * 25.4f; //kircheis_RoundPKG

        //{{//
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreThickness = m_fGlassCoreThickness * 25.4f;
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeX = m_fGlassCoreSubstrateSizeX * 25.4f;
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeY = m_fGlassCoreSubstrateSizeY * 25.4f;
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreTopSubstrateThickness = m_fGlassCoreTopSubstrateThickness * 25.4f;
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness
            = m_fGlassCoreBottomSubstrateThickness * 25.4f;
        //}}
    }

    m_packageSpec.m_bodyInfoMaster->m_bGlassCorePackage = m_bCheckGlassCorePackage;

    m_packageSpec.m_bodyInfoMaster->m_bSDCardMode = m_bSDCardMode;
    m_packageSpec.m_bodyInfoMaster->m_bOctagonMode = m_bOctagonMode;

    m_bChangedDeadBug = (m_packageSpec.m_deadBug != m_bDeadBug);
    m_packageSpec.m_deadBug = m_bDeadBug; //kircheis_20160622

    m_packageSpec.m_bodyInfoMaster->m_nRoundPackageOption = m_nRoundPackageOption; //kircheis_RoundPKG
}

void CBodySizeEditor::OnRadioUnitMil()
{
    UpdateData();

    m_nUnit = 1;

    GetDlgItem(IDC_RADIO_UNIT_MIL)->EnableWindow(FALSE);
    GetDlgItem(IDC_RADIO_UNIT_MM)->EnableWindow(TRUE);

    m_fBodySizeX /= 0.0254f;
    m_fBodySizeY /= 0.0254f;
    m_fBodySizeZ /= 0.0254f;
    m_fDistFromLeft /= 0.0254f;
    m_fLowFromTop /= 0.0254f;
    m_fRoundRadius /= 0.0254f;
    m_fRoundPackageRadius /= 0.0254f; //kircheis_RoundPKG

    m_fGlassCoreThickness /= 0.0254f;
    m_fGlassCoreSubstrateSizeX /= 0.0254f;
    m_fGlassCoreSubstrateSizeY /= 0.0254f;
    m_fGlassCoreTopSubstrateThickness /= 0.0254f;
    m_fGlassCoreBottomSubstrateThickness /= 0.0254f;

    UpdateData(FALSE);
}

void CBodySizeEditor::OnRadioUnitMm()
{
    UpdateData();

    m_nUnit = 0;

    GetDlgItem(IDC_RADIO_UNIT_MIL)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_UNIT_MM)->EnableWindow(FALSE);

    m_fBodySizeX *= 0.0254f;
    m_fBodySizeY *= 0.0254f;
    m_fBodySizeZ *= 0.0254f;
    m_fDistFromLeft *= 0.0254f;
    m_fLowFromTop *= 0.0254f;
    m_fRoundRadius *= 0.0254f;
    m_fDistOctagonChamfer *= 0.0254f;
    m_fRoundPackageRadius *= 0.0254f; //kircheis_RoundPKG

    m_fGlassCoreThickness *= 0.0254f;
    m_fGlassCoreSubstrateSizeX *= 0.0254f;
    m_fGlassCoreSubstrateSizeY *= 0.0254f;
    m_fGlassCoreTopSubstrateThickness *= 0.0254f;
    m_fGlassCoreBottomSubstrateThickness *= 0.0254f;

    UpdateData(FALSE);
}

void CBodySizeEditor::OnBnClickedCheckSdCard()
{
    m_bSDCardMode = ((CButton*)GetDlgItem(IDC_CHECK_SD_CARD))->GetCheck();

    ((CEdit*)GetDlgItem(IDC_EDIT_DIST_LEFT))->EnableWindow(m_bSDCardMode);
    ((CEdit*)GetDlgItem(IDC_EDIT_LOW_TOP))->EnableWindow(m_bSDCardMode);
    ((CEdit*)GetDlgItem(IDC_EDIT_RADIUS))->EnableWindow(m_bSDCardMode);
}

void CBodySizeEditor::OnBnClickedCheckOctagon()
{
    m_bOctagonMode = ((CButton*)GetDlgItem(IDC_CHECK_OCTAGON))->GetCheck();

    ((CEdit*)GetDlgItem(IDC_EDIT_DIST_OCTAGON_CHAMFER))->EnableWindow(m_bOctagonMode);
}

void CBodySizeEditor::OnBnClickedCheckDeadBug()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    m_bDeadBug = ((CButton*)GetDlgItem(IDC_CHECK_DEAD_BUG))->GetCheck();
}

void CBodySizeEditor::OnCbnSelchangeComboRoundOption() //kircheis_RoundPKG
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);
    m_nRoundPackageOption = m_ctrlCmbRoundOption.GetCurSel();

    BOOL bEnable = (m_nRoundPackageOption != Round_Normal);
    m_fRoundPackageRadius = bEnable ? m_fRoundPackageRadius : 0.f;
    ((CEdit*)GetDlgItem(IDC_EDIT_ROUND_PACKAGE_RADIUS))->EnableWindow(bEnable);

    UpdateData(FALSE);
}

BOOL CBodySizeEditor::IsValidGlassCorePackageInfo()
{
    if (m_bCheckGlassCorePackage == FALSE)
        return TRUE;

    BOOL bValidInfo = TRUE;
    if (m_fGlassCoreThickness <= 0.f)
    {
        ::SimpleMessage(_T("Invalid Glass Core Thickness!"), MB_OK);
        bValidInfo = FALSE;
    }
    if (m_fGlassCoreTopSubstrateThickness
        < 0.f) //Substrate 치수 정보가 0이면 Substrate 코팅 없는 순수 Glass만 있는 넘이다. 0은 허용해도 음수는 허용 못한다
    {
        ::SimpleMessage(_T("Invalid Glass Core Top Substrate Thickness!"), MB_OK);
        bValidInfo = FALSE;
    }
    if (m_fGlassCoreBottomSubstrateThickness
        < 0.f) //Substrate 치수 정보가 0이면 Substrate 코팅 없는 순수 Glass만 있는 넘이다. 0은 허용해도 음수는 허용 못한다
    {
        ::SimpleMessage(_T("Invalid Glass Core Bottom Substrate Thickness!"), MB_OK);
        bValidInfo = FALSE;
    }
    if (m_fGlassCoreSubstrateSizeX
        < 0.f) //Substrate 치수 정보가 0이면 Substrate 코팅 없는 순수 Glass만 있는 넘이다. 0은 허용해도 음수는 허용 못한다
    {
        ::SimpleMessage(_T("Invalid Glass Core Substrate Size X!"), MB_OK);
        bValidInfo = FALSE;
    }
    if (m_fGlassCoreSubstrateSizeX > m_fBodySizeX)
    {
        ::SimpleMessage(_T("Glass Core Substrate Size X is larger than Body size X!"), MB_OK);
        bValidInfo = FALSE;
    }
    if (m_fGlassCoreSubstrateSizeY
        < 0.f) //Substrate 치수 정보가 0이면 Substrate 코팅 없는 순수 Glass만 있는 넘이다. 0은 허용해도 음수는 허용 못한다
    {
        ::SimpleMessage(_T("Invalid Glass Core Substrate Size Y!"), MB_OK);
        bValidInfo = FALSE;
    }
    if (m_fGlassCoreSubstrateSizeY > m_fBodySizeY)
    {
        ::SimpleMessage(_T("Glass Core Substrate Size Y is larger than Body size Y!"), MB_OK);
        bValidInfo = FALSE;
    }

    return bValidInfo;
}

void CBodySizeEditor::OnBnClickedCheckGlassCorePackage()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);
    SetDlgGlassItemEnable(m_bCheckGlassCorePackage);
}

void CBodySizeEditor::SetDlgGlassItemEnable(BOOL bGlassCorePackage)
{
    GetDlgItem(IDC_EDIT_BODY_SIZE_Z)->EnableWindow(!bGlassCorePackage);

    GetDlgItem(IDC_STATIC_GLASS_CORE_THICKNESS)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_EDIT_GLASS_THICKNESS)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_STATIC_GLASS_CORE_SUBSTRATE_SIZE_X)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_EDIT_GLASS_SUBSTRATE_SIZE_X)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_STATIC_GLASS_CORE_SUBSTRATE_SIZE_Y)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_EDIT_GLASS_SUBSTRATE_SIZE_Y)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_STATIC_GLASS_CORE_TOP_SUBSTRATE_THICKNESS)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_EDIT_GLASS_TOP_SUBSTRATE_THICKNESS)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_STATIC_GLASS_CORE_BTM_SUBSTRATE_THICKNESS)->EnableWindow(bGlassCorePackage);
    GetDlgItem(IDC_EDIT_GLASS_BTM_SUBSTRATE_THICKNESS)->EnableWindow(bGlassCorePackage);
}

void CBodySizeEditor::OnEnChangeEditGlassThickness()
{
    // TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
    // CDialog::OnInitDialog() 함수를 재지정
    //하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
    // 이 알림 메시지를 보내지 않습니다.

    // TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);
    if (m_bCheckGlassCorePackage == FALSE)
        return;

    m_fBodySizeZ = m_fGlassCoreThickness + m_fGlassCoreTopSubstrateThickness + m_fGlassCoreBottomSubstrateThickness;
    CString strBodySizeZ;
    strBodySizeZ.Format(_T("%f"), m_fBodySizeZ);
    SetDlgItemText(IDC_EDIT_BODY_SIZE_Z, strBodySizeZ);
}

void CBodySizeEditor::OnEnChangeEditGlassTopSubstrateThickness()
{
    // TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
    // CDialog::OnInitDialog() 함수를 재지정
    //하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
    // 이 알림 메시지를 보내지 않습니다.

    // TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);
    if (m_bCheckGlassCorePackage == FALSE)
        return;

    m_fBodySizeZ = m_fGlassCoreThickness + m_fGlassCoreTopSubstrateThickness + m_fGlassCoreBottomSubstrateThickness;

    CString strBodySizeZ;
    strBodySizeZ.Format(_T("%f"), m_fBodySizeZ);
    SetDlgItemText(IDC_EDIT_BODY_SIZE_Z, strBodySizeZ);
}

void CBodySizeEditor::OnEnChangeEditGlassBottomSubstrateThickness()
{
    // TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
    // CDialog::OnInitDialog() 함수를 재지정
    //하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
    // 이 알림 메시지를 보내지 않습니다.

    // TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);
    if (m_bCheckGlassCorePackage == FALSE)
        return;

    m_fBodySizeZ = m_fGlassCoreThickness + m_fGlassCoreTopSubstrateThickness + m_fGlassCoreBottomSubstrateThickness;

    CString strBodySizeZ;
    strBodySizeZ.Format(_T("%f"), m_fBodySizeZ);
    SetDlgItemText(IDC_EDIT_BODY_SIZE_Z, strBodySizeZ);
}

void CBodySizeEditor::OnEnChangeEditGlassSubstrateSizeX()
{
    // TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
    // CDialog::OnInitDialog() 함수를 재지정
    //하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
    // 이 알림 메시지를 보내지 않습니다.

    // TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CBodySizeEditor::OnEnChangeEditGlassSubstrateSizeY()
{
    // TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
    // CDialog::OnInitDialog() 함수를 재지정
    //하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
    // 이 알림 메시지를 보내지 않습니다.

    // TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
