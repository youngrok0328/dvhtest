//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingSideDetailAlign.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingSideDetailAlign.h"
#include "VisionProcessingSideDetailAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
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
#define UM_IMAGE_LOT_VIEW_SELCHANGED_RAW (WM_USER + 1013)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE (WM_USER + 1014)

//CPP_7_________________________________ Implementation body
//
enum enumSetupInfoTab
{
    TAB_DEBUG_INFO = 0,
    TAB_TXT_LOG,
};

enum enumSideDetailAlignParamID
{
    SIDE_DETAIL_PARAM_START = 0,
    SIDE_DETAIL_COMMON_TITLE = SIDE_DETAIL_PARAM_START,
    SIDE_DETAIL_COMMON_PARAM_EDGE_COUNT,
    SIDE_DETAIL_COMMON_PARAM_EDGE_POINT_NOISE_FILTER,

    SIDE_DETAIL_SUBSTRATE_TITLE,
    SIDE_DETAIL_SUBSTRATE_PARAM_FRAME_ID,
    SIDE_DETAIL_SUBSTRATE_PARAM_IMAGE_COMBINE,
    SIDE_DETAIL_SUBSTRATE_PARAM_SEARCH_DIRECTION,
    SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DIRECTION,
    SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DETECT_MODE,
    SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_THRESHOLD,
    SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_WIDTH_TB,
    SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_LENGTH_TB,
    SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_WIDTH_LR,
    SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_LENGTH_LR,
    SIDE_DETAIL_SUBSTRATE_PARAM_INNER_ALIGN_PARAM,

    SIDE_DETAIL_GLASS_TITLE_COMMON,
    SIDE_DETAIL_GLASS_PARAM_EDGE_DETECT_MODE,
    SIDE_DETAIL_GLASS_PARAM_EDGE_THRESHOLD,
    SIDE_DETAIL_GLASS_TITLE_TB,
    SIDE_DETAIL_GLASS_PARAM_FRAME_ID_TB,
    SIDE_DETAIL_GLASS_PARAM_SEARCH_DIRECTION_TB,
    SIDE_DETAIL_GLASS_PARAM_EDGE_DIRECTION_TB,
    SIDE_DETAIL_GLASS_PARAM_USE_ROUGH_SEARCH_TB,
    SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_WIDTH_TB,
    SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_LENGTH_TB,
    SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_WIDTH_TB,
    SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_LENGTH_TB,
    SIDE_DETAIL_GLASS_TITLE_LR,
    SIDE_DETAIL_GLASS_PARAM_FRAME_ID_LR,
    SIDE_DETAIL_GLASS_PARAM_SEARCH_DIRECTION_LR,
    SIDE_DETAIL_GLASS_PARAM_EDGE_DIRECTION_LR,
    SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_WIDTH_LR,
    SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_LENGTH_LR,
    SIDE_DETAIL_GLASS_PARAM_USE_COMPENSATION_LR,

    SIDE_DETAIL_PARAM_END,
};

