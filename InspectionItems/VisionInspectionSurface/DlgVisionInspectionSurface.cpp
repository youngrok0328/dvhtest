//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionSurface.h"

//CPP_2_________________________________ This project's headers
#include "SurfaceCriteriaDlg.h"
#include "SurfaceROIEditorDlg.h"
#include "SurfaceUserCustomMaskDlg.h"
#include "VisionInspectionSurface.h"
#include "VisionInspectionSurfacePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <iomanip>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)

//CPP_7_________________________________ Implementation body
//
enum PropertyButtonItemID
{
    ITEM_ALGORITHM_BASICPARAM_USE_VMAP_IMAGE,
    ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_UM,
    ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_PX,
    ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_UM,
    ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_PX,
    ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_UM,
    ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_PX,
    ITEM_ALGORITHM_BASICPARAM_MERGE_ONLY_SAME_COLOR,
    ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_UM,
    ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_PX,
    ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_UM,
    ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_PX,
    ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_UM,
    ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_PX,
    ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_UM,
    ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_PX,
    ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_UM,
    ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_PX,
    ITEM_ALGORITHM_2NDINSPPARAM_USE_INSPECTION,
    ITEM_ALGORITHM_2NDINSPPARAM_MATCH_CODE,
    ITEM_ALGORITHM_2NDINSPPARAM_DLINSP_ONLY,
    ITEM_BUTTON_ID_IMAGECOMBINE,
    ITEM_BUTTON_ID_THRESHOLDANDROI,
    ITEM_BUTTON_ID_CRITERIAEDITOR,
    ITEM_BUTTON_ID_USERCUSTOMMASK,
    ITEM_BUTTON_ID_USERCUSTOMMASK_TEACH,
    ITEM_BUTTON_ID_GENERATEMASKALWAY,
};

enum enumSetupInfoTab
{
    TAB_INSPECTION_SPEC = 0,
    TAB_INSPECTION_RESULT,
    TAB_DETAIL_RESULT,
    TAB_DEBUG_INFO,
    TAB_TXT_LOG,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionSurface, CDialog)

CDlgVisionInspectionSurface::CDlgVisionInspectionSurface(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionSurface* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionSurface::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_imageLotView(nullptr)
    , m_imageLotViewRearSide(nullptr)
    , m_propertyGrid(nullptr)
    , m_procCommonSpecDlg(nullptr)
    // Normal & Front
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    // Rear
    , m_procCommonResultDlgRear(nullptr)
    , m_procCommonDetailResultDlgRear(nullptr)
    , m_procCommonLogDlgRear(nullptr)
    , m_procCommonDebugInfoDlgRear(nullptr)

    //{{View Um to Px
    , m_dMinAreaBrightContrast_Px(0.f)
    , m_dMinAreaDarkContrast_Px(0.f)
    , m_dMergeDistance_Px(0.f)
    , m_dfLocalContrastAreaSpan_Px(0.f)
    , m_dbackground_window_size_x_Px(0.f)
    , m_dbackground_window_size_y_Px(0.f)
    , m_dMinimumBrightBlobArea_Px(0.f)
    , m_dMininumDarkBlobArea_Px(0.f)
    , m_nUsed3DImageType(0)
    //}}

    , m_nCurCommonTab(TAB_INSPECTION_SPEC)
{
    m_pVisionInsp->ResetResult();
    m_pVisionPara = m_pVisionInsp->m_surfacePara;

    m_bShowResultDetail = FALSE;
    m_nImageID = 0;
}

CDlgVisionInspectionSurface::~CDlgVisionInspectionSurface()
{
    // Normal & Front
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;

    // Rear
    delete m_procCommonDebugInfoDlgRear;
    delete m_procCommonLogDlgRear;
    delete m_procCommonDetailResultDlgRear;
    delete m_procCommonResultDlgRear;

    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    delete m_imageLotView;
    delete m_imageLotViewRearSide;

    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionInspectionSurface::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_STATIC_SIDE_FRONT_SURFACE, m_Label_Side_Front_Status);
    DDX_Control(pDX, IDC_STATIC_SIDE_REAR_SURFACE, m_Label_Side_Rear_Status);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionSurface, CDialog)
