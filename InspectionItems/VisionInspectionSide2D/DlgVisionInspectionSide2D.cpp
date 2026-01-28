//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionSide2D.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionSide2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h" //kircheis_IllumCal
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)

//CPP_7_________________________________ Implementation body
//
enum enumSetupInfoTab
{
    TAB_INSPECTION_SPEC = 0,
    TAB_INSPECTION_RESULT,
    TAB_DETAIL_RESULT,
    TAB_DEBUG_INFO,
    TAB_TXT_LOG,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionSide2D, CDialog)

CDlgVisionInspectionSide2D::CDlgVisionInspectionSide2D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionSide2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionSide2D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_pVisionPara(pVisionInsp->m_VisionPara)
    , m_imageLotView(nullptr)
    , m_imageLotViewRearSide(nullptr)
    , m_propertyGrid(nullptr)
    , m_procCommonSpecDlg(nullptr)
    //Normal & Front
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    //Rear
    , m_procCommonResultDlgRear(nullptr)
    , m_procCommonDetailResultDlgRear(nullptr)
    , m_procCommonLogDlgRear(nullptr)
    , m_procCommonDebugInfoDlgRear(nullptr)
    , m_nCurCommonTab(TAB_INSPECTION_SPEC)
{
}

CDlgVisionInspectionSide2D::~CDlgVisionInspectionSide2D()
{
    //Rear
    delete m_procCommonDebugInfoDlgRear;
    delete m_procCommonLogDlgRear;
    delete m_procCommonDetailResultDlgRear;
    delete m_procCommonResultDlgRear;
    //Normal & Front
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;

    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    delete m_imageLotView;
    delete m_imageLotViewRearSide;
}

void CDlgVisionInspectionSide2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_STATIC_SIDE_FRONT_SIDE2D, m_Label_Side_Front_Status);
    DDX_Control(pDX, IDC_STATIC_SIDE_REAR_SIDE2D, m_Label_Side_Rear_Status);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionSide2D, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionSide2D::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionSide2D::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionSide2D::OnImageLotViewPaneSelChanged)