static LPCTSTR g_szSideDetailParamName[] = {
    _T("Common parameters"), //SIDE_DETAIL_COMMON_TITLE
    _T("Edge Count"), //SIDE_DETAIL_COMMON_PARAM_EDGE_COUNT
    _T("Edge point noise filter type"), //SIDE_DETAIL_COMMON_PARAM_EDGE_POINT_NOISE_FILTER

    _T("Substrate alignment parameters"), //SIDE_DETAIL_SUBSTRATE_TITLE
    _T("Frame ID"), //SIDE_DETAIL_SUBSTRATE_PARAM_FRAME_ID
    _T("Image Combine"), //SIDE_DETAIL_SUBSTRATE_PARAM_IMAGE_COMBINE
    _T("Search Direction"), //SIDE_DETAIL_SUBSTRATE_PARAM_SEARCH_DIRECTION
    _T("Edge Direction"), //SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DIRECTION
    _T("Edge Detection Mode"), //SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DETECT_MODE
    _T("Edge Threshold"), //SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_THRESHOLD
    _T("[T/B] Edge Search Width (um)"), //SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_WIDTH_TB
    _T("[T/B] Edge Search Length T/B(um)"), //SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_LENGTH_TB
    _T("[L/R] Edge Search Width L/R(%)"), //SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_WIDTH_LR
    _T("[L/R] Edge Search Length L/R(um)"), //SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_LENGTH_LR
    _T("Inner Align Parameter"), //SIDE_DETAIL_SUBSTRATE_PARAM_INNER_ALIGN_PARAM

    _T("Glass alignment parameters [Common]"), //SIDE_DETAIL_GLASS_TITLE_COMMON
    _T("Edge Detection Mode"), //SIDE_DETAIL_GLASS_PARAM_EDGE_DETECT_MODE
    _T("Edge Threshold"), //SIDE_DETAIL_GLASS_PARAM_EDGE_THRESHOLD
    _T("Glass alignment parameters [Top/Bottom]"), //SIDE_DETAIL_GLASS_TITLE_TB
    _T("[T/B] Frame ID"), //SIDE_DETAIL_GLASS_PARAM_FRAME_ID_TB
    _T("[T/B] Search Direction"), //SIDE_DETAIL_GLASS_PARAM_SEARCH_DIRECTION_TB
    _T("[T/B] Edge Direction"), //SIDE_DETAIL_GLASS_PARAM_EDGE_DIRECTION_TB
    _T("[T/B] Use Rough Search"), //SIDE_DETAIL_GLASS_PARAM_USE_ROUGH_SEARCH_TB
    _T("[T/B] Rough Search Width (um)"), //SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_WIDTH_TB
    _T("[T/B] Rough Search Length (um)"), //SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_LENGTH_TB
    _T("[T/B] Edge Search Width (um)"), //SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_WIDTH_TB
    _T("[T/B] Edge Search Length (um)"), //SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_LENGTH_TB
    _T("Glass alignment parameters [Left/Right]"), //SIDE_DETAIL_GLASS_TITLE_LR
    _T("[L/R] Frame ID"), //SIDE_DETAIL_GLASS_PARAM_FRAME_ID_LR
    _T("[L/R] Search Direction"), //SIDE_DETAIL_GLASS_PARAM_SEARCH_DIRECTION_LR
    _T("[L/R] Edge Direction"), //SIDE_DETAIL_GLASS_PARAM_EDGE_DIRECTION_LR
    _T("[L/R] Edge Search Width L/R(%)"), //SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_WIDTH_LR
    _T("[L/R] Edge Search Length L/R(um)"), //SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_LENGTH_LR
    _T("[L/R] Use Compensation"), //SIDE_DETAIL_GLASS_PARAM_USE_COMPENSATION_LR,
};

IMPLEMENT_DYNAMIC(CDlgVisionProcessingSideDetailAlign, CDialog)

CDlgVisionProcessingSideDetailAlign::CDlgVisionProcessingSideDetailAlign(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingSideDetailAlign* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionProcessingSideDetailAlign::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlgRear(nullptr)
    , m_procCommonLogDlgRear(nullptr)
    , m_imageLotView(nullptr)
    , m_imageLotViewRearSide(nullptr)
    , m_propertyGrid(nullptr)
    , m_nCurCommonTab(TAB_DEBUG_INFO)
{
    m_pVisionInsp = pVisionInsp;
    m_pVisionInsp->ResetResult(); //kircheis_2DMatVMSDK
    m_pVisionPara = m_pVisionInsp->m_VisionPara;

    m_nImageID = 0;
}

CDlgVisionProcessingSideDetailAlign::~CDlgVisionProcessingSideDetailAlign()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDebugInfoDlgRear;
    delete m_procCommonLogDlgRear;
    delete m_imageLotView;
    delete m_imageLotViewRearSide;
    delete m_propertyGrid;
}

void CDlgVisionProcessingSideDetailAlign::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_STATIC_SIDE_FRONT_DETAILALIGN, m_Label_Side_Front_Status);
    DDX_Control(pDX, IDC_STATIC_SIDE_REAR_DETAILALIGN, m_Label_Side_Rear_Status);
}

BEGIN_MESSAGE_MAP(CDlgVisionProcessingSideDetailAlign, CDialog)
//	ON_WM_CREATE()
//	ON_WM_CLOSE()
ON_WM_DESTROY()
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionProcessingSideDetailAlign::OnTcnSelchangeTabResult)
//	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &DlgVisionProcessingAlign2D::OnTcnSelchangeTabResult)
//	ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_CHANGED, &DlgVisionProcessingAlign2D::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_ROI_CHANGED, &CDlgVisionProcessingSideDetailAlign::OnImageLotViewRoiChanged)
ON_STN_CLICKED(
    IDC_STATIC_SIDE_FRONT_DETAILALIGN, &CDlgVisionProcessingSideDetailAlign::OnStnClickedStaticSideFrontDetailalign)
ON_STN_CLICKED(
    IDC_STATIC_SIDE_REAR_DETAILALIGN, &CDlgVisionProcessingSideDetailAlign::OnStnClickedStaticSideRearDetailalign)
END_MESSAGE_MAP()

// CDlgVisionProcessingSideDetailAlign 메시지 처리기입니다.