ON_WM_CREATE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionSurface::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionSurface::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionSurface::OnImageLotViewPaneSelChanged)
ON_STN_CLICKED(IDC_STATIC_SIDE_FRONT_SURFACE, &CDlgVisionInspectionSurface::OnStnClickedStaticSideFrontSurface)
ON_STN_CLICKED(IDC_STATIC_SIDE_REAR_SURFACE, &CDlgVisionInspectionSurface::OnStnClickedStaticSideRearSurface)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionSurface::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(rtClient);

    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

        m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
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

        m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
        m_imageLotViewRearSide->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    }

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);

    ////// Side Status Selector 위치 설정
    CRect rtSIdeFrontStatus, rtSIdeRearStatus;

    GetDlgItem(IDC_STATIC_SIDE_FRONT_SURFACE)->GetWindowRect(rtSIdeFrontStatus);
    GetDlgItem(IDC_STATIC_SIDE_REAR_SURFACE)->GetWindowRect(rtSIdeRearStatus);

    SetDlgItemText(IDC_STATIC_SIDE_FRONT_SURFACE, _T("SIDE FRONT"));
    SetDlgItemText(IDC_STATIC_SIDE_REAR_SURFACE, _T("SIDE REAR"));

    m_Label_Side_Front_Status.MoveWindow(m_procDlgInfo.m_rtSideFrontStatusArea);

    m_Label_Side_Rear_Status.MoveWindow(m_procDlgInfo.m_rtSideRearStatusArea);

    SetInitialSideVisionSelector();
    ////////////////////////////////////////////////////////

    // Dialog Control 위치 설정
    SetInitDialog();

    if (SystemConfig::GetInstance().m_nVisionType == VISIONTYPE_3D_INSP)
    {
        m_imageLotView->ShowImage(m_pVisionPara->m_SurfaceItem.AlgoPara.nUse3DImageType);
        m_imageLotView->ZoomPaneFit(true);
    }
    else
    {
        bool bIsSideVision = SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP;
        //bool bIsSideFront = SystemConfig::GetInstance().GetSideVisionNumber() == enSideVisionModule::SIDE_VISIONMODULE_FRONT ? true : false;

        if (bIsSideVision == false /* || (bIsSideVision && bIsSideFront)*/)
        {
            m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0));
            m_imageLotView->ZoomImageFit();
        }
        else
        {
            // Front/Rear 이미지 잘 가지고 오는지 확인 필요...
            m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0));
            m_imageLotView->ZoomImageFit();

            m_imageLotViewRearSide->ShowImage(m_pVisionInsp->GetImageFrameIndex(0));
            m_imageLotViewRearSide->ZoomImageFit();
        }
    }

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspectionSurface::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CDlgVisionInspectionSurface::OnDestroy()
{
    CDialog::OnDestroy();

    // Normal & Front
    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    // Rear
    m_procCommonDebugInfoDlgRear->DestroyWindow();
    m_procCommonLogDlgRear->DestroyWindow();
    m_procCommonResultDlgRear->DestroyWindow();
    m_procCommonDetailResultDlgRear->DestroyWindow();

    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionSurface::SetInitDialog()
{
    //전체적인 Setup UI 설정(ImageWindow,  Spec, Result, Log etc..)
    SetInitSetupWindow();

    //Parameter UI 설정
    UpdatePropertyGrid();
}

void CDlgVisionInspectionSurface::SetInitSetupWindow()
{
    // Dialog Size 설정, Dialog 위치 설정
    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_fixedInspectionSpecs);

    // Normal & Front
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    // Rear
    m_procCommonResultDlgRear = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup,
        *m_imageLotViewRearSide, m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlgRear
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotViewRearSide);
    m_procCommonLogDlgRear = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlgRear = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotViewRearSide, *m_procCommonLogDlg);

    UpdateCommonTabShow();

    //m_procCommonSpecDlg->ShowWindow(SW_SHOW);
}

