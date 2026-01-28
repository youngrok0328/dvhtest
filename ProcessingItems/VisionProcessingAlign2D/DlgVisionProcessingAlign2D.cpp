//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingAlign2D.h"

//CPP_2_________________________________ This project's headers
#include "Property_Stitch.h"
#include "VisionProcessingAlign2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonGridGroupDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/PropertyGrid.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1012)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_RAW (WM_USER + 1013)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE (WM_USER + 1014)

//CPP_7_________________________________ Implementation body
//
enum enumSetupInfoTab
{
    TAB_DEBUG_INFO = 0,
    TAB_TXT_LOG,
};

IMPLEMENT_DYNAMIC(DlgVisionProcessingAlign2D, CDialog)

DlgVisionProcessingAlign2D::DlgVisionProcessingAlign2D(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingAlign2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(DlgVisionProcessingAlign2D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_procCommonGridGroup(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_imageLotViewRearSide(nullptr)
    , m_procCommonGridGroupRear(nullptr)
    , m_procCommonLogDlgRear(nullptr)
    , m_procCommonDebugInfoDlgRear(nullptr)
{
    m_pVisionInsp = pVisionInsp;
}

DlgVisionProcessingAlign2D::~DlgVisionProcessingAlign2D()
{
    // Front
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonGridGroup;
    delete m_imageLotView;

    // Rear
    if (m_procCommonDebugInfoDlgRear != nullptr)
        delete m_procCommonDebugInfoDlgRear;
    if (m_procCommonLogDlgRear != nullptr)
        delete m_procCommonLogDlgRear;
    if (m_procCommonGridGroupRear != nullptr)
        delete m_procCommonGridGroupRear;
    if (m_imageLotViewRearSide != nullptr)
        delete m_imageLotViewRearSide;

    m_pVisionInsp->m_pVisionInspDlg = nullptr;
}

void DlgVisionProcessingAlign2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_STATIC_SIDE_FRONT_ALIGN2D, m_Label_Side_Front_Status);
    DDX_Control(pDX, IDC_STATIC_SIDE_REAR_ALIGN2D, m_Label_Side_Rear_Status);
}

BEGIN_MESSAGE_MAP(DlgVisionProcessingAlign2D, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &DlgVisionProcessingAlign2D::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &DlgVisionProcessingAlign2D::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &DlgVisionProcessingAlign2D::OnImageLotViewRoiChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW, &DlgVisionProcessingAlign2D::OnImageLotViewSelChangedRaw)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE, DlgVisionProcessingAlign2D::OnImageLotViewSelChangedImage)
ON_STN_CLICKED(IDC_STATIC_SIDE_FRONT_ALIGN2D, &DlgVisionProcessingAlign2D::OnStnClickedStaticSideFrontAlign2d)
ON_STN_CLICKED(IDC_STATIC_SIDE_REAR_ALIGN2D, &DlgVisionProcessingAlign2D::OnStnClickedStaticSideRearAlign2d)
END_MESSAGE_MAP()

// DlgVisionProcessingAlign2D 메시지 처리기입니다.

BOOL DlgVisionProcessingAlign2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision)
    {
        SetAlign2D_SideVision();
    }
    else
    {
        SetAlign2D_NormalVision();
    }

    return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgVisionProcessingAlign2D::SetAlign2D_NormalVision()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (bIsSideVision)
        return;

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, true, true, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    m_imageLotView->SetMessage_SelChangedRaw(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW);
    m_imageLotView->SetMessage_SelChangedImage(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE);

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    // Dialog Control 위치 설정
    CRect rtClient;
    GetClientRect(rtClient);

    // 강제로 Log 위치를 줄어고 파라메터 창을 늘렸다.
    // 나중에 이런것들을 고려해서 UI을 배치할 수 있는 법을 생각해 봤으면 좋겠다.

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    rtTab.right = m_procDlgInfo.m_rtParaArea.left;
    m_TabResult.MoveWindow(rtTab);

    CRect rtPara = m_procDlgInfo.m_rtParaArea;
    rtPara.bottom = rtTab.bottom;

    m_TabResult.InsertItem(TAB_DEBUG_INFO, _T("Debug"));
    m_TabResult.InsertItem(TAB_TXT_LOG, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);
    m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);

    ShowAlign2DNormalUI();
}