BOOL CDlgVisionProcessingSideDetailAlign::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    // Side Main Image View - 2024.05.29_JHB
    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        return FALSE; //Side Vision이 아니면 이 모듈이 돌아서는 안된다.
    }
    else
    {
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
        m_imageLotViewRearSide = new ImageLotView(
            rtLotViewRegionR, *m_pVisionInsp, GetSafeHwnd(), enSideVisionModule::SIDE_VISIONMODULE_REAR);
    }
    /////////////////////////////////////////////

    m_imageLotView->SetMessage_RoiChanged(UM_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_CHANGED);
    m_imageLotViewRearSide->SetMessage_RoiChanged(UM_ROI_CHANGED);
    m_imageLotViewRearSide->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    CRect rtPara = m_procDlgInfo.m_rtParaArea;

    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID); // IDC_STATIC);

    ////// Side Status Selector 위치 설정 - 2024.05.29_JHB
    CRect rtSIdeFrontStatus, rtSIdeRearStatus;

    GetDlgItem(IDC_STATIC_SIDE_FRONT_DETAILALIGN)->GetWindowRect(rtSIdeFrontStatus);
    GetDlgItem(IDC_STATIC_SIDE_REAR_DETAILALIGN)->GetWindowRect(rtSIdeRearStatus);

    SetDlgItemText(IDC_STATIC_SIDE_FRONT_DETAILALIGN, _T("SIDE FRONT"));
    SetDlgItemText(IDC_STATIC_SIDE_REAR_DETAILALIGN, _T("SIDE REAR"));

    m_Label_Side_Front_Status.MoveWindow(m_procDlgInfo.m_rtSideFrontStatusArea);

    m_Label_Side_Rear_Status.MoveWindow(m_procDlgInfo.m_rtSideRearStatusArea);

    SetInitialSideVisionSelector();
    ////////////////////////////////////////////////////////

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Debug"));
    m_TabResult.InsertItem(1, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonLogDlgRear = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlgRear = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotViewRearSide, *m_procCommonLogDlgRear);

    const long nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_procCommonDebugInfoDlg->ShowWindow(SW_HIDE);
        m_procCommonDebugInfoDlgRear->ShowWindow(SW_SHOW);
    }
    else
    {
        m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);
        m_procCommonDebugInfoDlgRear->ShowWindow(SW_HIDE);
    }
    // Dialog Control 위치 설정
    UpdatePropertyGrid();

    // ROI 표시 - 2024.05.29_JHB
    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        m_imageLotView->Overlay_RemoveAll();
        m_imageLotView->ROI_RemoveAll();

        m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
        m_imageLotView->ZoomImageFit();
    }
    else
    {
        m_imageLotView->Overlay_RemoveAll();
        m_imageLotView->ROI_RemoveAll();

        m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
        m_imageLotView->ZoomImageFit();

        m_imageLotViewRearSide->Overlay_RemoveAll();
        m_imageLotViewRearSide->ROI_RemoveAll();

        m_imageLotViewRearSide->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
        m_imageLotViewRearSide->ZoomImageFit();
    }
    ///////////////////////////////////

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionProcessingSideDetailAlign::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonDebugInfoDlgRear->DestroyWindow();
    m_procCommonLogDlgRear->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionProcessingSideDetailAlign::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    m_nCurCommonTab = (long)m_TabResult.GetCurSel();

    UpdateCommonTabShow();

    if (pResult)
    {
        *pResult = 0;
    }
}