void CDlgVisionInspectionSurface::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* BasicParam = m_propertyGrid->AddCategory(_T("Basic Parameter")))
    {
        if (auto* item = BasicParam->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Use Image"), m_pVisionPara->m_SurfaceItem.AlgoPara.nUse3DImageType,
                    (int*)&m_pVisionPara->m_SurfaceItem.AlgoPara.nUse3DImageType)))
        {
            item->GetConstraints()->AddConstraint(_T("Use ZMap"), SURFACE_3D_USE_ZMAP);
            item->GetConstraints()->AddConstraint(_T("Use VMap"), SURFACE_3D_USE_VMAP);

            item->SetID(ITEM_ALGORITHM_BASICPARAM_USE_VMAP_IMAGE);
            if (SystemConfig::GetInstance().IsVisionTypeBased2D() == true) //kircheis_SWIR
            {
                item->SetHidden(TRUE);
            }
            else
            {
                item->SetHidden(FALSE);
            }
        }

        if (auto* Item
            = BasicParam->AddChildItem(new CXTPPropertyGridItemDouble(_T("Minimum Area for Key Contrast (Bright)"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.fMinAreaBrightContrast, _T("%.2lf Um^2"),
                (double*)&m_pVisionPara->m_SurfaceItem.AlgoPara.fMinAreaBrightContrast)))
        {
            Item->SetID(ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_UM);
            ConvertUm2Px(ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_UM);

            if (auto* pxItem = Item->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T(""), m_dMinAreaBrightContrast_Px, _T("%.2lf Px^2"), (double*)&m_dMinAreaBrightContrast_Px)))
            {
                pxItem->SetID(ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_PX);
                //ConvertPx2Um(ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_PX);
            }
        }

        if (auto* Item = BasicParam->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Minimum Area for Key Contrast (Dark)"), m_pVisionPara->m_SurfaceItem.AlgoPara.fMinAreaDarkContrast,
                _T("%.2lf Um^2"), (double*)&m_pVisionPara->m_SurfaceItem.AlgoPara.fMinAreaDarkContrast)))
        {
            Item->SetID(ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_UM);
            ConvertUm2Px(ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_UM);

            if (auto* pxItem = Item->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T(""), m_dMinAreaDarkContrast_Px, _T("%.2lf Px^2"), (double*)&m_dMinAreaDarkContrast_Px)))
            {
                pxItem->SetID(ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_PX);
                //ConvertPx2Um(ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_PX);
            }
        }

        if (auto* Item = BasicParam->AddChildItem(new CXTPPropertyGridItemDouble(_T("Merge Distance"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.m_mergeDistance, _T("%.2lf Um"),
                (double*)&m_pVisionPara->m_SurfaceItem.AlgoPara.m_mergeDistance)))
        {
            Item->SetID(ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_UM);
            ConvertUm2Px(ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_UM);

            if (auto* pxItem = Item->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T(""), m_dMergeDistance_Px, _T("%.2lf Px"), (double*)&m_dMergeDistance_Px)))
            {
                pxItem->SetID(ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_PX);
                //ConvertPx2Um(ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_PX);
            }
        }

        if (auto* Item = BasicParam->AddChildItem(new CXTPPropertyGridItemBool(_T("Merge Only The Same Color"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.m_mergeOnlyTheSameColor,
                &m_pVisionPara->m_SurfaceItem.AlgoPara.m_mergeOnlyTheSameColor)))
        {
            Item->SetID(ITEM_ALGORITHM_BASICPARAM_MERGE_ONLY_SAME_COLOR);
        }

        if (auto* Item = BasicParam->AddChildItem(new CXTPPropertyGridItemDouble(_T("Local Contrast Area Span"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.fLocalContrastAreaSpan, _T("%.2lf Um"),
                (double*)&m_pVisionPara->m_SurfaceItem.AlgoPara.fLocalContrastAreaSpan)))
        {
            Item->SetID(ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_UM);
            ConvertUm2Px(ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_UM);

            if (auto* pxItem = Item->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T(""), m_dfLocalContrastAreaSpan_Px, _T("%.2lf Px"), (double*)&m_dfLocalContrastAreaSpan_Px)))
            {
                pxItem->SetID(ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_PX);
                //ConvertPx2Um(ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_PX);
            }
        }

        BasicParam->Expand();
    }

    if (auto* BackgroundParam = m_propertyGrid->AddCategory(_T("Background Parameter")))
    {
        if (auto* Item = BackgroundParam->AddChildItem(new CXTPPropertyGridItemDouble(_T("Window Size X"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_x, _T("%.2lf Um"),
                (double*)&m_pVisionPara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_x)))
        {
            Item->SetDescription(_T("'Window size X' must be smaller than the package size and greater than the ")
                                 _T("minimum defect size x 1.5 to be detected."));
            Item->SetID(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_UM);
            ConvertUm2Px(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_UM);

            if (auto* pxItem = Item->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T(""), m_dbackground_window_size_x_Px, _T("%.lf Px"), (double*)&m_dbackground_window_size_x_Px)))
            {
                pxItem->SetDescription(_T("'Window size X' must be smaller than the package size and greater than the ")
                                       _T("minimum defect size x 1.5 to be detected."));
                pxItem->SetID(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_PX);
                //ConvertPx2Um(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_PX);
            }
        }

        if (auto* Item = BackgroundParam->AddChildItem(new CXTPPropertyGridItemDouble(_T("Window Size Y"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_y, _T("%.2lf Um"),
                (double*)&m_pVisionPara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_y)))
        {
            Item->SetDescription(_T("'Window size Y' must be smaller than the package size and greater than the ")
                                 _T("minimum defect size x 1.5 to be detected."));
            Item->SetID(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_UM);
            ConvertUm2Px(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_UM);

            if (auto* pxItem = Item->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T(""), m_dbackground_window_size_y_Px, _T("%.lf Px"), (double*)&m_dbackground_window_size_y_Px)))
            {
                pxItem->SetDescription(_T("'Window size Y' must be smaller than the package size and greater than the ")
                                       _T("minimum defect size x 1.5 to be detected."));
                pxItem->SetID(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_PX);
                //ConvertPx2Um(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_PX);
            }
        }

        BackgroundParam->Expand();
    }

    if (auto* BlobParam = m_propertyGrid->AddCategory(_T("Blob Parameter")))
    {
        if (auto* Item = BlobParam->AddChildItem(new CXTPPropertyGridItemDouble(_T("Minimum Bright Blob Area"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.fMinimumBrightBlobArea_umSqure, _T("%.2lf Um^2"),
                (double*)&m_pVisionPara->m_SurfaceItem.AlgoPara.fMinimumBrightBlobArea_umSqure)))
        {
            Item->SetID(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_UM);
            ConvertUm2Px(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_UM);

            if (auto* pxItem = Item->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T(""), m_dMinimumBrightBlobArea_Px, _T("%.2lf Px^2"), (double*)&m_dMinimumBrightBlobArea_Px)))
            {
                pxItem->SetID(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_PX);
                //ConvertPx2Um(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_PX);
            }
        }

        if (auto* Item = BlobParam->AddChildItem(new CXTPPropertyGridItemDouble(_T("Minimum Dark Blob Area"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.fMininumDarkBlobArea_umSqure, _T("%.2lf Um^2"),
                (double*)&m_pVisionPara->m_SurfaceItem.AlgoPara.fMininumDarkBlobArea_umSqure)))
        {
            Item->SetID(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_UM);
            ConvertUm2Px(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_UM);

            if (auto* pxItem = Item->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T(""), m_dMininumDarkBlobArea_Px, _T("%.2lf Px^2"), (double*)&m_dMininumDarkBlobArea_Px)))
            {
                pxItem->SetID(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_PX);
                //ConvertPx2Um(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_PX);
            }
        }

        if (auto* Item = BlobParam->AddChildItem(new CXTPPropertyGridItemNumber(_T("Maximum Bright Blob Count"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.nMaxBrightBlobNum_New,
                (long*)&m_pVisionPara->m_SurfaceItem.AlgoPara.nMaxBrightBlobNum_New)))
        {
        }

        if (auto* Item = BlobParam->AddChildItem(new CXTPPropertyGridItemNumber(_T("Maximum Dark Blob Count"),
                m_pVisionPara->m_SurfaceItem.AlgoPara.nMaxDarkBlobNum_New,
                (long*)&m_pVisionPara->m_SurfaceItem.AlgoPara.nMaxDarkBlobNum_New)))
        {
        }

        BlobParam->Expand();
    }

    if (m_pVisionInsp->m_visionUnit.m_systemConfig.m_bUseAiInspection)
    {
        if (auto* DLParam = m_propertyGrid->AddCategory(_T("2nd Inspection Parameter (Deep Learning)")))
        {
            if (auto* Item = DLParam->AddChildItem(new CXTPPropertyGridItemBool(_T("Use Inspection"),
                    m_pVisionPara->m_SurfaceItem.AlgoPara.m_use2ndInspection,
                    &m_pVisionPara->m_SurfaceItem.AlgoPara.m_use2ndInspection)))
            {
                Item->SetID(ITEM_ALGORITHM_2NDINSPPARAM_USE_INSPECTION);
            }

            //Read2ndInspCodeInfo(_T("C:\\intekplus\\iDL\\Vision_x64\\System\\DLVisionSystem.ini"));

            //int nSelIndx = -1;

            //for (long i = 0; i < m_vecstr2ndInspCode.size(); i++)
            //{
            //	if (m_pVisionPara->m_SurfaceItem.AlgoPara.m_str2ndInspCode == m_vecstr2ndInspCode[i])
            //	{
            //		nSelIndx = i;
            //	}
            //}

            //auto *Item = DLParam->AddChildItem(new CXTPPropertyGridItemEnum(_T("DL Model Match Code"), nSelIndx));
            //for (long i = 0; i < m_vecstr2ndInspCode.size(); i++)
            //{
            //	Item->GetConstraints()->AddConstraint(m_vecstr2ndInspCode[i], i);
            //}
            if (auto* Item = DLParam->AddChildItem(new CXTPPropertyGridItem(_T("DL Model Match Code"),
                    m_pVisionPara->m_SurfaceItem.AlgoPara.m_str2ndInspCode,
                    &m_pVisionPara->m_SurfaceItem.AlgoPara.m_str2ndInspCode)))
            {
                Item->SetID(ITEM_ALGORITHM_2NDINSPPARAM_MATCH_CODE);

                //Read2ndInspCodeInfo(_T("C:\\intekplus\\iDL\\Vision_x64\\System\\DLVisionSystem.ini"));
            }

            //if (auto *Item = DLParam->AddChildItem(new CXTPPropertyGridItem(_T("DL Model Match Code"),
            //	m_pVisionPara->m_SurfaceItem.AlgoPara.m_str2ndInspCode, &m_pVisionPara->m_SurfaceItem.AlgoPara.m_str2ndInspCode)))
            //{
            //	Item->SetID(ITEM_ALGORITHM_2NDINSPPARAM_MATCH_CODE);

            //	Read2ndInspCodeInfo(_T("C:\\intekplus\\iDL\\Vision_x64\\System\\DLVisionSystem.ini"));

            //}

            //if (auto *Item = DLParam->AddChildItem(new CXTPPropertyGridItemBool(_T("Only DL Inspection"),
            //	m_pVisionPara->m_SurfaceItem.AlgoPara.m_useOnlyDLInsp, &m_pVisionPara->m_SurfaceItem.AlgoPara.m_useOnlyDLInsp)))
            //{
            //	Item->SetID(ITEM_ALGORITHM_2NDINSPPARAM_DLINSP_ONLY);
            //}

            DLParam->Expand();
        }
    }

    if (SystemConfig::GetInstance().IsVisionTypeBased2D() == true) //kircheis_SWIR
    {
        // Image Combine 은 2D 에서만 사용가능
        if (auto* ImageCombine = m_propertyGrid->AddCategory(_T("Image Combine")))
        {
            if (auto* Item = ImageCombine->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE)))
            {
                Item->SetID(ITEM_BUTTON_ID_IMAGECOMBINE);
            }

            ImageCombine->Expand();
        }
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Detail Setting")))
    {
        if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Edit Threshold & ROI"), TRUE, FALSE)))
        {
            Item->SetID(ITEM_BUTTON_ID_THRESHOLDANDROI);
        }

        if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Edit Criteria"), TRUE, FALSE)))
        {
            Item->SetID(ITEM_BUTTON_ID_CRITERIAEDITOR);
        }

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Surface Mask")))
    {
        if (m_pVisionPara->m_SurfaceItem.m_bSurfaceMaskMode)
        {
            if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Edit Mask"), TRUE, FALSE)))
            {
                Item->SetID(ITEM_BUTTON_ID_USERCUSTOMMASK);
            }

            if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Teach"), TRUE, FALSE)))
            {
                Item->SetID(ITEM_BUTTON_ID_USERCUSTOMMASK_TEACH);
            }

            if (auto* Item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Generate Mask Always"),
                    m_pVisionPara->m_SurfaceItem.m_bSurfaceMask_GenerateAlways,
                    &m_pVisionPara->m_SurfaceItem.m_bSurfaceMask_GenerateAlways)))
            {
                Item->SetID(ITEM_BUTTON_ID_GENERATEMASKALWAY);
            }
        }

        category->Expand();
    }

    m_propertyGrid->SetViewDivider(0.5);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionInspectionSurface::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().IsVisionTypeSide() == 1);
    const long nVisionModule = m_pVisionInsp->GetCurVisionModule_Status();

    CXTPPropertyGridItem* Item = (CXTPPropertyGridItem*)lparam;

    long nSelectIndex = Item->GetID();

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (nSelectIndex)
        {
            case ITEM_ALGORITHM_BASICPARAM_USE_VMAP_IMAGE:
            {
                if (bIsSideVision == false
                    || (bIsSideVision && nVisionModule == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
                {
                    m_imageLotView->ShowImage(m_pVisionPara->m_SurfaceItem.AlgoPara.nUse3DImageType);
                    m_imageLotView->ZoomPaneFit(true);
                }
                else if (bIsSideVision && nVisionModule == enSideVisionModule::SIDE_VISIONMODULE_REAR)
                {
                    m_imageLotViewRearSide->ShowImage(m_pVisionPara->m_SurfaceItem.AlgoPara.nUse3DImageType);
                    m_imageLotViewRearSide->ZoomPaneFit(true);
                }
            }
            break;
            case ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_UM:
                ConvertUm2Px(ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_UM);
                break;
            case ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_PX:
                ConvertPx2Um(ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_PX);
                break;
            case ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_UM:
                ConvertUm2Px(ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_UM);
                break;
            case ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_PX:
                ConvertPx2Um(ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_PX);
                break;
            case ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_UM:
                ConvertUm2Px(ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_UM);
                break;
            case ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_PX:
                ConvertPx2Um(ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_PX);
                break;
            case ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_UM:
                ConvertUm2Px(ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_UM);
                break;
            case ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_PX:
                ConvertPx2Um(ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_PX);
                break;
            case ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_UM:
                ConvertUm2Px(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_UM);
                break;
            case ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_PX:
                ConvertPx2Um(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_PX);
                break;
            case ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_UM:
                ConvertUm2Px(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_UM);
                break;
            case ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_PX:
                ConvertPx2Um(ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_PX);
                break;
            case ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_UM:
                ConvertUm2Px(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_UM);
                break;
            case ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_PX:
                ConvertPx2Um(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_PX);
                break;
            case ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_UM:
                ConvertUm2Px(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_UM);
                break;
            case ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_PX:
                ConvertPx2Um(ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_PX);
                break;

            case ITEM_BUTTON_ID_IMAGECOMBINE:
            {
                auto* proc = m_pVisionInsp;

                if (::Combine_SetParameter(
                        *proc, proc->GetImageFrameIndex(0), false, &proc->m_surfacePara->m_ImageProcMangePara)
                    == IDOK)
                {
                    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
                    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
                    if (image.GetMem() == nullptr)
                        return FALSE;

                    Ipvm::Image8u combineImage;
                    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
                        return FALSE;

                    if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false,
                            Ipvm::Rect32s(image), m_pVisionInsp->m_surfacePara->m_ImageProcMangePara, combineImage))
                    {
                        return FALSE;
                    }

                    CString FrameToString;

                    if (m_pVisionInsp->m_surfacePara->m_ImageProcMangePara.isCombine())
                    {
                        FrameToString.Format(_T("Combined Image **"));
                    }
                    else
                    {
                        FrameToString
                            = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
                    }

                    if (bIsSideVision == false
                        || (bIsSideVision && nVisionModule == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
                    {
                        m_imageLotView->SetImage(combineImage, FrameToString);
                    }
                    else if (bIsSideVision && nVisionModule == enSideVisionModule::SIDE_VISIONMODULE_REAR)
                    {
                        m_imageLotViewRearSide->SetImage(combineImage, FrameToString);
                    }

                    FrameToString.Empty();
                }

                break;
            }

            case ITEM_BUTTON_ID_THRESHOLDANDROI:
            {
                // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
                Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
                if (image.GetMem() == nullptr)
                    return FALSE;

                m_pVisionInsp->Surface_CreateMaskGroup(FALSE);

                m_nUsed3DImageType = m_pVisionPara->m_SurfaceItem.AlgoPara.nUse3DImageType; //SDY_3D Surface 수정

                CSurfaceROIEditorDlg DlgROIEditor(
                    m_pVisionInsp, FALSE, m_pVisionPara->m_SurfaceItem, 0, m_nUsed3DImageType, this);

                if (DlgROIEditor.DoModal() == IDOK)
                    m_pVisionInsp->SetSurfaceAlgoPara(DlgROIEditor.GetAlgoPara());

                break;
            }
            case ITEM_BUTTON_ID_CRITERIAEDITOR:
            {
                m_nUsed3DImageType
                    = m_pVisionPara->m_SurfaceItem.AlgoPara.nUse3DImageType; //SDY_3D Surface VMap UI 오류 수정

                CSurfaceCriteriaDlg DlgCriteria(m_pVisionInsp, m_pVisionPara->m_SurfaceItem, 2 /*um*/, 0 // surface ID
                    ,
                    FALSE, m_nUsed3DImageType, FALSE); //SDY_3D Surface VMap UI 오류 수정

                if (DlgCriteria.DoModal() == IDOK)
                    m_pVisionInsp->SetSurfaceItem(DlgCriteria.GetSurfaceItem());

                break;
            }

            case ITEM_BUTTON_ID_USERCUSTOMMASK:
            {
                Ipvm::Image8u image;
                if (m_pVisionInsp->GetDisplayAlignImage(m_pVisionPara->m_SurfaceItem, false, image))
                {
                    auto& SurfaceBitmapMask = m_pVisionPara->m_SurfaceBitmapMask;
                    CSurfaceUserCustomMaskDlg dlg(SurfaceBitmapMask, image, m_pVisionInsp);

                    if (dlg.DoModal() == IDOK)
                    {
                        SurfaceBitmapMask = *dlg.m_pEditSurfaceBitmapMask;
                    }
                }
                break;
            }

            case ITEM_BUTTON_ID_USERCUSTOMMASK_TEACH:
                m_pVisionInsp->DoTeach();
                break;

            default:
                break;
        }

        m_propertyGrid->Refresh();
    }
    return 0;
}