void DlgVisionProcessingAlign2D::SetAlign2D_SideVision()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (bIsSideVision == false)
        return;

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtLotViewRegionF, rtLotViewRegionR;
    rtLotViewRegionF.left = m_procDlgInfo.m_rtImageArea.left;
    rtLotViewRegionF.top = m_procDlgInfo.m_rtImageArea.top;
    rtLotViewRegionF.right = m_procDlgInfo.m_rtImageArea.right;
    rtLotViewRegionF.bottom = (m_procDlgInfo.m_rtImageArea.bottom / 2) - 1;

    rtLotViewRegionR.left = m_procDlgInfo.m_rtImageArea.left;
    rtLotViewRegionR.top = (m_procDlgInfo.m_rtImageArea.bottom / 2) + 1;
    rtLotViewRegionR.right = m_procDlgInfo.m_rtImageArea.right;
    rtLotViewRegionR.bottom = m_procDlgInfo.m_rtImageArea.bottom;

    // Side Front
    m_imageLotView = new ImageLotView(
        rtLotViewRegionF, *m_pVisionInsp, true, true, GetSafeHwnd(), enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    m_imageLotView->SetMessage_SelChangedRaw(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW);
    m_imageLotView->SetMessage_SelChangedImage(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE);

    // Side Rear
    m_imageLotViewRearSide = new ImageLotView(
        rtLotViewRegionR, *m_pVisionInsp, true, true, GetSafeHwnd(), enSideVisionModule::SIDE_VISIONMODULE_REAR);

    m_imageLotViewRearSide->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotViewRearSide->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    m_imageLotViewRearSide->SetMessage_SelChangedRaw(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW);
    m_imageLotViewRearSide->SetMessage_SelChangedImage(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE);

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    ////// Side Status Selector 위치 설정 - 2024.06.10_JHB
    CRect rtSIdeFrontStatus, rtSIdeRearStatus;

    GetDlgItem(IDC_STATIC_SIDE_FRONT_ALIGN2D)->GetWindowRect(rtSIdeFrontStatus);
    GetDlgItem(IDC_STATIC_SIDE_REAR_ALIGN2D)->GetWindowRect(rtSIdeRearStatus);

    SetDlgItemText(IDC_STATIC_SIDE_FRONT_ALIGN2D, _T("SIDE FRONT"));
    SetDlgItemText(IDC_STATIC_SIDE_REAR_ALIGN2D, _T("SIDE REAR"));

    m_Label_Side_Front_Status.MoveWindow(m_procDlgInfo.m_rtSideFrontStatusArea);

    m_Label_Side_Rear_Status.MoveWindow(m_procDlgInfo.m_rtSideRearStatusArea);

    SetInitialSideVisionSelector();
    ////////////////////////////////////////////////////////

    // Dialog Control 위치 설정
    CRect rtClient;
    GetClientRect(rtClient);

    // 강제로 Log 위치를 줄어고 파라메터 창을 늘렸다.
    // 나중에 이런것들을 고려해서 UI을 배치할 수 있는 법을 생각해 봤으면 좋겠다.

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    rtTab.right = m_procDlgInfo.m_rtParaArea.left;
    m_TabResult.MoveWindow(rtTab);

    CRect rtPara = m_procDlgInfo.m_rtParaArea;
    rtPara.bottom = rtTab.bottom;

    m_TabResult.InsertItem(TAB_DEBUG_INFO, _T("Debug"));
    m_TabResult.InsertItem(TAB_TXT_LOG, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    // Debug 및 Log Tab을 생성하는데, Side Vision F/R에 따라서 Show/Unshow 결정
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonLogDlgRear = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlgRear = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotViewRearSide, *m_procCommonLogDlgRear);

    ShowAlign2DSideVisionUI();
}

void DlgVisionProcessingAlign2D::ShowAlign2DNormalUI()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (bIsSideVision)
        return;

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    rtTab.right = m_procDlgInfo.m_rtParaArea.left;
    m_TabResult.MoveWindow(rtTab);

    CRect rtPara = m_procDlgInfo.m_rtParaArea;
    rtPara.bottom = rtTab.bottom;

    m_procCommonGridGroup = new ProcCommonGridGroupDlg(this, rtPara);

    if (m_pVisionInsp->getInspectionAreaInfo().m_stichCountX > 1
        || m_pVisionInsp->getInspectionAreaInfo().m_stichCountY > 1)
    {
        m_procCommonGridGroup->add(new Property_Stitch(
            *m_pVisionInsp, m_imageLotView, *m_procCommonDebugInfoDlg, m_pVisionInsp->m_VisionPara));
    }

    m_procCommonGridGroup->add(new VisionEdgeAlign::PropertyGrid(*m_pVisionInsp, *m_pVisionInsp->m_edgeAlign_inspection,
        m_imageLotView, *m_procCommonDebugInfoDlg, *m_pVisionInsp->m_edgeAlign_para));
}

