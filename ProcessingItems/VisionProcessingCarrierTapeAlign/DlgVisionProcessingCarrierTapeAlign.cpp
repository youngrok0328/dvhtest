//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingCarrierTapeAlign.h"

//CPP_2_________________________________ This project's headers
#include "PropertyGrid.h"
#include "VisionProcessingCarrierTapeAlign.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonGridGroupDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

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

IMPLEMENT_DYNAMIC(DlgVisionProcessingCarrierTapeAlign, CDialog)

DlgVisionProcessingCarrierTapeAlign::DlgVisionProcessingCarrierTapeAlign(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingCarrierTapeAlign* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(DlgVisionProcessingCarrierTapeAlign::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_procCommonGridGroup(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
{
    m_pVisionInsp = pVisionInsp;
}

DlgVisionProcessingCarrierTapeAlign::~DlgVisionProcessingCarrierTapeAlign()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonGridGroup;
    delete m_imageLotView;

    m_pVisionInsp->m_pVisionInspDlg = nullptr;
}

void DlgVisionProcessingCarrierTapeAlign::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(DlgVisionProcessingCarrierTapeAlign, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &DlgVisionProcessingCarrierTapeAlign::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &DlgVisionProcessingCarrierTapeAlign::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &DlgVisionProcessingCarrierTapeAlign::OnImageLotViewRoiChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW, &DlgVisionProcessingCarrierTapeAlign::OnImageLotViewSelChangedRaw)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE, DlgVisionProcessingCarrierTapeAlign::OnImageLotViewSelChangedImage)
END_MESSAGE_MAP()

// DlgVisionProcessingCarrierTapeAlign 메시지 처리기입니다.

BOOL DlgVisionProcessingCarrierTapeAlign::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetAlign2D_NormalVision();

    return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgVisionProcessingCarrierTapeAlign::SetAlign2D_NormalVision()
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

void DlgVisionProcessingCarrierTapeAlign::ShowAlign2DNormalUI()
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
    m_procCommonGridGroup->add(new PropertyGrid(*m_pVisionInsp, *m_pVisionInsp->m_inspector, m_imageLotView,
        *m_procCommonDebugInfoDlg, m_pVisionInsp->m_visionPara));
}

void DlgVisionProcessingCarrierTapeAlign::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonGridGroup->DestroyWindow();
}

void DlgVisionProcessingCarrierTapeAlign::OnClose()
{
    CDialog::OnDestroy();
}

void DlgVisionProcessingCarrierTapeAlign::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    m_procCommonDebugInfoDlg->ShowWindow(nTab == TAB_DEBUG_INFO ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == TAB_TXT_LOG ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

void DlgVisionProcessingCarrierTapeAlign::OnBnClickedButtonInspect()
{
    // Side Vision일 경우와 아닌 경우 분기
    static const long nVisionType = SystemConfig::GetInstance().GetVisionType();
    const long nSideVisionModule = m_pVisionInsp->GetCurVisionModule_Status();

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_procCommonGridGroup->event_afterInspection();

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();
}

LRESULT DlgVisionProcessingCarrierTapeAlign::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedPane();

    return 0L;
}

LRESULT DlgVisionProcessingCarrierTapeAlign::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedRoi();

    return 0L;
}

LRESULT DlgVisionProcessingCarrierTapeAlign::OnImageLotViewSelChangedRaw(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedRaw();

    return 0L;
}

LRESULT DlgVisionProcessingCarrierTapeAlign::OnImageLotViewSelChangedImage(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedImage();

    return 0L;
}
