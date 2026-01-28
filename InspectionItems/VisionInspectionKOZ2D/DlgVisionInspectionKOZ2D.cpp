//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionKOZ2D.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionKOZ2D.h"
#include "VisionInspectionKOZ2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ImageViewEx/PatternImage.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_CHANGED (WM_USER + 1011)
#define UM_ROI_CHANGED (WM_USER + 1012)

//CPP_7_________________________________ Implementation body
//
enum enumKOZInspectionParamID
{
    KOZ_INSP_PARAM_START = 0,
    KOZ_NOTCH_ALIGN_TITLE = KOZ_INSP_PARAM_START,
    KOZ_NOTCH_ALIGN_FRAME_NUM,
    KOZ_NOTCH_ALIGN_IMAGECOMBINE,
    KOZ_NOTCH_ALIGN_EDGE_SEARCH_LENGTH,
    KOZ_ALIGN_TITLE,
    KOZ_ALIGN_FRAME_NUM,
    KOZ_ALIGN_IMAGECOMBINE,
    KOZ_ALIGN_EDGE_SEARCH_LENGTH,
    KOZ_ALIGN_EDGE_SEARCH_WIDTH,
    KOZ_ALIGN_EDGE_SEARCH_GAP,
    KOZ_ALIGN_EDGE_SEARCH_DIRECTION,
    KOZ_ALIGN_EDGE_DIRECTION,
    KOZ_ALIGN_EDGE_TYPE,
    KOZ_ALIGN_EDGE_THRESHOLD_VALUE,
    //KOZ_ALIGN_EDGE_THRESHOLD_LEVEL,
    KOZ_CHIPPING_TITEL,
    KOZ_CHIPPING_FRAME_NUM,
    KOZ_CHIPPING_IMAGECOMBINE,
    KOZ_CHIPPING_IGNORE_OFFSET,
    KOZ_CHIPPING_EXPANSION_OFFSET,
    KOZ_CHIPPING_USE_IGNORE_OBJECT,
    KOZ_CHIPPING_IGNORE_DILATION_CNT,
    KOZ_CHIPPING_MERGE_DISTANCE,
    KOZ_CHIPPING_WINDOW_SIZE_X,
    KOZ_CHIPPING_WINDOW_SIZE_Y,
    KOZ_CHIPPING_DEFECT_COLOR,
    KOZ_CHIPPING_MIN_DARK_CONTRAST,
    KOZ_CHIPPING_MIN_BRIGHT_CONTRAST,
    KOZ_CHIPPING_MIN_BLOB_AREA,
    KOZ_CHIPPING_MIN_BLOB_WIDTH,
    KOZ_CHIPPING_MIN_BLOB_LENGTH,
    KOZ_INSP_PARAM_END,
};

static LPCTSTR g_szKOZInspParamName[] = {
    _T("Notch alignment parameters"), //KOZ_NOTCH_ALIGN_TITLE,
    _T("Notch - Alignment frame"), //KOZ_NOTCH_ALIGN_FRAME_NUM,
    _T("Notch - Image combine"), //KOZ_NOTCH_ALIGN_IMAGECOMBINE,
    _T("Notch - Edge search length ratio (%)"), //KOZ_NOTCH_ALIGN_EDGE_SEARCH_LENGTH,
    _T("KOZ alignment parameters"), //KOZ_ALIGN_TITLE,
    _T("KOZ alignment frame"), //KOZ_ALIGN_FRAME_NUM,
    _T("KOZ image combine"), //KOZ_ALIGN_IMAGECOMBINE,
    _T("Edge search length (um)"), //KOZ_ALIGN_EDGE_SEARCH_LENGTH,
    _T("Edge search width ratio (%)"), //KOZ_ALIGN_EDGE_SEARCH_WIDTH,
    _T("Edge search gap (um)"), //KOZ_ALIGN_EDGE_SEARCH_GAP,
    _T("Edge search direction"), //KOZ_ALIGN_EDGE_SEARCH_DIRECTION,
    _T("Edge direction"), //KOZ_ALIGN_EDGE_DIRECTION,
    _T("Edge type"), //KOZ_ALIGN_EDGE_TYPE,
    _T("Edge threshold value"), //KOZ_ALIGN_EDGE_THRESHOLD_VALUE,
    //_T("Edge threshold level"),							//KOZ_ALIGN_EDGE_THRESHOLD_LEVEL,
    _T("KOZ chipping algorithm parameters"), //KOZ_CHIPPING_TITEL,
    _T("KOZ chipping - Frame"), //KOZ_CHIPPING_FRAME_NUM,
    _T("KOZ chipping - Image combine"), //KOZ_CHIPPING_IMAGECOMBINE,
    _T("KOZ chipping - Ignore offset (um)"), //KOZ_CHIPPING_IGNORE_OFFSET,
    _T("KOZ chipping - Expansion offset (um)"), //KOZ_CHIPPING_EXPANSION_OFFSET,
    _T("KOZ chipping - Use ignore Preprocess Obj."), //KOZ_CHIPPING_USE_IGNORE_OBJECT,
    _T("KOZ chipping - Dilation count to Ignore"), //KOZ_CHIPPING_IGNORE_DILATION_CNT
    _T("KOZ chipping - Merge distance (um)"), //KOZ_CHIPPING_MERGE_DISTANCE,
    _T("KOZ chipping - Window size X (um)"), //KOZ_CHIPPING_WINDOW_SIZE_X,
    _T("KOZ chipping - Window size Y (um)"), //KOZ_CHIPPING_WINDOW_SIZE_Y,
    _T("KOZ chipping - Defect color"), //KOZ_CHIPPING_DEFECT_COLOR,
    _T("KOZ chipping - Min. dark contrast (Blue)"), //KOZ_CHIPPING_MIN_DARK_CONTRAST,
    _T("KOZ chipping - Min. bright contrast (Red)"), //KOZ_CHIPPING_MIN_BRIGHT_CONTRAST,
    _T("KOZ chipping - Min. blob area (um^2)"), //KOZ_CHIPPING_MIN_BLOB_AREA,
    _T("KOZ chipping - Min. blob width (um)"), //KOZ_CHIPPING_MIN_BLOB_WIDTH,
    _T("KOZ chipping - Min. blob length (um)"), //KOZ_CHIPPING_MIN_BLOB_LENGTH,
};