void CDlgVisionProcessingSideDetailAlign::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    const BOOL bIsGlassCorePackage = m_pVisionInsp->IsGlassCorePackage();

    if (auto* category = m_propertyGrid->AddCategory(g_szSideDetailParamName[SIDE_DETAIL_COMMON_TITLE])) //Common Param
    {
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(g_szSideDetailParamName[SIDE_DETAIL_COMMON_PARAM_EDGE_COUNT],
                m_pVisionPara->m_nEdgeCount, &m_pVisionPara->m_nEdgeCount))
            ->SetID(SIDE_DETAIL_COMMON_PARAM_EDGE_COUNT);

        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_COMMON_PARAM_EDGE_POINT_NOISE_FILTER],
                    m_pVisionPara->m_nEdgePointNoiseFilter, (int*)&m_pVisionPara->m_nEdgePointNoiseFilter)))
        {
            item->GetConstraints()->AddConstraint(_T("Not use filter"), ENF_NOT_USE);
            item->GetConstraints()->AddConstraint(_T("General filter"), ENF_USE_GENERAL_FILTER);
            item->GetConstraints()->AddConstraint(_T("RANSAC filter"), ENF_USE_RANSAC_FILTER);

            item->SetID(SIDE_DETAIL_COMMON_PARAM_EDGE_POINT_NOISE_FILTER);
        }
        category->Expand();
    }

    LPCTSTR szSubstrateTitle = (bIsGlassCorePackage == TRUE) ? g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_TITLE]
                                                             : _T("Detail alignment parameters");

    if (auto* category = m_propertyGrid->AddCategory(szSubstrateTitle)) //Substrate Align Param
    {
        m_pVisionPara->m_substrateAlignParam.m_alignFrameIndex.makePropertyGridItem(category,
            g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_FRAME_ID], SIDE_DETAIL_SUBSTRATE_PARAM_FRAME_ID);
        category
            ->AddChildItem(
                new CCustomItemButton(g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_IMAGE_COMBINE], TRUE, FALSE))
            ->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_IMAGE_COMBINE);
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_SEARCH_DIRECTION],
                    m_pVisionPara->m_substrateAlignParam.m_nSearchDirection,
                    (int*)&m_pVisionPara->m_substrateAlignParam.m_nSearchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), PI_ED_DIR_OUTER);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), PI_ED_DIR_INNER);

            item->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_SEARCH_DIRECTION);
        }
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DIRECTION],
                    m_pVisionPara->m_substrateAlignParam.m_nEdgeDirection,
                    (int*)&m_pVisionPara->m_substrateAlignParam.m_nEdgeDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("Both"), PI_ED_DIR_BOTH);
            item->GetConstraints()->AddConstraint(_T("Rising"), PI_ED_DIR_RISING);
            item->GetConstraints()->AddConstraint(_T("Falling"), PI_ED_DIR_FALLING);
            item->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DIRECTION);
        }
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DETECT_MODE],
                    m_pVisionPara->m_substrateAlignParam.m_nEdgeDetectMode,
                    (int*)&m_pVisionPara->m_substrateAlignParam.m_nEdgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), PI_ED_MAX_EDGE);
            item->GetConstraints()->AddConstraint(_T("First Edge"), PI_ED_FIRST_EDGE);
            item->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DETECT_MODE);
        }
        category
            ->AddChildItem(
                new CXTPPropertyGridItemDouble(g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_THRESHOLD],
                    m_pVisionPara->m_substrateAlignParam.m_dEdgeThreshold, _T("%.1lf"),
                    &m_pVisionPara->m_substrateAlignParam.m_dEdgeThreshold))
            ->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_THRESHOLD);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_WIDTH_TB],
                m_pVisionPara->m_substrateAlignParam.m_nEdgeSearchWidthTB_um,
                &m_pVisionPara->m_substrateAlignParam.m_nEdgeSearchWidthTB_um))
            ->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_WIDTH_TB);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_LENGTH_TB],
                m_pVisionPara->m_substrateAlignParam.m_nEdgeSearchLengthTB_um,
                &m_pVisionPara->m_substrateAlignParam.m_nEdgeSearchLengthTB_um))
            ->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_LENGTH_TB);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_WIDTH_LR],
                m_pVisionPara->m_substrateAlignParam.m_nEdgeSearchWidthLR_Ratio,
                &m_pVisionPara->m_substrateAlignParam.m_nEdgeSearchWidthLR_Ratio))
            ->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_WIDTH_LR);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_LENGTH_LR],
                m_pVisionPara->m_substrateAlignParam.m_nEdgeSearchLengthLR_um,
                &m_pVisionPara->m_substrateAlignParam.m_nEdgeSearchLengthLR_um))
            ->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_SEARCH_LENGTH_LR);
        if (bIsGlassCorePackage == TRUE)
        {
            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_SUBSTRATE_PARAM_INNER_ALIGN_PARAM],
                        m_pVisionPara->m_substrateAlignParam.m_nSubstrateInnerParamType,
                        (int*)&m_pVisionPara->m_substrateAlignParam.m_nSubstrateInnerParamType)))
            {
                item->GetConstraints()->AddConstraint(_T("Use Glass parameters"), USE_GLASS_PARAM);
                item->GetConstraints()->AddConstraint(_T("Use Substrate parameters"), USE_SUBSTRATE_PARAM);
                item->SetID(SIDE_DETAIL_SUBSTRATE_PARAM_EDGE_DETECT_MODE);
            }
        }
        else
            m_pVisionPara->m_substrateAlignParam.m_nSubstrateInnerParamType = USE_SUBSTRATE_PARAM;

        category->Expand();
    }
    if (bIsGlassCorePackage == TRUE)
    {
        if (auto* category
            = m_propertyGrid->AddCategory(g_szSideDetailParamName[SIDE_DETAIL_GLASS_TITLE_COMMON])) //Glass Align Param
        {
            //Common
            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_EDGE_DETECT_MODE],
                        m_pVisionPara->m_glassAlignParam.m_nEdgeDetectMode,
                        (int*)&m_pVisionPara->m_glassAlignParam.m_nEdgeDetectMode)))
            {
                item->GetConstraints()->AddConstraint(_T("Best Edge"), PI_ED_MAX_EDGE);
                item->GetConstraints()->AddConstraint(_T("First Edge"), PI_ED_FIRST_EDGE);
                item->SetID(SIDE_DETAIL_GLASS_PARAM_EDGE_DETECT_MODE);
            }
            category
                ->AddChildItem(
                    new CXTPPropertyGridItemDouble(g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_EDGE_THRESHOLD],
                        m_pVisionPara->m_glassAlignParam.m_dEdgeThreshold, _T("%.1lf"),
                        &m_pVisionPara->m_glassAlignParam.m_dEdgeThreshold))
                ->SetID(SIDE_DETAIL_GLASS_PARAM_EDGE_THRESHOLD);

            category->Expand();
        }
        if (auto* category
            = m_propertyGrid->AddCategory(g_szSideDetailParamName[SIDE_DETAIL_GLASS_TITLE_TB])) //Glass Align Param
        {
            //Top/Bottom
            auto frameIndex = m_pVisionPara->m_glassAlignParam.m_alignFrameIndex.makePropertyGridItem(category,
                g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_FRAME_ID_TB], SIDE_DETAIL_GLASS_PARAM_FRAME_ID_TB);
            frameIndex->SetReadOnly(!bIsGlassCorePackage);
            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_SEARCH_DIRECTION_TB],
                        m_pVisionPara->m_glassAlignParam.m_nSearchDirection,
                        (int*)&m_pVisionPara->m_glassAlignParam.m_nSearchDirection)))
            {
                item->GetConstraints()->AddConstraint(_T("In -> Out"), PI_ED_DIR_OUTER);
                item->GetConstraints()->AddConstraint(_T("Out -> In"), PI_ED_DIR_INNER);

                item->SetID(SIDE_DETAIL_GLASS_PARAM_SEARCH_DIRECTION_TB);
            }
            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_EDGE_DIRECTION_TB],
                        m_pVisionPara->m_glassAlignParam.m_nEdgeDirection,
                        (int*)&m_pVisionPara->m_glassAlignParam.m_nEdgeDirection)))
            {
                item->GetConstraints()->AddConstraint(_T("Both"), PI_ED_DIR_BOTH);
                item->GetConstraints()->AddConstraint(_T("Rising"), PI_ED_DIR_RISING);
                item->GetConstraints()->AddConstraint(_T("Falling"), PI_ED_DIR_FALLING);
                item->SetID(SIDE_DETAIL_GLASS_PARAM_EDGE_DIRECTION_TB);
            }
            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_USE_ROUGH_SEARCH_TB],
                        m_pVisionPara->m_glassAlignParam.m_bUseRoughAlign,
                        (int*)&m_pVisionPara->m_glassAlignParam.m_bUseRoughAlign)))
            {
                item->GetConstraints()->AddConstraint(_T("Not Use"), FALSE);
                item->GetConstraints()->AddConstraint(_T("Use"), TRUE);
                item->SetID(SIDE_DETAIL_GLASS_PARAM_USE_ROUGH_SEARCH_TB);
            }
            category
                ->AddChildItem(new CXTPPropertyGridItemNumber(
                    g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_WIDTH_TB],
                    m_pVisionPara->m_glassAlignParam.m_nRoughEdgeSearchWidthTB_um,
                    &m_pVisionPara->m_glassAlignParam.m_nRoughEdgeSearchWidthTB_um))
                ->SetID(SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_WIDTH_TB);
            category
                ->AddChildItem(new CXTPPropertyGridItemNumber(
                    g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_LENGTH_TB],
                    m_pVisionPara->m_glassAlignParam.m_nRoughEdgeSearchLengthTB_um,
                    &m_pVisionPara->m_glassAlignParam.m_nRoughEdgeSearchLengthTB_um))
                ->SetID(SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_LENGTH_TB);
            category
                ->AddChildItem(new CXTPPropertyGridItemNumber(
                    g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_WIDTH_TB],
                    m_pVisionPara->m_glassAlignParam.m_nEdgeSearchWidthTB_um,
                    &m_pVisionPara->m_glassAlignParam.m_nEdgeSearchWidthTB_um))
                ->SetID(SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_WIDTH_TB);
            category
                ->AddChildItem(new CXTPPropertyGridItemNumber(
                    g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_LENGTH_TB],
                    m_pVisionPara->m_glassAlignParam.m_nEdgeSearchLengthTB_um,
                    &m_pVisionPara->m_glassAlignParam.m_nEdgeSearchLengthTB_um))
                ->SetID(SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_LENGTH_TB);

            category->Expand();
        }
        if (auto* category
            = m_propertyGrid->AddCategory(g_szSideDetailParamName[SIDE_DETAIL_GLASS_TITLE_LR])) //Glass Align Param
        {
            //Left/Right
            auto frameIndex = m_pVisionPara->m_glassAlignParam.m_alignFrameIndexGlassLR.makePropertyGridItem(category,
                g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_FRAME_ID_LR], SIDE_DETAIL_GLASS_PARAM_FRAME_ID_LR);
            frameIndex->SetReadOnly(!bIsGlassCorePackage);
            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_SEARCH_DIRECTION_LR],
                        m_pVisionPara->m_glassAlignParam.m_nSearchDirectionGlassLR,
                        (int*)&m_pVisionPara->m_glassAlignParam.m_nSearchDirectionGlassLR)))
            {
                item->GetConstraints()->AddConstraint(_T("In -> Out"), PI_ED_DIR_OUTER);
                item->GetConstraints()->AddConstraint(_T("Out -> In"), PI_ED_DIR_INNER);

                item->SetID(SIDE_DETAIL_GLASS_PARAM_SEARCH_DIRECTION_LR);
            }
            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_EDGE_DIRECTION_LR],
                        m_pVisionPara->m_glassAlignParam.m_nEdgeDirectionGlassLR,
                        (int*)&m_pVisionPara->m_glassAlignParam.m_nEdgeDirectionGlassLR)))
            {
                item->GetConstraints()->AddConstraint(_T("Both"), PI_ED_DIR_BOTH);
                item->GetConstraints()->AddConstraint(_T("Rising"), PI_ED_DIR_RISING);
                item->GetConstraints()->AddConstraint(_T("Falling"), PI_ED_DIR_FALLING);
                item->SetID(SIDE_DETAIL_GLASS_PARAM_EDGE_DIRECTION_LR);
            }
            category
                ->AddChildItem(new CXTPPropertyGridItemNumber(
                    g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_WIDTH_LR],
                    m_pVisionPara->m_glassAlignParam.m_nEdgeSearchWidthLR_Ratio,
                    &m_pVisionPara->m_glassAlignParam.m_nEdgeSearchWidthLR_Ratio))
                ->SetID(SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_WIDTH_LR);
            category
                ->AddChildItem(new CXTPPropertyGridItemNumber(
                    g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_LENGTH_LR],
                    m_pVisionPara->m_glassAlignParam.m_nEdgeSearchLengthLR_um,
                    &m_pVisionPara->m_glassAlignParam.m_nEdgeSearchLengthLR_um))
                ->SetID(SIDE_DETAIL_GLASS_PARAM_EDGE_SEARCH_LENGTH_LR);

            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(g_szSideDetailParamName[SIDE_DETAIL_GLASS_PARAM_USE_COMPENSATION_LR],
                        m_pVisionPara->m_bUseGlassCompensationLeftRight,
                        (int*)&m_pVisionPara->m_bUseGlassCompensationLeftRight)))
            {
                item->GetConstraints()->AddConstraint(_T("Not Use"), 0);
                item->GetConstraints()->AddConstraint(_T("Use"), 1);

                item->SetID(SIDE_DETAIL_GLASS_PARAM_USE_COMPENSATION_LR);
            }
            category->Expand();
        }

        const BOOL bDisableRoughAlignParam = (m_pVisionPara->m_glassAlignParam.m_bUseRoughAlign == TRUE) ? FALSE : TRUE;
        m_propertyGrid->GetItem(SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_WIDTH_TB)
            ->SetReadOnly(bDisableRoughAlignParam);
        m_propertyGrid->GetItem(SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_LENGTH_TB)
            ->SetReadOnly(bDisableRoughAlignParam);
    }

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionProcessingSideDetailAlign::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CString strTemp{};

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (item->GetID())
        {
            case SIDE_DETAIL_SUBSTRATE_PARAM_FRAME_ID:
            {
                long nAlignFrameIdx = m_pVisionPara->m_substrateAlignParam.m_alignFrameIndex.getFrameIndex();
                m_nImageID = nAlignFrameIdx;
                ShowImage(nAlignFrameIdx);
                break;
            }
            case SIDE_DETAIL_SUBSTRATE_PARAM_IMAGE_COMBINE:
            {
                ClickedButtonSubstrateImageCombine();
                break;
            }
            case SIDE_DETAIL_GLASS_PARAM_FRAME_ID_TB:
            {
                long nAlignFrameIdx = m_pVisionPara->m_glassAlignParam.m_alignFrameIndex.getFrameIndex();
                m_nImageID = nAlignFrameIdx;
                ShowImage(nAlignFrameIdx);
                break;
            }
            case SIDE_DETAIL_GLASS_PARAM_FRAME_ID_LR:
            {
                long nAlignFrameIdx = m_pVisionPara->m_glassAlignParam.m_alignFrameIndexGlassLR.getFrameIndex();
                m_nImageID = nAlignFrameIdx;
                ShowImage(nAlignFrameIdx);
                break;
            }
            case SIDE_DETAIL_GLASS_PARAM_USE_ROUGH_SEARCH_TB:
            {
                BOOL bUseRoughAlign = FALSE;
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    bUseRoughAlign = (BOOL)value->GetEnum();
                if (bUseRoughAlign == FALSE)
                {
                    m_propertyGrid->GetItem(SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_WIDTH_TB)->SetReadOnly(TRUE);
                    m_propertyGrid->GetItem(SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_LENGTH_TB)->SetReadOnly(TRUE);
                }
                else
                {
                    m_propertyGrid->GetItem(SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_WIDTH_TB)->SetReadOnly(FALSE);
                    m_propertyGrid->GetItem(SIDE_DETAIL_GLASS_PARAM_ROUGH_EDGE_SEARCH_LENGTH_TB)->SetReadOnly(FALSE);
                }
            }
            break;
        }
    }

    strTemp.Empty();

    return 0;
}

