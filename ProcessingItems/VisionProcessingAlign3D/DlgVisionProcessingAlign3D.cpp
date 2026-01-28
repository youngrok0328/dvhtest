//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingAlign3D.h"

//CPP_2_________________________________ This project's headers
#include "Property_Stitch.h"
#include "VisionProcessingAlign3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
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

#define UM_IMAGE_LOT_VIEW_PANE_CHANGED (WM_USER + 1011)
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1012)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_RAW (WM_USER + 1013)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE (WM_USER + 1014)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgVisionProcessingAlign3D, CDialog)

CDlgVisionProcessingAlign3D::CDlgVisionProcessingAlign3D(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingAlign3D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionProcessingAlign3D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_packageSpec(pVisionInsp->m_packageSpec)
    , m_visionPara(pVisionInsp->m_VisionPara)
    , m_3DMergeResult(*pVisionInsp->m_3DMergeResult)
    , m_imageLotView(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_procCommonGridGroup(nullptr)
{
}

CDlgVisionProcessingAlign3D::~CDlgVisionProcessingAlign3D()
{
    delete m_procCommonGridGroup;
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_imageLotView;

    m_pVisionInsp->m_pVisionInspDlg = nullptr;
}

void CDlgVisionProcessingAlign3D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_tabResult);
}

BEGIN_MESSAGE_MAP(CDlgVisionProcessingAlign3D, CDialog)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionProcessingAlign3D::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &CDlgVisionProcessingAlign3D::OnImageLotViewRoiChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_CHANGED, &CDlgVisionProcessingAlign3D::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW, CDlgVisionProcessingAlign3D::OnImageLotViewSelChangedRaw)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE, CDlgVisionProcessingAlign3D::OnImageLotViewSelChangedImage)
END_MESSAGE_MAP()

// CDlgVisionProcessingAlign3D 메시지 처리기입니다.

BOOL CDlgVisionProcessingAlign3D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(rtClient);
    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, true, true, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_CHANGED);
    m_imageLotView->SetMessage_SelChangedRaw(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW);
    m_imageLotView->SetMessage_SelChangedImage(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE);

    if (m_pVisionInsp->getInspectionAreaInfo().m_stichCountX > 1
        || m_pVisionInsp->getInspectionAreaInfo().m_stichCountY > 1) //Stitching을 해야하는경우
        m_imageLotView->ShowImage(0, true);
    else
        m_imageLotView->ShowImage(0, true, true);

    // Dialog Control 위치 설정
    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_tabResult.MoveWindow(rtTab);

    m_tabResult.InsertItem(0, _T("Debug"));
    m_tabResult.InsertItem(1, _T("Log"));

    m_tabResult.AdjustRect(FALSE, rtTab);

    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);

    CWnd::FromHandle(m_imageLotView->GetSafeHwnd())->MoveWindow(m_procDlgInfo.m_rtImageArea);
    CWnd::FromHandle(m_imageLotView->GetSafeHwnd())->ShowWindow(SW_SHOW);

    ShowWindow(SW_SHOW);

    m_procCommonGridGroup = new ProcCommonGridGroupDlg(this, m_procDlgInfo.m_rtParaArea);

    bool bIsSrcRawImage(false);
    if (m_pVisionInsp->getInspectionAreaInfo().m_stichCountX > 1
        || m_pVisionInsp->getInspectionAreaInfo().m_stichCountY > 1)
    {
        m_procCommonGridGroup->add(
            new Property_Stitch(*m_pVisionInsp, m_imageLotView, *m_procCommonDebugInfoDlg, m_visionPara));
        bIsSrcRawImage = true;
    }

    m_procCommonGridGroup->add(new VisionEdgeAlign::PropertyGrid(*m_pVisionInsp, *m_pVisionInsp->m_edgeAlign_inspection,
        m_imageLotView, *m_procCommonDebugInfoDlg, *m_pVisionInsp->m_edgeAlign_para));

    m_imageLotView->ROI_Show(bIsSrcRawImage);

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionProcessingAlign3D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_tabResult.GetCurSel();

    m_procCommonDebugInfoDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 1 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

void CDlgVisionProcessingAlign3D::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_procCommonGridGroup->event_afterInspection();

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();
}

LRESULT CDlgVisionProcessingAlign3D::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedRoi();

    return 0L;
}

LRESULT CDlgVisionProcessingAlign3D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedPane();

    return 0L;
}

LRESULT CDlgVisionProcessingAlign3D::OnImageLotViewSelChangedRaw(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedRaw();

    return 0L;
}

LRESULT CDlgVisionProcessingAlign3D::OnImageLotViewSelChangedImage(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_procCommonGridGroup->event_changedImage();

    return 0L;
}