void DlgVisionProcessingAlign2D::ShowAlign2DSideVisionUI()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (bIsSideVision == false)
        return;

    // Stitch, Empty, Align Tab 생성
    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    rtTab.right = m_procDlgInfo.m_rtParaArea.left;
    m_TabResult.MoveWindow(rtTab);

    CRect rtPara = m_procDlgInfo.m_rtParaArea;
    rtPara.bottom = rtTab.bottom;

    // Front/Rear 둘 다 미리 생성해놓는다..
    m_procCommonGridGroup = new ProcCommonGridGroupDlg(this, rtPara);
    m_procCommonGridGroupRear = new ProcCommonGridGroupDlg(this, rtPara);

    if (m_pVisionInsp->getInspectionAreaInfo().m_stichCountX > 1
        || m_pVisionInsp->getInspectionAreaInfo().m_stichCountY > 1)
    {
        m_procCommonGridGroup->add(new Property_Stitch(*m_pVisionInsp, m_imageLotView, m_imageLotViewRearSide,
            *m_procCommonDebugInfoDlg, m_pVisionInsp->m_VisionPara));

        m_procCommonGridGroupRear->add(new Property_Stitch(*m_pVisionInsp, m_imageLotView, m_imageLotViewRearSide,
            *m_procCommonDebugInfoDlgRear, m_pVisionInsp->m_VisionPara));
    }

    m_procCommonGridGroup->add(new VisionEdgeAlign::PropertyGrid(*m_pVisionInsp, *m_pVisionInsp->m_edgeAlign_inspection,
        m_imageLotView, *m_procCommonDebugInfoDlg, *m_pVisionInsp->m_edgeAlign_para));

    m_procCommonGridGroupRear->add(
        new VisionEdgeAlign::PropertyGrid(*m_pVisionInsp, *m_pVisionInsp->m_edgeAlign_inspection,
            m_imageLotViewRearSide, *m_procCommonDebugInfoDlgRear, *m_pVisionInsp->m_edgeAlign_para));

    // Side Vision Module에 따라 Dialog를 Show or Hide할지 결정
    const long nSideVisionModule = m_pVisionInsp->GetCurVisionModule_Status();

    if (nSideVisionModule == enSideVisionModule::SIDE_VISIONMODULE_FRONT || nSideVisionModule < 0)
    {
        m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);
        m_procCommonGridGroup->ShowWindow(SW_SHOW);
        m_imageLotView->Overlay_Show(TRUE);
        m_imageLotView->ROI_Show(TRUE);

        m_procCommonDebugInfoDlgRear->ShowWindow(SW_HIDE);
        m_procCommonGridGroupRear->ShowWindow(SW_HIDE);
        m_imageLotViewRearSide->Overlay_Show(FALSE);
        m_imageLotViewRearSide->ROI_Show(FALSE);
    }
    else if (nSideVisionModule == enSideVisionModule::SIDE_VISIONMODULE_REAR
        || nSideVisionModule > enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_procCommonDebugInfoDlg->ShowWindow(SW_HIDE);
        m_procCommonGridGroup->ShowWindow(SW_HIDE);
        m_imageLotView->Overlay_Show(FALSE);
        m_imageLotView->ROI_Show(FALSE);

        m_procCommonDebugInfoDlgRear->ShowWindow(SW_SHOW);
        m_procCommonGridGroupRear->ShowWindow(SW_SHOW);
        m_imageLotViewRearSide->Overlay_Show(TRUE);
        m_imageLotViewRearSide->ROI_Show(TRUE);
    }
}

void DlgVisionProcessingAlign2D::OnDestroy()
{
    CDialog::OnDestroy();

    // Front
    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonGridGroup->DestroyWindow();

    // Rear
    if (m_procCommonDebugInfoDlgRear != nullptr)
        m_procCommonDebugInfoDlgRear->DestroyWindow();
    if (m_procCommonLogDlgRear != nullptr)
        m_procCommonLogDlgRear->DestroyWindow();
    if (m_procCommonGridGroupRear != nullptr)
        m_procCommonGridGroupRear->DestroyWindow();
}