enum enumSearchDirection
{
    SEARCH_DIRECTION_START = 0,
    SEARCH_DIRECTION_IN_OUT = SEARCH_DIRECTION_START,
    SEARCH_DIRECTION_OUT_IN,
    SEARCH_DIRECTION_END,
};

enum enumEdgeDirection
{
    EDGE_DIRECTION_START = 0,
    EDGE_DIRECTION_BOTH = EDGE_DIRECTION_START,
    EDGE_DIRECTION_RISING,
    EDGE_DIRECTION_FALLING,
    EDGE_DIRECTION_END,
};

enum enumEdgeDetectMode
{
    EDGE_DETECT_MODE_START = 0,
    EDGE_DETECT_MODE_BEST = EDGE_DETECT_MODE_START,
    EDGE_DETECT_MODE_FIRST,
    EDGE_DETECT_MODE_END,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionKOZ2D, CDialog)

CDlgVisionInspectionKOZ2D::CDlgVisionInspectionKOZ2D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionKOZ2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionKOZ2D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_propertyGrid(nullptr)
{
    m_pVisionInsp = pVisionInsp;
    m_pVisionInsp->ResetResult(); //kircheis_2DMatVMSDK
    m_pVisionPara = m_pVisionInsp->m_VisionPara;

    m_bShowResultDetail = FALSE;
    m_nImageID = 0;
    m_nBarcodeImage = 0;
}

CDlgVisionInspectionKOZ2D::~CDlgVisionInspectionKOZ2D()
{
    delete m_imageLotView;
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionInspectionKOZ2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_APPLY, m_buttonApply);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
    DDX_Control(pDX, IDC_COMBO_CURRPANE, m_ComboCurrPane);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionKOZ2D, CDialog)
//ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionKOZ2D::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionKOZ2D::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspectionKOZ2D::OnBnClickedButtonClose)
ON_CBN_SELCHANGE(IDC_COMBO_CURRPANE, &CDlgVisionInspectionKOZ2D::OnCbnSelchangeComboCurrpane)
ON_BN_CLICKED(IDC_BUTTON_PREIMAGE, &CDlgVisionInspectionKOZ2D::OnBnClickedButtonPreimage)
ON_BN_CLICKED(IDC_BUTTON_NEXTIMAGE, &CDlgVisionInspectionKOZ2D::OnBnClickedButtonNextimage)
ON_MESSAGE(UM_ROI_CHANGED, &CDlgVisionInspectionKOZ2D::OnRoiChanged)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionKOZ2D::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CDlgVisionInspectionKOZ2D::OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_CHANGED, &CDlgVisionInspectionKOZ2D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionKOZ2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    CRect rtPara = m_procDlgInfo.m_rtParaArea;

    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, CAST_UINT(IDC_STATIC));

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_fixedInspectionSpecs);
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    // Dialog Control 위치 설정
    UpdatePropertyGrid();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspectionKOZ2D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CDlgVisionInspectionKOZ2D::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
}

void CDlgVisionInspectionKOZ2D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionKOZ2D::SetInitDialog()
{
}

void CDlgVisionInspectionKOZ2D::SetInitParaWindow()
{
}

void CDlgVisionInspectionKOZ2D::SetInitButton()
{
    CRect rtDlg;
    GetClientRect(rtDlg);

    CRect rtButton;
    m_buttonClose.GetWindowRect(rtButton);
    long nButtonPitch = rtButton.Width() + 3;

    // Close 버튼 위치 지정
    m_buttonClose.MoveWindow(
        rtDlg.right - nButtonPitch, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());

    // Inspect 버튼 위치 지정
    m_buttonInspect.MoveWindow(
        rtDlg.right - nButtonPitch * 2, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());

    // Inspect 버튼 위치 지정
    m_buttonApply.MoveWindow(
        rtDlg.right - nButtonPitch * 3, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());
}

