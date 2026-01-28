//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionLid3D.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionLid3D.h"
#include "VisionInspectionLid3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
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
enum enumLidAlignParamID
{
    LID_ALIGN_PARAM_START = 1,
    LID_ALIGN_PARAM_IMAGECOMBINE = LID_ALIGN_PARAM_START,
    LID_ALIGN_PARAM_SEARCH_DIRECTION,
    LID_ALIGN_PARAM_EDGE_DIRECTION,
    LID_ALIGN_PARAM_EDGE_DETECT_MODE,
    LID_ALIGN_PARAM_FIRST_EDGE_VALUE,
    LID_ALIGN_PARAM_SEARCH_LENGTH,
    LID_ALIGN_PARAM_SEARCH_WIDTH,
    LID_ALIGN_PARAM_SEARCH_GAP,
    LID_ALIGN_PARAM_SUBSTRATE_PARAM,
    LID_ALIGN_PARAM_SUBSTRATE_ROI_SIZE,
    LID_ALIGN_PARAM_SUBSTRATE_OFFSET_X,
    LID_ALIGN_PARAM_SUBSTRATE_OFFSET_Y,
    LID_ALIGN_PARAM_LID_TILT_PARAM,
    LID_ALIGN_PARAM_LID_TILT_ROI_SIZE,
    LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_X,
    LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_Y,
    LID_ALIGN_PARAM_LID_ROI_PARAM,
    LID_ALIGN_PARAM_LID_IMPORT_WARPandHEIGHT_MAP,
    LID_ALIGN_PARAM_LID_EXPORT_WARPandHEIGHT_MAP,
    LID_ALIGN_PARAM_LID_SHOW_CUR_WARPandHEIGHT_MAP,
    LID_ALIGN_PARAM_END,
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

static LPCTSTR g_szEDGEAlignParamName[] = {
    _T("Edge detect parameter"),
    _T("Search direction"),
    _T("Edge direction"),
    _T("Edge detect mode"),
    _T("First edge value                     [ 1 ~ 30 ]"),
    _T("Edge search length (um)         [ 500 ~ 2000 ]"),
    _T("Edge search width ratio (%)     [ 10 ~ 100 ]"),
    _T("Detected edge gap (um)         [ 20 ~ 2000 ]"),
    _T("Substrate ROI parameters"),
    _T("Substrate ROI size (um)"),
    _T("Substrate ROI offset X (um)"),
    _T("Substrate ROI offset Y (um)"),
    _T("Lid tilt & height ROI parameters"),
    _T("Lid tilt & height ROI size (um)"),
    _T("Lid tilt & height ROI offset X (um)"),
    _T("Lid tilt & height ROI offset Y (um)"),
    _T("Lid warpage ROI parameters"),
    _T("Import lid ROI map File"),
    _T("Export lid ROI map File"),
    _T("Show current lid ROI map"),
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionLid3D, CDialog)

CDlgVisionInspectionLid3D::CDlgVisionInspectionLid3D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionLid3D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionLid3D::IDD, pParent)
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

CDlgVisionInspectionLid3D::~CDlgVisionInspectionLid3D()
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

void CDlgVisionInspectionLid3D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_APPLY, m_buttonApply);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
    DDX_Control(pDX, IDC_COMBO_CURRPANE, m_ComboCurrPane);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionLid3D, CDialog)
//ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionLid3D::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionLid3D::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspectionLid3D::OnBnClickedButtonClose)
ON_CBN_SELCHANGE(IDC_COMBO_CURRPANE, &CDlgVisionInspectionLid3D::OnCbnSelchangeComboCurrpane)
ON_BN_CLICKED(IDC_BUTTON_PREIMAGE, &CDlgVisionInspectionLid3D::OnBnClickedButtonPreimage)
ON_BN_CLICKED(IDC_BUTTON_NEXTIMAGE, &CDlgVisionInspectionLid3D::OnBnClickedButtonNextimage)
ON_MESSAGE(UM_ROI_CHANGED, &CDlgVisionInspectionLid3D::OnRoiChanged)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionLid3D::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CDlgVisionInspectionLid3D::OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_CHANGED, &CDlgVisionInspectionLid3D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionLid3D::OnInitDialog()
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

int CDlgVisionInspectionLid3D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CDlgVisionInspectionLid3D::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
}

void CDlgVisionInspectionLid3D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionLid3D::SetInitDialog()
{
}

void CDlgVisionInspectionLid3D::SetInitParaWindow()
{
}