void DlgVisionProcessingAlign2D::OnClose()
{
    CDialog::OnDestroy();
}

void DlgVisionProcessingAlign2D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision)
    {
        if (m_pVisionInsp->GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_FRONT)
        {
            m_procCommonDebugInfoDlgRear->ShowWindow(SW_HIDE);
            m_procCommonLogDlgRear->ShowWindow(SW_HIDE);

            m_procCommonDebugInfoDlg->ShowWindow(nTab == TAB_DEBUG_INFO ? SW_SHOW : SW_HIDE);
            m_procCommonLogDlg->ShowWindow(nTab == TAB_TXT_LOG ? SW_SHOW : SW_HIDE);
        }
        else if (m_pVisionInsp->GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
        {
            m_procCommonDebugInfoDlg->ShowWindow(SW_HIDE);
            m_procCommonLogDlg->ShowWindow(SW_HIDE);

            m_procCommonDebugInfoDlgRear->ShowWindow(nTab == TAB_DEBUG_INFO ? SW_SHOW : SW_HIDE);
            m_procCommonLogDlgRear->ShowWindow(nTab == TAB_TXT_LOG ? SW_SHOW : SW_HIDE);
        }
    }
    else
    {
        m_procCommonDebugInfoDlg->ShowWindow(nTab == TAB_DEBUG_INFO ? SW_SHOW : SW_HIDE);
        m_procCommonLogDlg->ShowWindow(nTab == TAB_TXT_LOG ? SW_SHOW : SW_HIDE);
    }

    if (pResult)
    {
        *pResult = 0;
    }
}

void DlgVisionProcessingAlign2D::OnBnClickedButtonInspect()
{
    // Side Vision일 경우와 아닌 경우 분기
    static const long nVisionType = SystemConfig::GetInstance().GetVisionType();
    const long nSideVisionModule = m_pVisionInsp->GetCurVisionModule_Status();

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    if (nVisionType != VISIONTYPE_SIDE_INSP
        || (nVisionType == VISIONTYPE_SIDE_INSP && nSideVisionModule == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
    {
        m_imageLotView->Overlay_RemoveAll();
        m_procCommonLogDlg->SetLogBoxText(_T(""));

        visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
        visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

        m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
        m_procCommonGridGroup->event_afterInspection();

        // 검사 중간 결과 화면 갱신
        m_procCommonDebugInfoDlg->Refresh();
    }
    else
    {
        m_imageLotViewRearSide->Overlay_RemoveAll();
        m_procCommonLogDlgRear->SetLogBoxText(_T(""));

        visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
        visionUnit.GetInspectionOverlayResult().Apply(m_imageLotViewRearSide->GetCoreView());

        m_procCommonLogDlgRear->SetLogBoxText(visionUnit.GetLastInspection_Text());
        m_procCommonGridGroupRear->event_afterInspection();

        // 검사 중간 결과 화면 갱신
        m_procCommonDebugInfoDlgRear->Refresh();
    }

    if (m_pVisionInsp->m_VisionPara.m_type == enumAlgorithmType::BodySize)
    {
        if (!m_pVisionInsp->m_result.m_additionalCriticalLog.IsEmpty())
        {
            MessageBox(m_pVisionInsp->m_result.m_additionalCriticalLog);
        }
    }
}

LRESULT DlgVisionProcessingAlign2D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedPane();

    return 0L;
}

LRESULT DlgVisionProcessingAlign2D::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedRoi();

    return 0L;
}

LRESULT DlgVisionProcessingAlign2D::OnImageLotViewSelChangedRaw(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedRaw();

    return 0L;
}

LRESULT DlgVisionProcessingAlign2D::OnImageLotViewSelChangedImage(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedImage();

    return 0L;
}

void DlgVisionProcessingAlign2D::SetInitialSideVisionSelector()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (bIsSideVision == false)
        return;

    // Default Color 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    const long nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    // 혹시 모를 쓰레기 값을 방지하기 위해 FRONT/REAR 값 이외의 값이 들어오면 무조건 FRONT로 변경시킨다.
    switch (nSideSection)
    {
        case enSideVisionModule::SIDE_VISIONMODULE_REAR:
            m_Label_Side_Front_Status.SetBkColor(defColor);
            m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
            break;

        case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
        default:
            m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
            m_Label_Side_Rear_Status.SetBkColor(defColor);
            break;
    }
}

