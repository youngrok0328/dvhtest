//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingTapeDetailAlign.h"

//CPP_2_________________________________ This project's headers
#include "Para.h"
#include "VisionProcessingTapeDetailAlign.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
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

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgVisionProcessingTapeDetailAlign, CDialog)

DlgVisionProcessingTapeDetailAlign::DlgVisionProcessingTapeDetailAlign(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingTapeDetailAlign* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(DlgVisionProcessingTapeDetailAlign::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_specTab(*pVisionInsp)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
{
    m_pVisionInsp = pVisionInsp;
}

DlgVisionProcessingTapeDetailAlign::~DlgVisionProcessingTapeDetailAlign()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_imageLotView;

    m_pVisionInsp->m_pVisionInspDlg = nullptr;
}

void DlgVisionProcessingTapeDetailAlign::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(DlgVisionProcessingTapeDetailAlign, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &DlgVisionProcessingTapeDetailAlign::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &DlgVisionProcessingTapeDetailAlign::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &DlgVisionProcessingTapeDetailAlign::OnImageLotViewRoiChanged)
END_MESSAGE_MAP()

// DlgVisionProcessingTapeDetailAlign 메시지 처리기입니다.

BOOL DlgVisionProcessingTapeDetailAlign::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    m_specTab.Create(m_procDlgInfo.m_rtParaArea, this);

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Debug"));
    m_TabResult.InsertItem(1, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgVisionProcessingTapeDetailAlign::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
}

void DlgVisionProcessingTapeDetailAlign::OnClose()
{
    CDialog::OnDestroy();
}

void DlgVisionProcessingTapeDetailAlign::SetInitParaWindow()
{
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0));

    m_specTab.UpdatePropertyGrid();
}

void DlgVisionProcessingTapeDetailAlign::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp);
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();
}

void DlgVisionProcessingTapeDetailAlign::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_procCommonDebugInfoDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 1 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT DlgVisionProcessingTapeDetailAlign::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_pVisionInspParaDlg->SetROI();

    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing());
    m_pVisionInsp->DoInspection(true);

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);

    return 0L;
}

LRESULT DlgVisionProcessingTapeDetailAlign::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_pVisionInspParaDlg->GetROI();

    return 0L;
}