ON_STN_CLICKED(IDC_STATIC_SIDE_FRONT_SIDE2D, &CDlgVisionInspectionSide2D::OnStnClickedStaticSideFrontSide2d)
ON_STN_CLICKED(IDC_STATIC_SIDE_REAR_SIDE2D, &CDlgVisionInspectionSide2D::OnStnClickedStaticSideRearSide2d)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionSide2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(rtClient);

    // Side Main Image View - 2024.05.29_JHB
    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        return FALSE;
    }

    CRect rtLotViewRegionF, rtLotViewRegionR;
    rtLotViewRegionF.left = m_procDlgInfo.m_rtImageArea.left;
    rtLotViewRegionF.top = m_procDlgInfo.m_rtImageArea.top;
    rtLotViewRegionF.right = m_procDlgInfo.m_rtImageArea.right;
    rtLotViewRegionF.bottom = (m_procDlgInfo.m_rtImageArea.bottom / 2) - 1;

    rtLotViewRegionR.left = m_procDlgInfo.m_rtImageArea.left;
    rtLotViewRegionR.top = (m_procDlgInfo.m_rtImageArea.bottom / 2) + 1;
    rtLotViewRegionR.right = m_procDlgInfo.m_rtImageArea.right;
    rtLotViewRegionR.bottom = m_procDlgInfo.m_rtImageArea.bottom;

    m_imageLotView = new ImageLotView(
        rtLotViewRegionF, *m_pVisionInsp, GetSafeHwnd(), enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    m_imageLotViewRearSide
        = new ImageLotView(rtLotViewRegionR, *m_pVisionInsp, GetSafeHwnd(), enSideVisionModule::SIDE_VISIONMODULE_REAR);

    /////////////////////////////////////////////

    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    m_imageLotViewRearSide->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);

    ////// Side Status Selector 위치 설정 - 2024.05.29_JHB
    CRect rtSIdeFrontStatus, rtSIdeRearStatus;

    GetDlgItem(IDC_STATIC_SIDE_FRONT_SIDE2D)->GetWindowRect(rtSIdeFrontStatus);
    GetDlgItem(IDC_STATIC_SIDE_REAR_SIDE2D)->GetWindowRect(rtSIdeRearStatus);

    SetDlgItemText(IDC_STATIC_SIDE_FRONT_SIDE2D, _T("SIDE FRONT"));
    SetDlgItemText(IDC_STATIC_SIDE_REAR_SIDE2D, _T("SIDE REAR"));

    m_Label_Side_Front_Status.MoveWindow(m_procDlgInfo.m_rtSideFrontStatusArea);

    m_Label_Side_Rear_Status.MoveWindow(m_procDlgInfo.m_rtSideRearStatusArea);

    SetInitialSideVisionSelector();
    ////////////////////////////////////////////////////////

    // Dialog Control 위치 설정
    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_fixedInspectionSpecs);
    //Normal & Front
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);
    //Rear
    m_procCommonResultDlgRear = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup,
        *m_imageLotViewRearSide, m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlgRear
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotViewRearSide);
    m_procCommonLogDlgRear = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlgRear = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotViewRearSide, *m_procCommonLogDlgRear);

    UpdateCommonTabShow();

    UpdatePropertyGrid();

    // ROI 표시 - 2024.05.29_JHB
    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        return FALSE;
    }

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    m_imageLotViewRearSide->Overlay_RemoveAll();
    m_imageLotViewRearSide->ROI_RemoveAll();

    m_imageLotViewRearSide->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotViewRearSide->ZoomImageFit();

    ///////////////////////////////////

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionInspectionSide2D::OnDestroy()
{
    CDialog::OnDestroy();

    //Rear
    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    //Normal & Front
    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();

    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionSide2D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionSide2D::OnBnClickedButtonInspect()
{
    const long nVisionType = SystemConfig::GetInstance().GetVisionType();
    const long nCurSideSelection = m_pVisionInsp->GetCurVisionModule_Status();

    // 1. VisionType이 Side Vision이 아닐 경우
    // 2. VisionType이 Side Vision이면서 Side Front일 경우
    // --> 기존 m_imageLotView를 사용
    if (nVisionType != VISIONTYPE_SIDE_INSP
        || (nVisionType == VISIONTYPE_SIDE_INSP && nCurSideSelection == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
    {
        m_imageLotView->Overlay_RemoveAll();
        m_procCommonLogDlg->SetLogBoxText(_T(""));

        if (m_pVisionInsp->GetImageFrameCount())
        {
            auto frameIndex = m_pVisionInsp->GetImageFrameIndex(0);
            m_imageLotView->ShowImage(frameIndex);
        }

        auto& visionUnit = m_pVisionInsp->m_visionUnit;

        visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
        visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

        m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
        m_imageLotView->Overlay_Show(TRUE);

        // 검사 중간 결과 화면 갱신
        m_procCommonDebugInfoDlg->Refresh();

        // 검사 결과 화면 갱신
        m_procCommonResultDlg->Refresh();
    }
    else
    {
        m_imageLotViewRearSide->Overlay_RemoveAll();
        m_procCommonLogDlgRear->SetLogBoxText(_T(""));

        if (m_pVisionInsp->GetImageFrameCount())
        {
            auto frameIndex = m_pVisionInsp->GetImageFrameIndex(0);
            m_imageLotViewRearSide->ShowImage(frameIndex);
        }

        auto& visionUnit = m_pVisionInsp->m_visionUnit;

        visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
        visionUnit.GetInspectionOverlayResult().Apply(m_imageLotViewRearSide->GetCoreView());

        m_procCommonLogDlgRear->SetLogBoxText(visionUnit.GetLastInspection_Text());
        m_imageLotViewRearSide->Overlay_Show(TRUE);

        // 검사 중간 결과 화면 갱신
        m_procCommonDebugInfoDlgRear->Refresh();

        // 검사 결과 화면 갱신
        m_procCommonResultDlgRear->Refresh();
    }

    // 검사 결과 탭 보여주기
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
}

void CDlgVisionInspectionSide2D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    m_nCurCommonTab = m_TabResult.GetCurSel();

    UpdateCommonTabShow();

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT CDlgVisionInspectionSide2D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
{
    const long itemIndex = long(wparam);

    if (itemIndex < 0)
    {
        m_TabResult.SetCurSel(1);
    }
    else
    {
        const long nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

        m_TabResult.SetCurSel(TAB_DETAIL_RESULT);
        m_nCurCommonTab = TAB_DETAIL_RESULT;

        if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            m_procCommonDetailResultDlgRear->Refresh(itemIndex);
        else
            m_procCommonDetailResultDlg->Refresh(itemIndex);
    }

    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0L;
}

LRESULT CDlgVisionInspectionSide2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(lparam);
    //CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        //switch (item->GetID())
        //{
        //}
    }

    return 0;
}

void CDlgVisionInspectionSide2D::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();
    /*
	if (auto *category = m_propertyGrid->AddCategory(_T("Image")))
	{
		category->Expand();
	}

	if (auto *category = m_propertyGrid->AddCategory(_T("Inspection Parameter")))
	{
		category->Expand();
	}
	*/
    m_propertyGrid->SetViewDivider(0.4);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionInspectionSide2D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_SIDE_DETAIL_ALIGN), false,
        m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    const long nVisionType = SystemConfig::GetInstance().GetVisionType();
    const long nCurSideSelection = m_pVisionInsp->GetCurVisionModule_Status();

    // 1. VisionType이 Side Vision이 아닐 경우
    // 2. VisionType이 Side Vision이면서 Side Front일 경우
    // --> 기존 m_imageLotView를 사용
    if (nVisionType != VISIONTYPE_SIDE_INSP
        || (nVisionType == VISIONTYPE_SIDE_INSP && nCurSideSelection == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
    {
        m_imageLotView->ShowCurrentImage(false);
        m_imageLotView->ZoomImageFit();
    }
    else
    {
        m_imageLotViewRearSide->ShowCurrentImage(false);
        m_imageLotViewRearSide->ZoomImageFit();
    }

    return 0L;
}

void CDlgVisionInspectionSide2D::SetInitialSideVisionSelector()
{
    // Default Color 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    const long nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    // 혹시 모를 쓰레기 값을 방지하기 위해 FRONT/REAR 값 이외의 값이 들어오면 무조건 FRONT로 변경시킨다.
    switch (nSideSection)
    {
        case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
        default:
            m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
            m_Label_Side_Rear_Status.SetBkColor(defColor);
            break;

        case enSideVisionModule::SIDE_VISIONMODULE_REAR:
            m_Label_Side_Front_Status.SetBkColor(defColor);
            m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
            break;
    }
}

void CDlgVisionInspectionSide2D::OnStnClickedStaticSideFrontSide2d()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_pVisionInsp->GetCurVisionModule_Status() != enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
        m_Label_Side_Rear_Status.SetBkColor(defColor);

        m_pVisionInsp->m_visionUnit.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        m_pVisionInsp->SetCurVisionModule_Status(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    }
    else
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
    }
    m_pVisionInsp->ResetDebugInfo();

    if (m_pVisionInsp->m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_SIDE_DETAIL_ALIGN))
        m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_SIDE_DETAIL_ALIGN), false,
            m_pVisionInsp->GetCurVisionModule_Status());

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    UpdateCommonTabShow();
}