void CDlgVisionProcessingSideDetailAlign::ClickedButtonSubstrateImageCombine()
{
    auto* proc = m_pVisionInsp;

    const auto nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    if (::Combine_SetParameter(*proc, m_pVisionPara->m_substrateAlignParam.m_alignFrameIndex.getFrameIndex(), false,
            &proc->m_VisionPara->m_substrateAlignParam.m_alignImageProcManagePara)
        == IDOK)
    {
        ////////////////////////
        auto processor = m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_2D);
        auto& memory = processor->getReusableMemory();

        // Raw 이미지를 받아온다.
        auto frameIndex = m_pVisionPara->m_substrateAlignParam.m_alignFrameIndex.getFrameIndex();
        m_pVisionPara->m_substrateAlignParam.m_alignFrameIndex.setFrameIndex(frameIndex);
        const auto& image = m_pVisionPara->m_substrateAlignParam.m_alignFrameIndex.getImage(false);
        if (image.GetMem() == nullptr)
        {
            return;
        }

        Ipvm::Image8u combineImage;

        // 이미지 raw로 받아옴
        if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
            return;

        // Combine 이미지 생성 > 기존 값은 원본 이미지로 하였다.
        Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), combineImage);
        Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());

        // 이미지 combine 해서 결과 표시
        if (CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_substrateAlignParam.m_alignImageProcManagePara, combineImage)
            == false)
        {
            return;
        }

        // 이미지 표시 - 2024.05.29_JHB
        if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
        {
            m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(frameIndex));
            m_imageLotView->SetImage(combineImage, _T("Substrate Combined Image **"));
        }
        else
        {
            if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            {
                m_imageLotViewRearSide->ShowImage(m_pVisionInsp->GetImageFrameIndex(frameIndex));
                m_imageLotViewRearSide->SetImage(combineImage, _T("Substrate Combined Side Rear Image **"));
            }
            else
            {
                m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(frameIndex));
                m_imageLotView->SetImage(combineImage, _T("Substrate Combined Side Front Image **"));
            }
        }
    }
}