void DlgVisionProcessingAlign2D::OnStnClickedStaticSideFrontAlign2d()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (bIsSideVision == false)
        return;

    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);
    const long nTab = m_TabResult.GetCurSel();
    const bool bShowDebugInfo = (nTab == TAB_DEBUG_INFO);
    const bool bShowLog = (nTab == TAB_TXT_LOG);

    m_pVisionInsp->ResetDebugInfo();

    if (m_pVisionInsp->GetCurVisionModule_Status() != enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
        m_Label_Side_Rear_Status.SetBkColor(defColor);

        m_pVisionInsp->m_visionUnit.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        m_pVisionInsp->SetCurVisionModule_Status(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        m_imageLotView->SetSideVisionModule(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

        // 1. 기존 Side Vision에서 사용한 Tab 정보 저장
        const long nGridTabNum = m_procCommonGridGroupRear->GetGridTab();

        // 2. 기존 GridGroup Hide
        m_procCommonGridGroupRear->ShowWindow(SW_HIDE);
        m_procCommonDebugInfoDlgRear->ShowWindow(SW_HIDE);
        m_procCommonLogDlgRear->ShowWindow(SW_HIDE);

        m_imageLotViewRearSide->Overlay_Show(FALSE);
        m_imageLotViewRearSide->ROI_Show(FALSE);

        // 3. 해당 SIDE VISION에 맞는 UI Show
        m_procCommonGridGroup->ShowWindow(SW_SHOW);
        m_procCommonDebugInfoDlg->ShowWindow(bShowDebugInfo ? SW_SHOW : SW_HIDE);
        m_procCommonLogDlg->ShowWindow(bShowLog ? SW_SHOW : SW_HIDE);
        m_procCommonDebugInfoDlg->Refresh();

        m_imageLotView->Overlay_Show(TRUE);
        m_imageLotView->ROI_Show(TRUE);

        // 4. 기존 SIDE VISION에서 사용한 Tab으로 이동
        m_procCommonGridGroup->SetGridTab(nGridTabNum);
    }
    else
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
    }
}

void DlgVisionProcessingAlign2D::OnStnClickedStaticSideRearAlign2d()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (bIsSideVision == false)
        return;

    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);
    const long nTab = m_TabResult.GetCurSel();
    const bool bShowDebugInfo = (nTab == TAB_DEBUG_INFO);
    const bool bShowLog = (nTab == TAB_TXT_LOG);

    m_pVisionInsp->ResetDebugInfo();

    if (m_pVisionInsp->GetCurVisionModule_Status() != enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_Label_Side_Front_Status.SetBkColor(defColor);
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));

        m_pVisionInsp->m_visionUnit.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_REAR);
        m_pVisionInsp->SetCurVisionModule_Status(enSideVisionModule::SIDE_VISIONMODULE_REAR);
        m_imageLotViewRearSide->SetSideVisionModule(enSideVisionModule::SIDE_VISIONMODULE_REAR);

        // 1. 기존 Side Vision에서 사용한 Tab 정보 저장
        const long nGridTabNum = m_procCommonGridGroup->GetGridTab();

        // 2. 기존 GridGroup Hide
        m_procCommonGridGroup->ShowWindow(SW_HIDE);
        m_procCommonDebugInfoDlg->ShowWindow(SW_HIDE);
        m_procCommonLogDlg->ShowWindow(SW_HIDE);

        m_imageLotView->Overlay_Show(FALSE);
        m_imageLotView->ROI_Show(FALSE);

        // 3. 해당 SIDE VISION에 맞는 UI Show
        m_procCommonGridGroupRear->ShowWindow(SW_SHOW);
        m_procCommonDebugInfoDlgRear->ShowWindow(bShowDebugInfo ? SW_SHOW : SW_HIDE);
        m_procCommonLogDlgRear->ShowWindow(bShowLog ? SW_SHOW : SW_HIDE);
        m_procCommonDebugInfoDlgRear->Refresh();

        m_imageLotViewRearSide->Overlay_Show(TRUE);
        m_imageLotViewRearSide->ROI_Show(TRUE);

        // 4. 기존 SIDE VISION에서 사용한 Tab으로 이동
        m_procCommonGridGroupRear->SetGridTab(nGridTabNum);
    }
    else
    {
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
    }
}