void CDlgVisionInspectionKOZ2D::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    ShowImage(m_nImageID);
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();

    // 검사 결과 화면 갱신
    m_procCommonResultDlg->Refresh();

    // 검사 결과 탭 보여주기
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
}

void CDlgVisionInspectionKOZ2D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_procCommonSpecDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonResultDlg->ShowWindow((nTab == 1) ? SW_SHOW : SW_HIDE);
    m_procCommonDetailResultDlg->ShowWindow((nTab == 2) ? SW_SHOW : SW_HIDE);
    m_procCommonDebugInfoDlg->ShowWindow(nTab == 3 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 4 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

void CDlgVisionInspectionKOZ2D::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionKOZ2D::ShowPrevImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    if (m_nImageID <= 0)
        return;

    m_nImageID--;

    ShowImage(TRUE);
}

void CDlgVisionInspectionKOZ2D::ShowNextImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    if (m_nImageID
        >= (long)(m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size()) - 1)
        return;

    m_nImageID++;

    ShowImage(TRUE);
}

void CDlgVisionInspectionKOZ2D::ShowImage(BOOL bChange)
{
    if (m_pVisionInsp->GetImageFrameCount() == 0)
    {
        return;
    }

    if (!bChange)
    {
        m_nImageID = max(0, m_pVisionInsp->GetImageFrameIndex(0));
    }

    if ((long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() <= 0)
        return;

    if ((long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size()
        <= m_nImageID)
        m_nImageID
            = (long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() - 1;

    m_imageLotView->ShowImage(m_nImageID);
}

void CDlgVisionInspectionKOZ2D::OnCbnSelchangeComboCurrpane()
{
    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    long nSel = m_ComboCurrPane.GetCurSel();
    m_pVisionInsp->SetCurrentPaneID(nSel);

    if (m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetPreviousVisionProcessing(m_pVisionInsp), false,
            m_pVisionInsp->GetCurVisionModule_Status()))
    {
        ShowImage();
    }

    m_pVisionInsp->m_visionUnit.GetInspectionOverlayResult().Apply(
        m_pVisionInsp->m_pVisionInspDlg->m_imageLotView->GetCoreView());

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    SetROI();
}

void CDlgVisionInspectionKOZ2D::OnBnClickedButtonPreimage()
{
    ShowPrevImage();
}

void CDlgVisionInspectionKOZ2D::OnBnClickedButtonNextimage()
{
    ShowNextImage();
}

LRESULT CDlgVisionInspectionKOZ2D::OnRoiChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    GetROI();
    ShowCharBinImage();

    return 0;
}

BOOL CDlgVisionInspectionKOZ2D::ShowCharBinImage()
{
    return TRUE;
}

LRESULT CDlgVisionInspectionKOZ2D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
{
    const long itemIndex = long(wparam);

    if (itemIndex < 0)
    {
        m_TabResult.SetCurSel(1);
    }
    else
    {
        m_TabResult.SetCurSel(2);

        m_procCommonDetailResultDlg->Refresh(itemIndex);
    }

    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0L;
}

void CDlgVisionInspectionKOZ2D::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(g_szKOZInspParamName[KOZ_NOTCH_ALIGN_TITLE])) //Notch Align Param
    {
        m_pVisionPara->m_NotchAlignFrameIndex.makePropertyGridItem(
            category, g_szKOZInspParamName[KOZ_NOTCH_ALIGN_FRAME_NUM], KOZ_NOTCH_ALIGN_FRAME_NUM);
        category->AddChildItem(new CCustomItemButton(g_szKOZInspParamName[KOZ_NOTCH_ALIGN_IMAGECOMBINE], TRUE, FALSE))
            ->SetID(KOZ_NOTCH_ALIGN_IMAGECOMBINE);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_NOTCH_ALIGN_EDGE_SEARCH_LENGTH], m_pVisionPara->m_nNotchEdgeSearchLengthRatio))
            ->SetID(KOZ_NOTCH_ALIGN_EDGE_SEARCH_LENGTH);
        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(g_szKOZInspParamName[KOZ_ALIGN_TITLE])) //KOZ Align Param
    {
        m_pVisionPara->m_KOZAlignFrameIndex.makePropertyGridItem(
            category, g_szKOZInspParamName[KOZ_ALIGN_FRAME_NUM], KOZ_ALIGN_FRAME_NUM);
        category->AddChildItem(new CCustomItemButton(g_szKOZInspParamName[KOZ_ALIGN_IMAGECOMBINE], TRUE, FALSE))
            ->SetID(KOZ_ALIGN_IMAGECOMBINE);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_ALIGN_EDGE_SEARCH_LENGTH], m_pVisionPara->m_nEdgeSearchLength_um))
            ->SetID(KOZ_ALIGN_EDGE_SEARCH_LENGTH);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_ALIGN_EDGE_SEARCH_WIDTH], m_pVisionPara->m_nEdgeSearchWidthRatio))
            ->SetID(KOZ_ALIGN_EDGE_SEARCH_WIDTH);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_ALIGN_EDGE_SEARCH_GAP], m_pVisionPara->m_nEdgeDetectGap_um))
            ->SetID(KOZ_ALIGN_EDGE_SEARCH_GAP);
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                g_szKOZInspParamName[KOZ_ALIGN_EDGE_SEARCH_DIRECTION], m_pVisionPara->m_nSearchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), SEARCH_DIRECTION_IN_OUT);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), SEARCH_DIRECTION_OUT_IN);

            item->SetID(KOZ_ALIGN_EDGE_SEARCH_DIRECTION);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                g_szKOZInspParamName[KOZ_ALIGN_EDGE_DIRECTION], m_pVisionPara->m_nEdgeDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("Both"), EDGE_DIRECTION_BOTH);
            item->GetConstraints()->AddConstraint(_T("Rising"), EDGE_DIRECTION_RISING);
            item->GetConstraints()->AddConstraint(_T("Falling"), EDGE_DIRECTION_FALLING);
            item->SetID(KOZ_ALIGN_EDGE_DIRECTION);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                g_szKOZInspParamName[KOZ_ALIGN_EDGE_TYPE], m_pVisionPara->m_nEdgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EDGE_DETECT_MODE_BEST);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EDGE_DETECT_MODE_FIRST);
            item->SetID(KOZ_ALIGN_EDGE_TYPE);
        }
        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                g_szKOZInspParamName[KOZ_ALIGN_EDGE_THRESHOLD_VALUE], m_pVisionPara->m_dEdgeThreshold, _T("%.1lf")))
            ->SetID(KOZ_ALIGN_EDGE_THRESHOLD_VALUE);
        //if (auto *item = category->AddChildItem(new CXTPPropertyGridItemEnum(g_szKOZInspParamName[KOZ_ALIGN_EDGE_THRESHOLD_LEVEL], m_pVisionPara->m_nEdgeThresholdLevel)))
        //{
        //	item->GetConstraints()->AddConstraint(_T("Low"), EDGE_THRESHOLD_LEVEL_LOW);
        //	item->GetConstraints()->AddConstraint(_T("Middle"), EDGE_THRESHOLD_LEVEL_MIDDLE);
        //	item->GetConstraints()->AddConstraint(_T("High"), EDGE_THRESHOLD_LEVEL_HIGH);
        //	item->GetConstraints()->AddConstraint(_T("Highest"), EDGE_THRESHOLD_LEVEL_HIGHEST);
        //	item->SetID(KOZ_ALIGN_EDGE_THRESHOLD_LEVEL);
        //}
        //category->Expand();
    }

    if (auto* category
        = m_propertyGrid->AddCategory(g_szKOZInspParamName[KOZ_CHIPPING_TITEL])) //Chipping Inspection Param
    {
        m_pVisionPara->m_ChippingInspFrameIndex.makePropertyGridItem(
            category, g_szKOZInspParamName[KOZ_CHIPPING_FRAME_NUM], KOZ_CHIPPING_FRAME_NUM);
        category->AddChildItem(new CCustomItemButton(g_szKOZInspParamName[KOZ_CHIPPING_IMAGECOMBINE], TRUE, FALSE))
            ->SetID(KOZ_CHIPPING_IMAGECOMBINE);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_CHIPPING_IGNORE_OFFSET], m_pVisionPara->m_nChippingIgnoreOffset_um))
            ->SetID(KOZ_CHIPPING_IGNORE_OFFSET);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(g_szKOZInspParamName[KOZ_CHIPPING_EXPANSION_OFFSET],
                m_pVisionPara->m_nChippingInspectionExpandOffset_um))
            ->SetID(KOZ_CHIPPING_EXPANSION_OFFSET);
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                g_szKOZInspParamName[KOZ_CHIPPING_USE_IGNORE_OBJECT], m_pVisionPara->m_nUseIgnorePreprocessObj)))
        {
            item->GetConstraints()->AddConstraint(_T("Not Use"), OPT_NOT_USE);
            item->GetConstraints()->AddConstraint(_T("Use"), OPT_USE);
            item->SetID(KOZ_CHIPPING_USE_IGNORE_OBJECT);
        }
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_CHIPPING_IGNORE_DILATION_CNT], m_pVisionPara->m_nIgnoreDilationCount))
            ->SetID(KOZ_CHIPPING_IGNORE_DILATION_CNT);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_CHIPPING_MERGE_DISTANCE], m_pVisionPara->m_nChippingMergeDistance_um))
            ->SetID(KOZ_CHIPPING_MERGE_DISTANCE);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_CHIPPING_WINDOW_SIZE_X], m_pVisionPara->m_nWindowSizeX_um))
            ->SetID(KOZ_CHIPPING_WINDOW_SIZE_X);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_CHIPPING_WINDOW_SIZE_Y], m_pVisionPara->m_nWindowSizeY_um))
            ->SetID(KOZ_CHIPPING_WINDOW_SIZE_Y);
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                g_szKOZInspParamName[KOZ_CHIPPING_DEFECT_COLOR], m_pVisionPara->m_nDefectColor)))
        {
            item->GetConstraints()->AddConstraint(_T("Dark"), DEFECT_DARK);
            item->GetConstraints()->AddConstraint(_T("Bright"), DEFECT_BRIGHT);
            item->GetConstraints()->AddConstraint(_T("All"), DEFECT_ALL);
            item->SetID(KOZ_CHIPPING_DEFECT_COLOR);
        }
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemNumber(g_szKOZInspParamName[KOZ_CHIPPING_MIN_DARK_CONTRAST],
                    m_pVisionPara->m_nMinDarkContrast, &m_pVisionPara->m_nMinDarkContrast)))
        {
            if (auto* slider = Item->AddSliderControl())
            {
                slider->SetMin(0);
                slider->SetMax(256);
            }
            if (auto* spin = Item->AddSpinButton())
            {
                spin->SetMin(0);
                spin->SetMax(256);
            }

            Item->SetID(KOZ_CHIPPING_MIN_DARK_CONTRAST);
        }
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemNumber(g_szKOZInspParamName[KOZ_CHIPPING_MIN_BRIGHT_CONTRAST],
                    m_pVisionPara->m_nMinBrightContrast, &m_pVisionPara->m_nMinBrightContrast)))
        {
            if (auto* slider = Item->AddSliderControl())
            {
                slider->SetMin(0);
                slider->SetMax(256);
            }
            if (auto* spin = Item->AddSpinButton())
            {
                spin->SetMin(0);
                spin->SetMax(256);
            }

            Item->SetID(KOZ_CHIPPING_MIN_BRIGHT_CONTRAST);
        }
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_CHIPPING_MIN_BLOB_AREA], m_pVisionPara->m_nMinimumBlobArea_um2))
            ->SetID(KOZ_CHIPPING_MIN_BLOB_AREA);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_CHIPPING_MIN_BLOB_WIDTH], m_pVisionPara->m_nMinimumBlobWidth_um))
            ->SetID(KOZ_CHIPPING_MIN_BLOB_WIDTH);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szKOZInspParamName[KOZ_CHIPPING_MIN_BLOB_LENGTH], m_pVisionPara->m_nMinimumBlobLenght_um))
            ->SetID(KOZ_CHIPPING_MIN_BLOB_LENGTH);

        category->Expand();
    }

    if (m_pVisionPara->m_nEdgeDetectMode != EDGE_DETECT_MODE_FIRST)
        m_propertyGrid->GetItem(KOZ_ALIGN_EDGE_THRESHOLD_VALUE + 1)->SetReadOnly(TRUE);
    else
        m_propertyGrid->GetItem(KOZ_ALIGN_EDGE_THRESHOLD_VALUE + 1)->SetReadOnly(FALSE);

    SetROI();

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionInspectionKOZ2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CString strTemp;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        long nSearchLowLimit_um = 500;
        const long nSearchHighLimit_um = 2000;

        switch (item->GetID())
        {
            ////////////// Notch Align////////////////////////
            case KOZ_NOTCH_ALIGN_FRAME_NUM:
            {
                long nNotchAlignFrameIdx = m_pVisionPara->m_NotchAlignFrameIndex.getFrameIndex();
                m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                m_nImageID = nNotchAlignFrameIdx;
                ShowImage(nNotchAlignFrameIdx);
                break;
            }
            case KOZ_NOTCH_ALIGN_IMAGECOMBINE:
                ClickedButtonNotchImageCombine();
                m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                break;
            case KOZ_NOTCH_ALIGN_EDGE_SEARCH_LENGTH:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nNotchEdgeSearchLengthRatio = min(200, max(110, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nNotchEdgeSearchLengthRatio);
                    m_propertyGrid->GetItem(KOZ_NOTCH_ALIGN_EDGE_SEARCH_LENGTH)->SetValue(strTemp);
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                    DrawNotchEdgeSearchLine();
                }
                break;
            ////////////// KOZ Align ////////////////////////
            case KOZ_ALIGN_FRAME_NUM:
            {
                long nKOZAlignFrameIdx = m_pVisionPara->m_KOZAlignFrameIndex.getFrameIndex();
                m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                m_nImageID = nKOZAlignFrameIdx;
                ShowImage(nKOZAlignFrameIdx);
                break;
            }
            case KOZ_ALIGN_IMAGECOMBINE:
                ClickedButtonImageCombine();
                m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                break;
            case KOZ_ALIGN_EDGE_SEARCH_LENGTH:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nEdgeSearchLength_um
                        = min(nSearchHighLimit_um, max(nSearchLowLimit_um, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nEdgeSearchLength_um);
                    m_propertyGrid->GetItem(KOZ_ALIGN_EDGE_SEARCH_LENGTH)->SetValue(strTemp);
                    DrawEdgeSearchROI();
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                }
                break;
            case KOZ_ALIGN_EDGE_SEARCH_WIDTH:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nEdgeSearchWidthRatio = min(100, max(10, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nEdgeSearchWidthRatio);
                    m_propertyGrid->GetItem(KOZ_ALIGN_EDGE_SEARCH_WIDTH)->SetValue(strTemp);
                    DrawEdgeSearchROI();
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                }
                break;
            case KOZ_ALIGN_EDGE_SEARCH_GAP:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nEdgeDetectGap_um = min(2000, max(20, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nEdgeDetectGap_um);
                    m_propertyGrid->GetItem(KOZ_ALIGN_EDGE_SEARCH_GAP)->SetValue(strTemp);
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                }
                break;
            case KOZ_ALIGN_EDGE_SEARCH_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                {
                    m_pVisionPara->m_nSearchDirection = value->GetEnum();
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                }
                break;
            case KOZ_ALIGN_EDGE_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                {
                    m_pVisionPara->m_nEdgeDirection = value->GetEnum();
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                }
                break;
            case KOZ_ALIGN_EDGE_TYPE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                {
                    m_pVisionPara->m_nEdgeDetectMode = value->GetEnum();
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                }
                break;
            case KOZ_ALIGN_EDGE_THRESHOLD_VALUE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    m_pVisionPara->m_dEdgeThreshold = value->GetDouble();
                    m_pVisionPara->m_dEdgeThreshold = max(1., m_pVisionPara->m_dEdgeThreshold);
                    strTemp.Format(_T("%.1lf"), m_pVisionPara->m_dEdgeThreshold);
                    m_propertyGrid->GetItem(KOZ_ALIGN_EDGE_THRESHOLD_VALUE)->SetValue(strTemp);

                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                }
                break;
            ////////////// Chipping Inspection ////////////////////////
            case KOZ_CHIPPING_FRAME_NUM:
            {
                long nChippingInspFrameIdx = m_pVisionPara->m_ChippingInspFrameIndex.getFrameIndex();
                m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                m_nImageID = nChippingInspFrameIdx;
                ShowImage(nChippingInspFrameIdx);
                break;
            }
            case KOZ_CHIPPING_IMAGECOMBINE:
                ClickedButtonChippingImageCombine();
                m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                break;
            case KOZ_CHIPPING_IGNORE_OFFSET:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nChippingIgnoreOffset_um = (long)min(2000, max(0, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nChippingIgnoreOffset_um);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_IGNORE_OFFSET)->SetValue(strTemp);
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    ShowChippingMaskImage();
                }
                break;
            case KOZ_CHIPPING_EXPANSION_OFFSET:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nChippingInspectionExpandOffset_um = (long)min(2000, max(0, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nChippingInspectionExpandOffset_um);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_EXPANSION_OFFSET)->SetValue(strTemp);
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    ShowChippingMaskImage();
                }
                break;
            case KOZ_CHIPPING_USE_IGNORE_OBJECT:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nUseIgnorePreprocessObj = value->GetEnum();
                m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                ShowChippingMaskImage();
                break;
            case KOZ_CHIPPING_IGNORE_DILATION_CNT:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nIgnoreDilationCount = CAST_LONG(min(100, max(-30, value->GetNumber())));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nIgnoreDilationCount);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_IGNORE_DILATION_CNT)->SetValue(strTemp);
                    m_pVisionInsp->m_bIsNeedToMakeChippingMask = true;
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                    ShowChippingMaskImage();
                }
                break;
            case KOZ_CHIPPING_MERGE_DISTANCE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nChippingMergeDistance_um = CAST_LONG(min(2000, max(0, value->GetNumber())));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nChippingMergeDistance_um);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_MERGE_DISTANCE)->SetValue(strTemp);
                }
                break;
            case KOZ_CHIPPING_WINDOW_SIZE_X:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nWindowSizeX_um = (long)max(100, value->GetNumber());
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nWindowSizeX_um);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_WINDOW_SIZE_X)->SetValue(strTemp);
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                    ShowChippingThresholdImage();
                }
                break;
            case KOZ_CHIPPING_WINDOW_SIZE_Y:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nWindowSizeY_um = (long)max(100, value->GetNumber());
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nWindowSizeY_um);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_WINDOW_SIZE_Y)->SetValue(strTemp);
                    m_pVisionInsp->m_bIsNeedToMakeChippingBackground = true;
                    ShowChippingThresholdImage();
                }
                break;
            case KOZ_CHIPPING_MIN_DARK_CONTRAST:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nMinDarkContrast = (long)min(255, max(0, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nMinDarkContrast);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_MIN_DARK_CONTRAST)->SetValue(strTemp);
                    ShowChippingThresholdImage();
                }
                break;
            case KOZ_CHIPPING_MIN_BRIGHT_CONTRAST:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nMinBrightContrast = (long)min(255, max(0, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nMinBrightContrast);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_MIN_BRIGHT_CONTRAST)->SetValue(strTemp);
                    ShowChippingThresholdImage();
                }
                break;
            case KOZ_CHIPPING_MIN_BLOB_AREA:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nMinimumBlobArea_um2 = (long)max(0, value->GetNumber());
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nMinimumBlobArea_um2);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_MIN_BLOB_AREA)->SetValue(strTemp);
                }
                break;
            case KOZ_CHIPPING_MIN_BLOB_WIDTH:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nMinimumBlobWidth_um = (long)max(0, value->GetNumber());
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nMinimumBlobWidth_um);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_MIN_BLOB_WIDTH)->SetValue(strTemp);
                }
                break;
            case KOZ_CHIPPING_MIN_BLOB_LENGTH:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nMinimumBlobLenght_um = (long)max(0, value->GetNumber());
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nMinimumBlobLenght_um);
                    m_propertyGrid->GetItem(KOZ_CHIPPING_MIN_BLOB_LENGTH)->SetValue(strTemp);
                }
                break;
            case KOZ_CHIPPING_DEFECT_COLOR:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nDefectColor = value->GetEnum();
                break;
        }
    }
    else if (wparam == XTP_PGN_SELECTION_CHANGED && item->GetID() == KOZ_CHIPPING_MIN_DARK_CONTRAST)
    {
        if (auto* control = item->GetInplaceControls()->GetAt(0))
        {
            auto* slider = dynamic_cast<CXTPPropertyGridInplaceSlider*>(control);

            if (slider)
            {
                slider->GetSliderCtrl()->SetPageSize(8);
            }
        }
        ShowChippingThresholdImage();
    }
    else if (wparam == XTP_PGN_SELECTION_CHANGED && item->GetID() == KOZ_CHIPPING_MIN_BRIGHT_CONTRAST)
    {
        if (auto* control = item->GetInplaceControls()->GetAt(0))
        {
            auto* slider = dynamic_cast<CXTPPropertyGridInplaceSlider*>(control);

            if (slider)
            {
                slider->GetSliderCtrl()->SetPageSize(8);
            }
        }
        ShowChippingThresholdImage();
    }

    return 0;
}