void CDlgVisionProcessingSideDetailAlign::ClickedButtonGlassImageCombine()
{
    auto* proc = m_pVisionInsp;
    const auto nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    if (::Combine_SetParameter(*proc, m_pVisionPara->m_glassAlignParam.m_alignFrameIndex.getFrameIndex(), false,
            &proc->m_VisionPara->m_glassAlignParam.m_alignImageProcManagePara)
        == IDOK)
    {
        ////////////////////////
        auto processor = m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_2D);
        auto& memory = processor->getReusableMemory();

        // Raw 이미지를 받아온다.
        auto frameIndex = m_pVisionPara->m_glassAlignParam.m_alignFrameIndex.getFrameIndex();
        m_pVisionPara->m_glassAlignParam.m_alignFrameIndex.setFrameIndex(frameIndex);
        const auto& image = m_pVisionPara->m_glassAlignParam.m_alignFrameIndex.getImage(false);

        if (image.GetMem() == nullptr)
        {
            return;
        }

        Ipvm::Image8u combineImage;

        // 이미지 raw로 받아옴
        if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
            return;

        // Combine 이미지 생성 > 기존 값은 원본 이미지로 하였다.
        Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), combineImage);
        Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());

        // 이미지 combine 해서 결과 표시
        if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_glassAlignParam.m_alignImageProcManagePara, combineImage))
        {
            return;
        }

        // 이미지 표시 - 2024.05.29_JHB
        if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
        {
            m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), TRUE);
            m_imageLotView->SetImage(combineImage, _T("Substrate Combined Image **"));
        }
        else
        {
            if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
                m_imageLotViewRearSide->SetImage(combineImage, _T("Substrate Combined Side Rear Image **"));
            else
                m_imageLotView->SetImage(combineImage, _T("Substrate Combined Side Front Image **"));
        }
    }
}