void CDlgVisionInspectionSide2D::OnStnClickedStaticSideRearSide2d()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_pVisionInsp->GetCurVisionModule_Status() != enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_Label_Side_Front_Status.SetBkColor(defColor);
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));

        m_pVisionInsp->m_visionUnit.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_REAR);
        m_pVisionInsp->SetCurVisionModule_Status(enSideVisionModule::SIDE_VISIONMODULE_REAR);
    }
    else
    {
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
    }

    m_pVisionInsp->ResetDebugInfo();

    if (m_pVisionInsp->m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_SIDE_DETAIL_ALIGN))
        m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_SIDE_DETAIL_ALIGN), false,
            m_pVisionInsp->GetCurVisionModule_Status());

    m_imageLotViewRearSide->Overlay_RemoveAll();
    m_imageLotViewRearSide->ROI_RemoveAll();

    m_imageLotViewRearSide->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotViewRearSide->ZoomImageFit();

    UpdateCommonTabShow();
}

void CDlgVisionInspectionSide2D::UpdateCommonTabShow()
{
    const long nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    m_procCommonSpecDlg->ShowWindow(SW_HIDE);

    m_procCommonResultDlg->ShowWindow(SW_HIDE);
    m_procCommonDetailResultDlg->ShowWindow(SW_HIDE);
    m_procCommonDebugInfoDlg->ShowWindow(SW_HIDE);
    m_procCommonLogDlg->ShowWindow(SW_HIDE);

    m_procCommonResultDlgRear->ShowWindow(SW_HIDE);
    m_procCommonDetailResultDlgRear->ShowWindow(SW_HIDE);
    m_procCommonDebugInfoDlgRear->ShowWindow(SW_HIDE);
    m_procCommonLogDlgRear->ShowWindow(SW_HIDE);

    if (m_nCurCommonTab == TAB_INSPECTION_SPEC)
    {
        m_procCommonSpecDlg->ShowWindow(SW_SHOW);
        return;
    }

    if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        switch (m_nCurCommonTab)
        {
            case TAB_INSPECTION_RESULT:
                m_procCommonResultDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_DETAIL_RESULT:
                m_procCommonDetailResultDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_DEBUG_INFO:
                m_procCommonDebugInfoDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_TXT_LOG:
                m_procCommonLogDlgRear->ShowWindow(SW_SHOW);
                break;
            default:
                m_procCommonSpecDlg->ShowWindow(SW_SHOW);
                break;
        }
    }
    else
    {
        switch (m_nCurCommonTab)
        {
            case TAB_INSPECTION_RESULT:
                m_procCommonResultDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_DETAIL_RESULT:
                m_procCommonDetailResultDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_DEBUG_INFO:
                m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_TXT_LOG:
                m_procCommonLogDlg->ShowWindow(SW_SHOW);
                break;
            default:
                m_procCommonSpecDlg->ShowWindow(SW_SHOW);
                break;
        }
    }
}