void CDlgVisionInspectionSurface::ConvertUm2Px(long i_nSelectIndex)
{
    if (i_nSelectIndex < 0) //음수값이면 쓰레기값
        return;

    double fUmTemp(0.f), fPxTemp(0.f);
    float fUm2Px = m_pVisionInsp->getScale().umToPixelXY();
    double dUm2Px = double(m_pVisionInsp->getScale().umToPixelXY());

    switch (i_nSelectIndex)
    {
        case ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_UM:
            fUmTemp = m_pVisionPara->m_SurfaceItem.AlgoPara.fMinAreaBrightContrast;
            fPxTemp = fUmTemp * pow(fUm2Px, 2);
            m_dMinAreaBrightContrast_Px = fPxTemp;
            break;

        case ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_UM:
            fUmTemp = m_pVisionPara->m_SurfaceItem.AlgoPara.fMinAreaDarkContrast;
            fPxTemp = fUmTemp * pow(fUm2Px, 2);
            m_dMinAreaDarkContrast_Px = fPxTemp;
            break;

        case ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_UM:
            fUmTemp = m_pVisionPara->m_SurfaceItem.AlgoPara.m_mergeDistance;
            fPxTemp = fUmTemp * fUm2Px;
            m_dMergeDistance_Px = fPxTemp;
            break;

        case ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_UM:
            fUmTemp = m_pVisionPara->m_SurfaceItem.AlgoPara.fLocalContrastAreaSpan;
            fPxTemp = fUmTemp * fUm2Px;
            m_dfLocalContrastAreaSpan_Px = fPxTemp;
            break;

        case ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_UM:
            fUmTemp = m_pVisionPara->m_SurfaceItem.AlgoPara.fMinimumBrightBlobArea_umSqure;
            fPxTemp = fUmTemp * fUm2Px * fUm2Px;
            m_dMinimumBrightBlobArea_Px = (double)((long)(fPxTemp + .5f));
            break;

        case ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_UM:
            fUmTemp = m_pVisionPara->m_SurfaceItem.AlgoPara.fMininumDarkBlobArea_umSqure;
            fPxTemp = fUmTemp * fUm2Px * fUm2Px;
            m_dMininumDarkBlobArea_Px = (double)((long)(fPxTemp + .5f));
            break;

        case ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_UM:
            fUmTemp = m_pVisionPara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_x;
            fPxTemp = (double)((long)((fUmTemp * dUm2Px) + .5f));
            m_dbackground_window_size_x_Px = fPxTemp;
            break;

        case ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_UM:
            fUmTemp = m_pVisionPara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_y;
            fPxTemp = (double)((long)((fUmTemp * dUm2Px) + .5f));
            m_dbackground_window_size_y_Px = fPxTemp;
            break;
    }
}