void CDlgVisionInspectionLid3D::SetInitButton()
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

void CDlgVisionInspectionLid3D::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
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

void CDlgVisionInspectionLid3D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDlgVisionInspectionLid3D::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionLid3D::ShowPrevImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    if (m_nImageID <= 0)
        return;

    m_nImageID--;

    ShowImage(TRUE);
}

void CDlgVisionInspectionLid3D::ShowNextImage()
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

void CDlgVisionInspectionLid3D::ShowImage(BOOL bChange)
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

void CDlgVisionInspectionLid3D::OnCbnSelchangeComboCurrpane()
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

void CDlgVisionInspectionLid3D::OnBnClickedButtonPreimage()
{
    ShowPrevImage();
}

void CDlgVisionInspectionLid3D::OnBnClickedButtonNextimage()
{
    ShowNextImage();
}

LRESULT CDlgVisionInspectionLid3D::OnRoiChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    GetROI();
    ShowCharBinImage();

    return 0;
}

BOOL CDlgVisionInspectionLid3D::ShowCharBinImage()
{
    return TRUE;
}

LRESULT CDlgVisionInspectionLid3D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

void CDlgVisionInspectionLid3D::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Lid align paramters")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(LID_ALIGN_PARAM_IMAGECOMBINE);

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(g_szEDGEAlignParamName[0]))
    {
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szEDGEAlignParamName[LID_ALIGN_PARAM_SEARCH_DIRECTION - 1],
                    m_pVisionPara->m_nSearchDirection))) //, (int *)m_pVisionPara->m_nSearchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), SEARCH_DIRECTION_IN_OUT);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), SEARCH_DIRECTION_OUT_IN);

            item->SetID(LID_ALIGN_PARAM_SEARCH_DIRECTION);
        }
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szEDGEAlignParamName[LID_ALIGN_PARAM_EDGE_DIRECTION - 1],
                    m_pVisionPara->m_nEdgeDirection))) //, (int *)m_pVisionPara->m_nEdgeDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("Both"), EDGE_DIRECTION_BOTH);
            item->GetConstraints()->AddConstraint(_T("Rising"), EDGE_DIRECTION_RISING);
            item->GetConstraints()->AddConstraint(_T("Falling"), EDGE_DIRECTION_FALLING);

            item->SetID(LID_ALIGN_PARAM_EDGE_DIRECTION);
        }
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szEDGEAlignParamName[LID_ALIGN_PARAM_EDGE_DETECT_MODE - 1],
                    m_pVisionPara->m_nEdgeDetectMode))) //, (int *)m_pVisionPara->m_nEdgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EDGE_DETECT_MODE_BEST);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EDGE_DETECT_MODE_FIRST);

            item->SetID(LID_ALIGN_PARAM_EDGE_DETECT_MODE);
        }

        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_FIRST_EDGE_VALUE - 1], m_pVisionPara->m_nFirstEdgeValue))
            ->SetID(LID_ALIGN_PARAM_FIRST_EDGE_VALUE);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_SEARCH_LENGTH - 1], m_pVisionPara->m_nEdgeSearchLength_um))
            ->SetID(LID_ALIGN_PARAM_SEARCH_LENGTH);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_SEARCH_WIDTH - 1], m_pVisionPara->m_nEdgeSearchWidthRatio))
            ->SetID(LID_ALIGN_PARAM_SEARCH_WIDTH);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_SEARCH_GAP - 1], m_pVisionPara->m_nEdgeDetectGap_um))
            ->SetID(LID_ALIGN_PARAM_SEARCH_GAP);

        category->Expand();
    }
    if (auto* category = m_propertyGrid->AddCategory(g_szEDGEAlignParamName[LID_ALIGN_PARAM_SUBSTRATE_PARAM - 1]))
    {
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_SUBSTRATE_ROI_SIZE - 1], m_pVisionPara->m_nSubstrateROI_size_um))
            ->SetID(LID_ALIGN_PARAM_SUBSTRATE_ROI_SIZE);
        category
            ->AddChildItem(
                new CXTPPropertyGridItemNumber(g_szEDGEAlignParamName[LID_ALIGN_PARAM_SUBSTRATE_OFFSET_X - 1],
                    m_pVisionPara->m_nSubstrateROI_OffsetX_um))
            ->SetID(LID_ALIGN_PARAM_SUBSTRATE_OFFSET_X);
        category
            ->AddChildItem(
                new CXTPPropertyGridItemNumber(g_szEDGEAlignParamName[LID_ALIGN_PARAM_SUBSTRATE_OFFSET_Y - 1],
                    m_pVisionPara->m_nSubstrateROI_OffsetY_um))
            ->SetID(LID_ALIGN_PARAM_SUBSTRATE_OFFSET_Y);
        category->Expand();
    }
    if (auto* category = m_propertyGrid->AddCategory(g_szEDGEAlignParamName[LID_ALIGN_PARAM_LID_TILT_PARAM - 1]))
    {
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_LID_TILT_ROI_SIZE - 1], m_pVisionPara->m_nLidTiltROI_size_um))
            ->SetID(LID_ALIGN_PARAM_LID_TILT_ROI_SIZE);
        category
            ->AddChildItem(
                new CXTPPropertyGridItemNumber(g_szEDGEAlignParamName[LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_X - 1],
                    m_pVisionPara->m_nLidTiltROI_OffsetX_um))
            ->SetID(LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_X);
        category
            ->AddChildItem(
                new CXTPPropertyGridItemNumber(g_szEDGEAlignParamName[LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_Y - 1],
                    m_pVisionPara->m_nLidTiltROI_OffsetY_um))
            ->SetID(LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_Y);
        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(g_szEDGEAlignParamName[LID_ALIGN_PARAM_LID_ROI_PARAM - 1]))
    {
        category
            ->AddChildItem(new CCustomItemButton(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_LID_IMPORT_WARPandHEIGHT_MAP - 1], TRUE, FALSE))
            ->SetID(LID_ALIGN_PARAM_LID_IMPORT_WARPandHEIGHT_MAP);
        category
            ->AddChildItem(new CCustomItemButton(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_LID_EXPORT_WARPandHEIGHT_MAP - 1], TRUE, FALSE))
            ->SetID(LID_ALIGN_PARAM_LID_EXPORT_WARPandHEIGHT_MAP);
        category
            ->AddChildItem(new CCustomItemButton(
                g_szEDGEAlignParamName[LID_ALIGN_PARAM_LID_SHOW_CUR_WARPandHEIGHT_MAP - 1], TRUE, FALSE))
            ->SetID(LID_ALIGN_PARAM_LID_SHOW_CUR_WARPandHEIGHT_MAP);

        category->Expand();
    }

    if (m_pVisionPara->m_nEdgeDetectMode != EDGE_DETECT_MODE_FIRST)
        m_propertyGrid->GetItem(LID_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(TRUE);
    else
        m_propertyGrid->GetItem(LID_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(FALSE);

    SetROI();

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionInspectionLid3D::OnGridNotify(WPARAM wparam, LPARAM lparam)
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
            case LID_ALIGN_PARAM_IMAGECOMBINE:
                ClickedButtonImageCombine();
                break;
            case LID_ALIGN_PARAM_SEARCH_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nSearchDirection = value->GetEnum();
                break;
            case LID_ALIGN_PARAM_EDGE_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nEdgeDirection = value->GetEnum();
                break;
            case LID_ALIGN_PARAM_EDGE_DETECT_MODE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nEdgeDetectMode = value->GetEnum();

                if (m_pVisionPara->m_nEdgeDetectMode != EDGE_DETECT_MODE_FIRST)
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(TRUE);
                else
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(FALSE);

                break;
            case LID_ALIGN_PARAM_FIRST_EDGE_VALUE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                    if (value->GetNumber() < 1 || value->GetNumber() > 30)
                    {
                        m_pVisionPara->m_nFirstEdgeValue = 1;
                        strTemp.Format(_T("%d"), m_pVisionPara->m_nFirstEdgeValue);
                        m_propertyGrid->GetItem(LID_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetValue(strTemp);
                    }
                    else
                        m_pVisionPara->m_nFirstEdgeValue = value->GetNumber();
                break;
            case LID_ALIGN_PARAM_SEARCH_LENGTH:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nEdgeSearchLength_um
                        = min(nSearchHighLimit_um, max(nSearchLowLimit_um, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nEdgeSearchLength_um);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_SEARCH_LENGTH + 1)->SetValue(strTemp);
                    DrawEdgeSearchROI();
                }
                break;
            case LID_ALIGN_PARAM_SEARCH_WIDTH:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nEdgeSearchWidthRatio = min(100, max(10, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nEdgeSearchWidthRatio);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_SEARCH_WIDTH + 1)->SetValue(strTemp);
                    DrawEdgeSearchROI();
                }
                break;
            case LID_ALIGN_PARAM_SEARCH_GAP:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nEdgeDetectGap_um = min(2000, max(20, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nEdgeDetectGap_um);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_SEARCH_GAP + 1)->SetValue(strTemp);
                }
                break;
            case LID_ALIGN_PARAM_SUBSTRATE_ROI_SIZE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nSubstrateROI_size_um = value->GetNumber();
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nSubstrateROI_size_um);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_SUBSTRATE_ROI_SIZE + 1)->SetValue(strTemp);
                    DrawSubstrateROI();
                }
                break;
            case LID_ALIGN_PARAM_SUBSTRATE_OFFSET_X:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nSubstrateROI_OffsetX_um = value->GetNumber();
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nSubstrateROI_OffsetX_um);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_SUBSTRATE_OFFSET_X + 1)->SetValue(strTemp);
                    DrawSubstrateROI();
                }
                break;
            case LID_ALIGN_PARAM_SUBSTRATE_OFFSET_Y:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nSubstrateROI_OffsetY_um = value->GetNumber();
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nSubstrateROI_OffsetY_um);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_SUBSTRATE_OFFSET_Y + 1)->SetValue(strTemp);
                    DrawSubstrateROI();
                }
                break;
            case LID_ALIGN_PARAM_LID_TILT_ROI_SIZE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nLidTiltROI_size_um = value->GetNumber();
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nLidTiltROI_size_um);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_LID_TILT_ROI_SIZE + 1)->SetValue(strTemp);
                    DrawLidTiltROI();
                }
                break;
            case LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_X:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nLidTiltROI_OffsetX_um = value->GetNumber();
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nLidTiltROI_OffsetX_um);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_X + 1)->SetValue(strTemp);
                    DrawLidTiltROI();
                }
                break;
            case LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_Y:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nLidTiltROI_OffsetY_um = value->GetNumber();
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nLidTiltROI_OffsetY_um);
                    m_propertyGrid->GetItem(LID_ALIGN_PARAM_LID_TILT_ROI_OFFSET_Y + 1)->SetValue(strTemp);
                    DrawLidTiltROI();
                }
                break;
            case LID_ALIGN_PARAM_LID_IMPORT_WARPandHEIGHT_MAP:
                ClickedButtonImportLidHeightROI_MapData();
                DrawLidHeightROI();
                break;
            case LID_ALIGN_PARAM_LID_EXPORT_WARPandHEIGHT_MAP:
                ClickedButtonExportLidHeightROI_MapData();
                break;
            case LID_ALIGN_PARAM_LID_SHOW_CUR_WARPandHEIGHT_MAP:
                DrawLidHeightROI();
                break;
        }
    }

    strTemp.Empty();

    return 0;
}