void CDlgVisionInspectionKOZ2D::ClickedButtonImageCombine()
{
    auto* proc = m_pVisionInsp;
    long nKOZAlignFrameIdx = m_pVisionPara->m_KOZAlignFrameIndex.getFrameIndex();
    if (::Combine_SetParameter(*proc, nKOZAlignFrameIdx, false, &proc->m_VisionPara->m_ImageProcManagePara) == IDOK)
    {
        // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
        Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage(false, nKOZAlignFrameIdx);
        if (image.GetMem() == nullptr)
            return;

        Ipvm::Image8u combineImage;
        if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
            return;

        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());
        if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_ImageProcManagePara, combineImage))
        {
            return;
        }

        CString FrameToString;

        if (m_pVisionInsp->m_VisionPara->m_ImageProcManagePara.isCombine())
        {
            FrameToString.Format(_T("Combined Image **"));
        }
        else
        {
            FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
        }

        m_imageLotView->SetImage(combineImage, FrameToString);
    }
}

void CDlgVisionInspectionKOZ2D::ClickedButtonNotchImageCombine()
{
    auto* proc = m_pVisionInsp;
    long nNotchAlignFrameIdx = proc->m_VisionPara->m_NotchAlignFrameIndex.getFrameIndex();
    if (::Combine_SetParameter(*proc, nNotchAlignFrameIdx, false, &proc->m_VisionPara->m_NotchImageProcManagePara)
        == IDOK)
    {
        // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
        Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage(false, nNotchAlignFrameIdx);
        if (image.GetMem() == nullptr)
            return;

        Ipvm::Image8u combineImage;
        if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
            return;

        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());
        if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_NotchImageProcManagePara, combineImage))
        {
            return;
        }

        CString FrameToString;

        if (m_pVisionInsp->m_VisionPara->m_NotchImageProcManagePara.isCombine())
        {
            FrameToString.Format(_T("Combined Image **"));
        }
        else
        {
            FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
        }

        m_imageLotView->SetImage(combineImage, FrameToString);
    }
}