void CDlgVisionInspectionSurface::ConvertPx2Um(long i_nSelectIndex)
{
    if (i_nSelectIndex < 0) //음수값이면 쓰레기값
        return;

    double fUmTemp(0.f), fPxTemp(0.f);
    float fPx2Um = m_pVisionInsp->getScale().pixelToUmXY();
    float fUm2Px = m_pVisionInsp->getScale().umToPixelXY();
    double dPx2Um = double(m_pVisionInsp->getScale().pixelToUmXY());

    switch (i_nSelectIndex)
    {
        case ITEM_ALGORITHM_BASICPARAM_MINIMUM_BRIGHTCONTRAST_PX:
            fPxTemp = m_dMinAreaBrightContrast_Px;
            fUmTemp = fPxTemp / pow(fUm2Px, 2);
            m_pVisionPara->m_SurfaceItem.AlgoPara.fMinAreaBrightContrast = fUmTemp;
            break;

        case ITEM_ALGORITHM_BASICPARAM_MINIMUM_DARKCONTRAST_PX:
            fPxTemp = m_dMinAreaDarkContrast_Px;
            fUmTemp = fPxTemp / pow(fUm2Px, 2);
            m_pVisionPara->m_SurfaceItem.AlgoPara.fMinAreaDarkContrast = fUmTemp;
            break;

        case ITEM_ALGORITHM_BASICPARAM_MERGEDSITANCE_PX:
            fPxTemp = m_dMergeDistance_Px;
            fUmTemp = fPxTemp * fPx2Um;
            m_pVisionPara->m_SurfaceItem.AlgoPara.m_mergeDistance = fUmTemp;
            break;

        case ITEM_ALGORITHM_BASICPARAM_LOCAL_CONTRAST_AREASPAN_PX:
            fPxTemp = m_dfLocalContrastAreaSpan_Px;
            fUmTemp = fPxTemp * fPx2Um;
            m_pVisionPara->m_SurfaceItem.AlgoPara.fLocalContrastAreaSpan = fUmTemp;
            break;

        case ITEM_ALGORITHM_BLOBPARAM_MINIMUM_BRIGHTBLOBAREA_PX:
            fPxTemp = m_dMinimumBrightBlobArea_Px;
            fUmTemp = fPxTemp / (fUm2Px * fUm2Px);
            m_pVisionPara->m_SurfaceItem.AlgoPara.fMinimumBrightBlobArea_umSqure = fUmTemp;
            break;

        case ITEM_ALGORITHM_BLOBPARAM_MINIMUM_DARKBLOBAREA_PX:
            fPxTemp = m_dMininumDarkBlobArea_Px;
            fUmTemp = fPxTemp / (fUm2Px * fUm2Px);
            m_pVisionPara->m_SurfaceItem.AlgoPara.fMininumDarkBlobArea_umSqure = fUmTemp;
            break;
        case ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEX_PX:
            fPxTemp = m_dbackground_window_size_x_Px;
            fUmTemp = fPxTemp * dPx2Um;
            m_pVisionPara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_x = fUmTemp;
            break;
        case ITEM_ALGOROTHM_BACKGROUND_WINDOW_SIZEY_PX:
            fPxTemp = m_dbackground_window_size_y_Px;
            fUmTemp = fPxTemp * dPx2Um;
            m_pVisionPara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_y = fUmTemp;
            break;
    }
}