LRESULT CDlgVisionProcessingSideDetailAlign::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    //m_imageLotView->ShowCurrentImage(false);
    //m_imageLotView->ZoomImageFit();

    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        m_imageLotView->ShowCurrentImage(false);
        m_imageLotView->ZoomImageFit();
    }
    else
    {
        m_imageLotView->ShowCurrentImage(false);
        m_imageLotView->ZoomImageFit();

        m_imageLotViewRearSide->ShowCurrentImage(false);
        m_imageLotViewRearSide->ZoomImageFit();
    }

    return 0L;
}

void CDlgVisionProcessingSideDetailAlign::AfterInspect()
{
}

void CDlgVisionProcessingSideDetailAlign::OnBnClickedButtonInspect()
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

        auto& visionUnit = m_pVisionInsp->m_visionUnit;

        visionUnit.RunInspection(m_pVisionInsp, false, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());
        m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
        m_imageLotView->Overlay_Show(TRUE);

        m_procCommonDebugInfoDlg->Refresh();
    }
    else
    {
        m_imageLotViewRearSide->Overlay_RemoveAll();
        m_procCommonLogDlgRear->SetLogBoxText(_T(""));

        auto& visionUnit = m_pVisionInsp->m_visionUnit;

        visionUnit.RunInspection(m_pVisionInsp, false, enSideVisionModule::SIDE_VISIONMODULE_REAR);
        visionUnit.GetInspectionOverlayResult().Apply(m_imageLotViewRearSide->GetCoreView());
        m_procCommonLogDlgRear->SetLogBoxText(visionUnit.GetLastInspection_Text());
        m_imageLotViewRearSide->Overlay_Show(TRUE);

        m_procCommonDebugInfoDlgRear->Refresh();
    }

    // 검사 중간 결과 화면 갱신
}