void CDlgVisionInspectionKOZ2D::ClickedButtonChippingImageCombine()
{
    auto* proc = m_pVisionInsp;
    long nChippingInspFrameIdx = proc->m_VisionPara->m_ChippingInspFrameIndex.getFrameIndex();
    if (::Combine_SetParameter(*proc, nChippingInspFrameIdx, false, &proc->m_VisionPara->m_ChippingImageProcManagePara)
        == IDOK)
    {
        // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
        Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage(false, nChippingInspFrameIdx);
        if (image.GetMem() == nullptr)
            return;

        Ipvm::Image8u combineImage;
        if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
            return;

        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());
        if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_ChippingImageProcManagePara, combineImage))
        {
            return;
        }

        CString FrameToString;

        if (m_pVisionInsp->m_VisionPara->m_ChippingImageProcManagePara.isCombine())
        {
            FrameToString.Format(_T("Combined Image **"));
        }
        else
        {
            FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
        }

        m_imageLotView->SetImage(combineImage, FrameToString);
    }
}

void CDlgVisionInspectionKOZ2D::ShowImage(long nFrameIdx)
{
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage(false, nFrameIdx);
    if (image.GetMem() == nullptr)
        return;
    m_imageLotView->SetImage(image);
}

void CDlgVisionInspectionKOZ2D::ShowChippingMaskImage()
{
    m_imageLotView->Overlay_RemoveAll();

    long nFrameIdx = m_pVisionPara->m_ChippingInspFrameIndex.getFrameIndex();
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage(false, nFrameIdx);
    if (image.GetMem() == nullptr)
        return;

    if (m_pVisionInsp->m_bIsNeedToMakeChippingMask)
    {
        m_pVisionInsp->DoAlign(true);
        m_pVisionInsp->MakeKOZChippingAreaMask();
    }

    //{{ Display 하기
    PatternImage pattern;
    pattern.setImage(image);
    pattern.addPattern(m_pVisionInsp->m_imageChippingInspMask, RGB(0, 255, 0));

    m_imageLotView->SetImage(pattern, _T("Chipping Mask Image **"));
    m_imageLotView->Overlay_Show(SW_SHOW);
    //}}
}