void CDlgVisionInspectionLid3D::ClickedButtonImageCombine()
{
    auto* proc = m_pVisionInsp;

    if (::Combine_SetParameter(*proc, proc->GetImageFrameIndex(0), false, &proc->m_VisionPara->m_ImageProcManagePara)
        == IDOK)
    {
        // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
        Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
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

        FrameToString.Empty();
    }
}

void CDlgVisionInspectionLid3D::ClickedButtonImportLidHeightROI_MapData()
{
    CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Comma separated values file format (*.csv)|*.csv||"));

    if (dlg.DoModal() != IDOK)
        return;

    m_pVisionPara->ReadLid3DGroupInfoFromCSV(dlg.GetPathName());
}

void CDlgVisionInspectionLid3D::ClickedButtonExportLidHeightROI_MapData()
{
    CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Comma separated values file format (*.csv)|*.csv||"));

    if (dlg.DoModal() != IDOK)
        return;

    m_pVisionPara->WriteLid3DGroupInfoToCSV(dlg.GetPathName());
}

void CDlgVisionInspectionLid3D::GetROI()
{
}

void CDlgVisionInspectionLid3D::SetROI()
{
}

LRESULT CDlgVisionInspectionLid3D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    SetROI();

    return 0L;
}

void CDlgVisionInspectionLid3D::DrawEdgeSearchROI()
{
    m_imageLotView->Overlay_RemoveAll();

    //Package Align 정보 가져오기
    Ipvm::Rect32r frtBody = m_pVisionInsp->GetBodyRect();
    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        return;

    //Lid 정보 가져오기
    if (m_pVisionInsp->GetLidInfoDB() == FALSE)
        return;

    long nLidNum = (long)m_pVisionInsp->m_vecsLidInfoDB.size();

    std::vector<Ipvm::Rect32s> vecrtSearchROI(0);
    std::vector<Ipvm::Rect32s> vecrtEdgeSearchROI(4);

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        m_pVisionInsp->MakeEdgeSearchROI(m_pVisionInsp->m_vecsLidInfoDB[nLid], vecrtEdgeSearchROI);

        vecrtSearchROI.insert(vecrtSearchROI.end(), vecrtEdgeSearchROI.begin(), vecrtEdgeSearchROI.end());
    }

    m_imageLotView->Overlay_AddRectangles(vecrtSearchROI, RGB(0, 255, 0));

    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionLid3D::DrawSubstrateROI()
{
    m_imageLotView->Overlay_RemoveAll();

    //Package Align 정보 가져오기
    Ipvm::Rect32r frtBody = m_pVisionInsp->GetBodyRect();
    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        return;

    //Lid 정보 가져오기
    if (m_pVisionInsp->GetLidInfoDB() == FALSE)
        return;

    auto alignResult = *m_pVisionInsp->m_sEdgeAlignResult;
    if (alignResult.bAvailable == FALSE)
        return;

    std::vector<Ipvm::Rect32s> vecrtSubstrateROI(0);

    const float fSubstrateROI_OffsetX_um((float)m_pVisionPara->m_nSubstrateROI_OffsetX_um);
    const float fSubstrateROI_OffsetY_um((float)m_pVisionPara->m_nSubstrateROI_OffsetY_um);
    const float fSubstrateROI_size_um((float)m_pVisionPara->m_nSubstrateROI_size_um);

    m_pVisionInsp->MakeBasicROI(
        alignResult, fSubstrateROI_OffsetX_um, fSubstrateROI_OffsetY_um, fSubstrateROI_size_um, vecrtSubstrateROI);

    m_imageLotView->Overlay_AddRectangles(vecrtSubstrateROI, RGB(0, 255, 0));

    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionLid3D::DrawLidTiltROI()
{
    m_imageLotView->Overlay_RemoveAll();

    m_pVisionInsp->DoAlignLid(TRUE);

    long nLidNum = (long)m_pVisionInsp->m_vecsLidAlignResult.size();
    if (nLidNum <= 0)
        return;

    const float fLidTiltROI_OffsetX_um((float)m_pVisionPara->m_nLidTiltROI_OffsetX_um);
    const float fLidTiltROI_OffsetY_um((float)m_pVisionPara->m_nLidTiltROI_OffsetY_um);
    const float fLidTiltROI_size_um((float)m_pVisionPara->m_nLidTiltROI_size_um);

    std::vector<Ipvm::Rect32s> vecrtLidTiltROI(0);
    std::vector<Ipvm::Rect32s> vecrtLidTiltTotalROI(0);

    for (auto alignResult : m_pVisionInsp->m_vecsLidAlignResult)
    {
        if (alignResult.bAvailable == FALSE)
            continue;

        m_pVisionInsp->MakeBasicROI(
            alignResult, fLidTiltROI_OffsetX_um, fLidTiltROI_OffsetY_um, fLidTiltROI_size_um, vecrtLidTiltROI);
        vecrtLidTiltTotalROI.insert(vecrtLidTiltTotalROI.end(), vecrtLidTiltROI.begin(), vecrtLidTiltROI.end());
    }

    m_imageLotView->Overlay_AddRectangles(vecrtLidTiltTotalROI, RGB(0, 255, 0));

    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionLid3D::DrawLidHeightROI()
{
    m_imageLotView->Overlay_RemoveAll();

    m_pVisionInsp->DoAlignLid(TRUE);

    long nLidNum = (long)m_pVisionInsp->m_vecsLidAlignResult.size();
    long nLidGroupInfoNum = (long)m_pVisionPara->m_vecsLid3DGroupInfo_um.size();
    if (nLidNum <= 0 || nLidGroupInfoNum <= 0)
        return;

    std::vector<Ipvm::Rect32s> vec2rtLidROI(0);
    std::vector<Ipvm::Rect32s> vecrtLidTotalROI(0);

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        if (m_pVisionInsp->m_vecsLidAlignResult[nLid].bAvailable == FALSE)
            continue;

        m_pVisionInsp->MakeLidROI(nLid, vec2rtLidROI);
        vecrtLidTotalROI.insert(vecrtLidTotalROI.end(), vec2rtLidROI.begin(), vec2rtLidROI.end());
    }

    if (vecrtLidTotalROI.size() <= 0)
        return;
    m_imageLotView->Overlay_AddRectangles(vecrtLidTotalROI, RGB(0, 255, 0));

    m_imageLotView->Overlay_Show(true);
}