void CDlgVisionProcessingSideDetailAlign::ShowImage(long nFrameIdx)
{
    const long nVisionType = SystemConfig::GetInstance().GetVisionType();
    const long nCurSideSelection = m_pVisionInsp->GetCurVisionModule_Status();

    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage(false, nFrameIdx);
    if (image.GetMem() == nullptr)
        return;

    if (nVisionType != VISIONTYPE_SIDE_INSP
        || (nVisionType == VISIONTYPE_SIDE_INSP && nCurSideSelection == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
    {
        m_imageLotView->SetImage(image);
    }
    else
    {
        m_imageLotViewRearSide->SetImage(image);
    }
}

void CDlgVisionProcessingSideDetailAlign::SetInitialSideVisionSelector()
{
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

void CDlgVisionProcessingSideDetailAlign::OnStnClickedStaticSideFrontDetailalign()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_pVisionInsp->m_visionUnit.GetSideVisionSection() != enSideVisionModule::SIDE_VISIONMODULE_FRONT)
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

    if (m_pVisionInsp->m_visionUnit.IsTheUsingVisionProcessingByGuid(
            m_pVisionInsp->m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    UpdateCommonTabShow();
}

void CDlgVisionProcessingSideDetailAlign::OnStnClickedStaticSideRearDetailalign()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_pVisionInsp->m_visionUnit.GetSideVisionSection() != enSideVisionModule::SIDE_VISIONMODULE_REAR)
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

    if (m_pVisionInsp->m_visionUnit.IsTheUsingVisionProcessingByGuid(
            m_pVisionInsp->m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    m_imageLotViewRearSide->Overlay_RemoveAll();
    m_imageLotViewRearSide->ROI_RemoveAll();

    m_imageLotViewRearSide->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotViewRearSide->ZoomImageFit();

    UpdateCommonTabShow();
}

void CDlgVisionProcessingSideDetailAlign::UpdateCommonTabShow()
{
    const long nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    m_procCommonDebugInfoDlg->ShowWindow(SW_HIDE);
    m_procCommonDebugInfoDlgRear->ShowWindow(SW_HIDE);
    m_procCommonLogDlg->ShowWindow(SW_HIDE);
    m_procCommonLogDlgRear->ShowWindow(SW_HIDE);

    if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        if (m_nCurCommonTab == TAB_DEBUG_INFO)
            m_procCommonDebugInfoDlgRear->ShowWindow(SW_SHOW);
        else
            m_procCommonLogDlgRear->ShowWindow(SW_SHOW);
    }
    else
    {
        if (m_nCurCommonTab == TAB_DEBUG_INFO)
            m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);
        else
            m_procCommonLogDlg->ShowWindow(SW_SHOW);
    }
}