void CDlgVisionInspectionKOZ2D::ShowChippingThresholdImage()
{
    m_imageLotView->Overlay_RemoveAll();
    long nFrameIdx = m_pVisionPara->m_ChippingInspFrameIndex.getFrameIndex();
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage(false, nFrameIdx);
    if (image.GetMem() == nullptr)
        return;

    Ipvm::Image8u combineImage;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
        return;

    Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());
    if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
            m_pVisionInsp->m_VisionPara->m_ChippingImageProcManagePara, combineImage))
    {
        return;
    }

    if (m_pVisionInsp->m_bIsNeedToMakeChippingMask)
    {
        m_pVisionInsp->DoAlign(true);
        m_pVisionInsp->MakeKOZChippingAreaMask();
    }

    Ipvm::Image8u darkThreshImage, brightThreshImage;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(darkThreshImage))
        return;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(brightThreshImage))
        return;

    if (m_pVisionInsp->m_bIsNeedToMakeChippingBackground)
    {
        m_pVisionInsp->MakeKOZChippingBackgroundImage(combineImage);
    }

    m_pVisionInsp->MakeKOZChippingThresholdImage(combineImage, m_pVisionInsp->m_imageChippingBackground,
        m_pVisionInsp->m_imageChippingInspMask, darkThreshImage, brightThreshImage);

    //{{ Display 하기
    PatternImage pattern;
    pattern.setImage(combineImage);
    pattern.addPattern(m_pVisionInsp->m_imageChippingInspMask, RGB(0, 255, 0));
    pattern.addPattern(darkThreshImage, RGB(0, 0, 255));
    pattern.addPattern(brightThreshImage, RGB(255, 0, 0));

    m_imageLotView->SetImage(pattern, _T("Chipping Threshold Image **"));
    m_imageLotView->Overlay_Show(SW_SHOW);
    //}}
}