void CDlgVisionInspectionSurface::OnBnClickedButtonInspect()
{
    const bool bIsSideVision = SystemConfig::GetInstance().IsVisionTypeSide();
    const long nCurSideSelection = m_pVisionInsp->GetCurVisionModule_Status();

    // 1. VisionType이 Side Vision이 아닐 경우
    // 2. VisionType이 Side Vision이면서 Side Front일 경우
    // --> 기존 m_imageLotView를 사용
    if (bIsSideVision == false || (bIsSideVision && nCurSideSelection == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
    {
        // Spec Update
        m_imageLotView->Overlay_RemoveAll();

        m_procCommonLogDlg->SetLogBoxText(_T(""));

        auto& visionUnit = m_pVisionInsp->m_visionUnit;
        visionUnit.RunInspectionInTeachMode(m_pVisionInsp, true, m_pVisionInsp->GetCurVisionModule_Status());

        m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());

        // 검사 중간 결과 화면 갱신
        m_procCommonDebugInfoDlg->Refresh();

        // 검사 결과 화면 갱신
        m_procCommonResultDlg->Refresh();

        VisionInspectionOverlayResult overlayResult;
        m_pVisionInsp->GetOverlayResult(&overlayResult, true);
        overlayResult.Apply(m_imageLotView->GetCoreView());

        COLORREF color;

        if (m_pVisionInsp->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
        {
            Ipvm::Rect32s imageRoi = Ipvm::Rect32s(0, 0, m_pVisionInsp->getReusableMemory().GetInspImageSizeX(),
                m_pVisionInsp->getReusableMemory().GetInspImageSizeY());

            color = RGB(0, 255, 0);

            CString str;
            str.Format(_T("Complete"));

            m_imageLotView->Overlay_AddRectangle(imageRoi, color);
            m_imageLotView->Overlay_AddText(Ipvm::Point32s2(imageRoi.m_left, imageRoi.m_bottom - 30), str, color, 10);

            str.Empty();
        }

        m_imageLotView->Overlay_Show(TRUE);

        if (m_pVisionInsp->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode == FALSE)
            ShowImage(TRUE);
    }
    else
    {
        // Spec Update
        m_imageLotViewRearSide->Overlay_RemoveAll();

        m_procCommonLogDlgRear->SetLogBoxText(_T(""));

        auto& visionUnit = m_pVisionInsp->m_visionUnit;
        visionUnit.RunInspectionInTeachMode(m_pVisionInsp, true, m_pVisionInsp->GetCurVisionModule_Status());

        m_procCommonLogDlgRear->SetLogBoxText(visionUnit.GetLastInspection_Text());

        // 검사 중간 결과 화면 갱신
        m_procCommonDebugInfoDlgRear->Refresh();

        // 검사 결과 화면 갱신
        m_procCommonResultDlgRear->Refresh();

        VisionInspectionOverlayResult overlayResult;
        m_pVisionInsp->GetOverlayResult(&overlayResult, true);
        overlayResult.Apply(m_imageLotViewRearSide->GetCoreView());

        COLORREF color;

        if (m_pVisionInsp->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
        {
            Ipvm::Rect32s imageRoi = Ipvm::Rect32s(0, 0, m_pVisionInsp->getReusableMemory().GetInspImageSizeX(),
                m_pVisionInsp->getReusableMemory().GetInspImageSizeY());

            color = RGB(0, 255, 0);

            CString str;
            str.Format(_T("Complete"));

            m_imageLotViewRearSide->Overlay_AddRectangle(imageRoi, color);
            m_imageLotViewRearSide->Overlay_AddText(
                Ipvm::Point32s2(imageRoi.m_left, imageRoi.m_bottom - 30), str, color, 10);

            str.Empty();
        }

        m_imageLotViewRearSide->Overlay_Show(TRUE);

        if (m_pVisionInsp->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode == FALSE)
            ShowImage(TRUE);
    }

    // 검사 결과 탭 보여주기
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);

    //// Spec Update
    //m_imageLotView->Overlay_RemoveAll();

    //m_procCommonLogDlg->SetLogBoxText(_T(""));

    //auto& visionUnit = m_pVisionInsp->m_visionUnit;
    //visionUnit.RunInspectionInTeachMode(m_pVisionInsp, true, m_pVisionInsp->GetCurVisionModule_Status());

    //m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());

    //// 검사 중간 결과 화면 갱신
    //m_procCommonDebugInfoDlg->Refresh();

    //// 검사 결과 화면 갱신
    //m_procCommonResultDlg->Refresh();

    //// 검사 결과 탭 보여주기
    //m_TabResult.SetCurSel(1);
    //OnTcnSelchangeTabResult(nullptr, nullptr);

    //VisionInspectionOverlayResult overlayResult;
    //m_pVisionInsp->GetOverlayResult(&overlayResult, true);
    //overlayResult.Apply(m_imageLotView->GetCoreView());

    //COLORREF color;

    //if(m_pVisionInsp->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
    //{
    //	Ipvm::Rect32s imageRoi = Ipvm::Rect32s(0, 0,
    //		m_pVisionInsp->getReusableMemory().GetInspImageSizeX(),
    //		m_pVisionInsp->getReusableMemory().GetInspImageSizeY());

    //	color = RGB(0, 255, 0);

    //	CString str;
    //	str.Format(_T("Complete"));

    //	m_imageLotView->Overlay_AddRectangle(imageRoi, color);
    //	m_imageLotView->Overlay_AddText(Ipvm::Point32s2(imageRoi.m_left, imageRoi.m_bottom-30), str, color, 10);

    //	str.Empty();
    //}

    //m_imageLotView->Overlay_Show(TRUE);

    //if(m_pVisionInsp->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode == FALSE)
    //	ShowImage(TRUE);
}

void CDlgVisionInspectionSurface::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    m_nCurCommonTab = m_TabResult.GetCurSel();

    /*m_procCommonSpecDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
	m_procCommonResultDlg->ShowWindow((nTab == 1) ? SW_SHOW : SW_HIDE);
	m_procCommonDetailResultDlg->ShowWindow((nTab == 2) ? SW_SHOW : SW_HIDE);
	m_procCommonDebugInfoDlg->ShowWindow(nTab == 3 ? SW_SHOW : SW_HIDE);
	m_procCommonLogDlg->ShowWindow(nTab == 4 ? SW_SHOW : SW_HIDE);*/

    UpdateCommonTabShow();

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT CDlgVisionInspectionSurface::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
{
    const long itemIndex = long(wparam);

    /*if (itemIndex < 0)
	{
		m_TabResult.SetCurSel(1);
	}
	else
	{
		m_TabResult.SetCurSel(2);

		m_procCommonDetailResultDlg->Refresh(itemIndex);
	}*/

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
        {
            m_procCommonDetailResultDlgRear->Refresh(itemIndex);
        }
        else
        {
            m_procCommonDetailResultDlg->Refresh(itemIndex);
        }
    }

    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0L;
}

void CDlgVisionInspectionSurface::ShowImage(BOOL bChange)
{
    long nVisionType = SystemConfig::GetInstance().GetVisionType();
    bool bIsSideVision = SystemConfig::GetInstance().IsVisionTypeSide();
    bool bIsSideFront
        = m_pVisionInsp->GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_FRONT ? true : false;

    if (nVisionType == VISIONTYPE_3D_INSP)
    {
        // 3D Frame 보여주기
        //m_imageLotView->ShowImage(0);
        //m_imageLotView->ZoomImageFit();
        m_imageLotView->ShowImage(m_pVisionPara->m_SurfaceItem.AlgoPara.nUse3DImageType);
        m_imageLotView->ZoomPaneFit(true);
        return;
    }

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

    if ((bIsSideVision == false && nVisionType != VISIONTYPE_3D_INSP) || (bIsSideVision && bIsSideFront))
    {
        m_imageLotView->ShowImage(m_nImageID);
        m_imageLotView->ZoomImageFit();
    }
    else if (bIsSideVision && bIsSideFront == false)
    {
        m_imageLotViewRearSide->ShowImage(m_nImageID);
        m_imageLotViewRearSide->ZoomImageFit();
    }
}

LRESULT CDlgVisionInspectionSurface::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    if (m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetPreviousVisionProcessing(m_pVisionInsp), true,
            m_pVisionInsp->GetCurVisionModule_Status()))
    {
        ShowImage();
    }

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다

    const bool bIsSideVision = SystemConfig::GetInstance().IsVisionTypeSide();
    const long nCurSideSelection = m_pVisionInsp->GetCurVisionModule_Status();

    // 1. VisionType이 Side Vision이 아닐 경우
    // 2. VisionType이 Side Vision이면서 Side Front일 경우
    // --> 기존 m_imageLotView를 사용
    if (bIsSideVision == false || (bIsSideVision && nCurSideSelection == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
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

void CDlgVisionInspectionSurface::SetInitialSideVisionSelector()
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

void CDlgVisionInspectionSurface::OnStnClickedStaticSideFrontSurface()
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

    if (m_pVisionInsp->m_visionUnit.RunInspectionInTeachMode(
            m_pVisionInsp, true, m_pVisionInsp->GetCurVisionModule_Status()))
    {
        ShowImage();
    }

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    UpdateCommonTabShow();
}

void CDlgVisionInspectionSurface::OnStnClickedStaticSideRearSurface()
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

    if (m_pVisionInsp->m_visionUnit.RunInspectionInTeachMode(
            m_pVisionInsp, true, m_pVisionInsp->GetCurVisionModule_Status()))
    {
        ShowImage();
    }

    m_imageLotViewRearSide->Overlay_RemoveAll();
    m_imageLotViewRearSide->ROI_RemoveAll();

    m_imageLotViewRearSide->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotViewRearSide->ZoomImageFit();

    UpdateCommonTabShow();
}

void CDlgVisionInspectionSurface::UpdateCommonTabShow()
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