void CDlgVisionInspectionKOZ2D::GetROI()
{
}

void CDlgVisionInspectionKOZ2D::SetROI()
{
}

LRESULT CDlgVisionInspectionKOZ2D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    SetROI();

    return 0L;
}

void CDlgVisionInspectionKOZ2D::DrawEdgeSearchROI()
{
    m_imageLotView->Overlay_RemoveAll();

    //Package Align 정보 가져오기
    Ipvm::Rect32r frtBody = m_pVisionInsp->GetBodyRect();
    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        return;

    PI_RECT prtSpecOfKOZ;
    std::vector<Ipvm::Rect32s> vecrtSearchROI(0);
    m_pVisionInsp->MakeEdgeSearchROI(vecrtSearchROI, prtSpecOfKOZ);

    m_imageLotView->Overlay_AddRectangle(prtSpecOfKOZ, RGB(255, 0, 255));
    m_imageLotView->Overlay_AddRectangles(vecrtSearchROI, RGB(0, 255, 0));

    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionKOZ2D::DrawNotchEdgeSearchLine()
{
    m_imageLotView->Overlay_RemoveAll();

    //Package Align 정보 가져오기
    Ipvm::Rect32r frtBody = m_pVisionInsp->GetBodyRect();
    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        return;

    std::vector<Ipvm::Point32r2> vecfptLineStart(0);
    std::vector<Ipvm::Point32r2> vecfptLineEnd(0);

    m_pVisionInsp->GetNotchEdgeSearchLine(vecfptLineStart, vecfptLineEnd);

    m_imageLotView->Overlay_AddLine(vecfptLineStart, vecfptLineEnd, RGB(0, 255, 0));

    m_imageLotView->Overlay_Show(true);
}
