//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionMark.h"

//CPP_2_________________________________ This project's headers
#include "DlgMarkAlgorithmParameter.h"
#include "DlgSelectEnableMarkMapData.h"
#include "DlgVisionInspectionMarkOperator.h"
#include "InspResult.h"
#include "VisionInspectionMark.h"
#include "VisionInspectionMarkPara.h"
#include "VisionInspectionMarkSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ImageViewEx/PatternImage.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>

//CPP_5_________________________________ Standard library headers
#include <fstream>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1012)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgVisionInspectionMark, CDialog)

CDlgVisionInspectionMark::CDlgVisionInspectionMark(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionMark* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionMark::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_imageLotView(nullptr)
    , m_propertyGrid(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_nDebugViewImageIndex(0)
    , m_bBtnSplitePush(FALSE)
{
    m_pVisionPara = m_pVisionInsp->m_VisionPara;
    m_pVisionSpec = m_pVisionInsp->m_VisionSpec[0];
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_ThresholdImage), 0, m_ThresholdImage);

    m_pVisionInspOpParaDlg = new CDlgVisionInspectionMarkOperator(pVisionInsp->getScale(), this);

    m_bShowResultDetail = FALSE;

    MakeMarkInfoPixel();

    auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;

    sReadMarkMapInfo sRecipeMarkInfo;
    sRecipeMarkInfo.vecTeachROI_Info.push_back(sMarkMapInfo_px.sTeachROI_Info);
    sRecipeMarkInfo.vecMergeROI_Info = sMarkMapInfo_px.vecMergeROI_Info;
    sRecipeMarkInfo.vecIgnoreROI_Info = sMarkMapInfo_px.vecIgnoreROI_Info;
    SetReadMapDataInfo_px(sRecipeMarkInfo);
}

CDlgVisionInspectionMark::~CDlgVisionInspectionMark()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    delete m_imageLotView;

    delete m_pVisionInspOpParaDlg;
    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionInspectionMark::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionMark, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionMark::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionMark::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspectionMark::OnBnClickedButtonClose)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &CDlgVisionInspectionMark::OnRoiChanged)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionMark::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionMark::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionMark::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    SetInitDialog();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspectionMark::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_pVisionInspOpParaDlg->Create(IDD_DIALOG_OPERATOR_PARAMETER, this);

    return 0;
}

void CDlgVisionInspectionMark::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();

    m_pVisionInspOpParaDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionMark::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionMark::SetInitDialog()
{
    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    //전체적인 Setup UI 설정(ImageWindow,  Spec, Result, Log etc..)
    SetInitSetupWindow();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();
}

void CDlgVisionInspectionMark::SetInitSetupWindow()
{
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
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_pVisionInspOpParaDlg->MoveWindow(m_procDlgInfo.m_rtParaArea);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);

    //Parameter UI 설정
    UpdatePropertyGrid();

    SetROI(m_pVisionPara->m_nROISettingMethod);
    GetROI();

    long nMarkTeachViewIndex(0);
    m_pVisionInsp->m_visionUnit.GetSwitchMarkTeachViewIndex(nMarkTeachViewIndex);
    SwitchParameterDialog(nMarkTeachViewIndex);
    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    if (SystemConfig::GetInstance().m_nCurrentAccessMode == _OPERATOR)
        m_procCommonSpecDlg->EnableWindow(FALSE);
    else
        m_procCommonSpecDlg->EnableWindow(TRUE);
}

BOOL CDlgVisionInspectionMark::SwitchParameterDialog(long i_nCurTabIndex)
{
    if (i_nCurTabIndex < 0)
        return FALSE;

    switch (i_nCurTabIndex)
    {
        case _OPERATOR:
            m_propertyGrid->ShowWindow(SW_HIDE);
            m_pVisionInspOpParaDlg->ShowWindow(SW_SHOW);
            m_pVisionInspOpParaDlg->SetRoiParameter();
            break;
        case _ENGINEER:
            m_pVisionInspOpParaDlg->ShowWindow(SW_HIDE);
            m_propertyGrid->ShowWindow(SW_SHOW);
            SetRoiParameter();
            break;
        default:
            break;
    }

    if (SystemConfig::GetInstance().m_nCurrentAccessMode
        == _OPERATOR) //현재 AccessMode가 Operator면 Property는 강제로 숨긴다
    {
        m_pVisionInspOpParaDlg->ShowWindow(SW_SHOW);
        m_propertyGrid->ShowWindow(SW_HIDE);
    }

    return TRUE;
}

void CDlgVisionInspectionMark::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("2D Mark Spec Setup Parameter")))
    {
        if (auto* DebugImage = m_propertyGrid->AddCategory(_T("Image Debug View")))
        {
            //Combo : Image Duebg
            if (auto* Item = DebugImage->AddChildItem(new CXTPPropertyGridItemEnum(
                    _T("Debug Image"), m_nDebugViewImageIndex, (int*)&m_nDebugViewImageIndex)))
            {
                for (long n = 0; n < DebugImage_End; n++)
                    Item->GetConstraints()->AddConstraint(g_szImageDebugModeName[n], n);

                Item->SetID(ITEM_ID_COMBO_DEBUGVIEW_IMAGE);
            }

            DebugImage->Expand();
        }

        if (auto* AlgoParam = m_propertyGrid->AddCategory(_T("Algorithm & Image")))
        {
            // Algorithm Btn
            if (auto* Item = AlgoParam->AddChildItem(new CCustomItemButton(_T("Algorithm Parameter"), TRUE, FALSE)))
                Item->SetID(ITEM_ID_BUTTON_ALGORITHM_PARAM);

            // ImageComBine Btn
            if (auto* Item = AlgoParam->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE)))
                Item->SetID(ITEM_ID_BUTTON_IMAGECOMBINE);

            AlgoParam->Expand();
        }

        if (auto* ROISettingParam = m_propertyGrid->AddCategory(_T("Reference Modity Tool")))
        {
            if (auto* Item = ROISettingParam->AddChildItem(new CXTPPropertyGridItemEnum(_T("ROI Setting Method"),
                    m_pVisionPara->m_nROISettingMethod, (int*)&m_pVisionPara->m_nROISettingMethod)))
            {
                Item->GetConstraints()->AddConstraint(_T("Manual"), ROISettingMethod_Manual);
                Item->GetConstraints()->AddConstraint(_T("Map Data"), ROISettingMethod_Mapdata);

                Item->SetID(ITEM_ID_SELECT_USE_MAP);
            }

            if (auto* Item
                = ROISettingParam->AddChildItem(new CCustomItemButton(_T("Import Mark Map Data"), TRUE, FALSE)))
                Item->SetID(ITEM_ID_MARK_ID_IMPORT_MAP);

            if (auto* Item
                = ROISettingParam->AddChildItem(new CCustomItemButton(_T("Export Mark Map Data"), TRUE, FALSE)))
                Item->SetID(ITEM_ID_MARK_ID_EXPORT_MAP);

            if (auto* Item
                = ROISettingParam->AddChildItem(new CCustomItemButton(_T("Select Enable Mark Map Data"), TRUE, FALSE)))
            {
                Item->SetID(ITEM_ID_MARK_ID_SELECT_ENABLE_MARK_MAPDATA);
                if (m_pVisionPara->m_nROISettingMethod != ROISettingMethod_Mapdata)
                    Item->SetReadOnly(TRUE);
            }

            if (auto* Item
                = ROISettingParam->AddChildItem(new CCustomItemButton(_T("Show Mark ROI (Map Data)"), TRUE, FALSE)))
                Item->SetID(ITEM_ID_MARK_ID_SHOW_MAP);

            //Char Merge box : Combo
            if (auto* Item = ROISettingParam->AddChildItem(new CXTPPropertyGridItemEnum(_T("Char Merge Box Num"),
                    m_pVisionPara->m_teach_merge_num, (int*)&m_pVisionPara->m_teach_merge_num)))
            {
                Item->GetConstraints()->AddConstraint(_T("None"), 0);
                Item->GetConstraints()->AddConstraint(_T("1"), 1);
                Item->GetConstraints()->AddConstraint(_T("2"), 2);
                Item->GetConstraints()->AddConstraint(_T("3"), 3);
                Item->GetConstraints()->AddConstraint(
                    _T("ROI Merge"), -1); //kircheis_고민/확인필요//왜 -1을 사용 했지?? 인자형은 DWORD_PTR인데

                Item->SetID(ITEM_ID_COMBO_CHARMERGE_ROINUM);
            }

            //Char ignore box : Combo
            if (auto* Item = ROISettingParam->AddChildItem(new CXTPPropertyGridItemEnum(
                    _T("Char Ignore Box Num"), m_pVisionPara->m_nIgnoreROINum, (int*)&m_pVisionPara->m_nIgnoreROINum)))
            {
                Item->GetConstraints()->AddConstraint(_T("None"), 0);
                Item->GetConstraints()->AddConstraint(_T("1"), 1);
                Item->GetConstraints()->AddConstraint(_T("2"), 2);
                Item->GetConstraints()->AddConstraint(_T("3"), 3);

                Item->SetID(ITEM_ID_COMBO_CHARIGNORE_ROINUM);
            }

            ROISettingParam->Expand();
        }

        if (auto* ModifyAndThreshold = m_propertyGrid->AddCategory(_T("Threshold & Modify Tool")))
        {
            if (auto* Item = ModifyAndThreshold->AddChildItem(new CXTPPropertyGridItemBool(
                    _T("Align Area Ignore"), m_pVisionPara->m_bAlignAreaIgnore, &m_pVisionPara->m_bAlignAreaIgnore)))
            {
                Item->SetID(ITEM_ID_MARK_ALIGNAREAIGNORE);
            }

            if (auto* Item
                = ModifyAndThreshold->AddChildItem(new CXTPPropertyGridItemNumber(_T("Ignore Area Dilate Count"),
                    m_pVisionPara->m_nIgnoreDilateCount, &m_pVisionPara->m_nIgnoreDilateCount)))
            {
                Item->SetID(ITEM_ID_MARK_IGNOREAREADILATECOUNT);
            }

            //Threshold : Slider Bar
            if (auto* Item = ModifyAndThreshold->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Threshold"), m_pVisionSpec->m_nMarkThreshold, &m_pVisionSpec->m_nMarkThreshold)))
            {
                if (auto* ThresholdSlider = Item->AddSliderControl())
                {
                    ThresholdSlider->SetMin(0);
                    ThresholdSlider->SetMax(256);
                }

                if (auto* SpinButton = Item->AddSpinButton())
                {
                    SpinButton->SetMin(0);
                    SpinButton->SetMax(256);
                }

                Item->SetID(ITEM_ID_SLIDER_THRESHOLD);
            }

            ModifyAndThreshold->Expand();
        }

        if (auto* DetailParam = m_propertyGrid->AddCategory(_T("Detail Setup")))
        {
            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Mark Angle"), m_pVisionPara->m_nMarkAngle_deg, &m_pVisionPara->m_nMarkAngle_deg)))
            {
                Item->SetID(ITEM_ID_MARK_ANGLE);
            }

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemEnum(
                    _T("Mark Inspection Mode"), m_pVisionPara->m_nMarkInspMode, (int*)&m_pVisionPara->m_nMarkInspMode)))
            {
                for (long n = 0; n < MarkInspectionMode_End; n++)
                    Item->GetConstraints()->AddConstraint(g_szMarkInspctionModeName[n], n);

                RefreshSpecCtrl(m_pVisionPara->m_nMarkInspMode);
                Item->SetID(ITEM_ID_MARK_INSPCTIONMODE);
            }

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemEnum(_T("Image Processing"),
                    m_pVisionPara->m_nMarkPreProcMode, (int*)&m_pVisionPara->m_nMarkPreProcMode)))
            {
                for (long n = 0; n < ImageProcessingMode_End; n++)
                    Item->GetConstraints()->AddConstraint(g_szImageProcessingModeName[n], n);

                Item->SetID(ITEM_ID_COMBO_IMAGE_PROCESSING_MODE);
            }

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemBool(
                    _T("Locator Target ROI"), m_pVisionPara->m_bLocatorTargetROI, &m_pVisionPara->m_bLocatorTargetROI)))
                Item->SetID(ITEM_ID_COMBO_LOCATOR_TARGETROI_USE);

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemBool(
                    _T("White Back Ground"), m_pVisionPara->m_bWhiteBackGround, &m_pVisionPara->m_bWhiteBackGround)))
                Item->SetID(ITEM_ID_COMBO_BLACK_IMAGE_USE);

            if (auto* Item
                = DetailParam->AddChildItem(new CXTPPropertyGridItemBool(_T("Select Char Insp Mode Use Blob"),
                    m_pVisionPara->m_bUseNormalSizeInspMode, &m_pVisionPara->m_bUseNormalSizeInspMode)))
                Item->SetID(ITEM_ID_COMBO_CHAR_INSP_MODE);

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemEnum(_T("Select Angle Insp Mode"),
                    m_pVisionPara->m_nSelectAngleReference, (int*)&m_pVisionPara->m_nSelectAngleReference)))
            {
                for (long n = 0; n < AngleInspMode_End; n++)
                    Item->GetConstraints()->AddConstraint(g_szAngleInspModeName[n], n);

                Item->SetID(ITEM_ID_COMBO_ANGLE_INSP_MODE);
            }

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemNumber(_T("Locator Search Offset X"),
                    m_pVisionPara->m_nLocSearchOffsetX, &m_pVisionPara->m_nLocSearchOffsetX)))
                Item->SetID(ITEM_ID_EDIT_LOCATOR_SEARCH_OFFSET_X);

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemNumber(_T("Locator Search Offset Y"),
                    m_pVisionPara->m_nLocSearchOffsetY, &m_pVisionPara->m_nLocSearchOffsetY)))
                Item->SetID(ITEM_ID_EDIT_LOCATOR_SEARCH_OFFSET_Y);

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemNumber(_T("Char Search Offset X"),
                    m_pVisionPara->m_nCharSearchOffsetX, &m_pVisionPara->m_nCharSearchOffsetX)))
                Item->SetID(ITEM_ID_EDIT_CHAR_SEARCH_OFFSET_X);

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemNumber(_T("Char Search Offset Y"),
                    m_pVisionPara->m_nCharSearchOffsetY, &m_pVisionPara->m_nCharSearchOffsetY)))
                Item->SetID(ITEM_ID_EDIT_CHAR_SEARCH_OFFSET_Y);

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Image Sampling"), m_pVisionPara->m_nImageSampleRate, &m_pVisionPara->m_nImageSampleRate)))
                Item->SetID(ITEM_ID_EDIT_IMAGE_SAMPLING);

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemEnum(
                    _T("Mark Direction"), m_pVisionPara->m_nMarkDirection, (int*)&m_pVisionPara->m_nMarkDirection)))
            {
                for (long n = 0; n < AngleInspMode_End; n++)
                    Item->GetConstraints()->AddConstraint(g_szMarkDirectionName[n], n);

                Item->SetID(ITEM_ID_COMBO_MARK_DIRECTION);
            }

            if (auto* Item = DetailParam->AddChildItem(new CXTPPropertyGridItemBool(_T("Inital Ignore ROI Use"),
                    m_pVisionPara->m_bInitialIgnoreBox, &m_pVisionPara->m_bInitialIgnoreBox)))
                Item->SetID(ITEM_ID_COMBO_INITAL_IGNOREROI_USE);

            DetailParam->Expand();
        }

        if (auto* TeachingTab = m_propertyGrid->AddCategory(_T("Mark Teaching")))
        {
            if (auto* Item = TeachingTab->AddChildItem(new CCustomItemButton(_T("Split Char Teach"), FALSE, FALSE)))
                Item->SetID(ITEM_ID_BUTTON_SPLIT_CHAR_TEACH);

            if (auto* Item = TeachingTab->AddChildItem(new CCustomItemButton(_T("Teach In ROI"), FALSE, FALSE)))
                Item->SetID(ITEM_ID_BUTTON_TEACH);

            TeachingTab->Expand();
        }

        if (auto* InspectionTest = m_propertyGrid->AddCategory(_T("Inspection Test")))
        {
            if (auto* Item = InspectionTest->AddChildItem(new CCustomItemButton(_T("Test Inspection"), TRUE, FALSE)))
                Item->SetID(ITEM_ID_BUTTON_INSPECTION_TEST);

            InspectionTest->Expand();
        }

        category->Expand();
    }

    SetRoiParameter();

    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->SetRedraw(TRUE);
}

LRESULT CDlgVisionInspectionMark::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* Item = (CXTPPropertyGridItem*)lparam;

    const auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (Item->GetID())
        {
            case ITEM_ID_COMBO_DEBUGVIEW_IMAGE:
            {
                Ipvm::Image8u RotateImage;
                PreImageViewSeq(RotateImage);

                PatternImage patternImage;
                patternImage.setImage(RotateImage);

                switch (m_nDebugViewImageIndex)
                {
                    case DebugImage_Origin:
                        patternImage.addPattern(*m_pVisionInsp->m_oriViewer, RGB(0, 255, 0));
                        break;
                    case DebugImage_Binary:
                        patternImage.addPattern(*m_pVisionInsp->m_binViewer, RGB(255, 255, 255));
                        break;
                    case DebugImage_Under:
                        patternImage.addPattern(*m_pVisionInsp->m_overViewer, RGB(0, 0, 255));
                        break;
                    case DebugImage_Over:
                        patternImage.addPattern(*m_pVisionInsp->m_underViewer, RGB(255, 0, 0));
                        break;
                    case DebugImage_Blob:
                        patternImage.addPattern(*m_pVisionInsp->m_blobSizeOverViewer, RGB(0, 0, 255));
                        patternImage.addPattern(*m_pVisionInsp->m_blobSizeUnderViewer, RGB(255, 0, 0));
                        break;
                }

                m_imageLotView->SetImage(patternImage);
                m_imageLotView->Overlay_Show(SW_SHOW);
                break;
            }
            case ITEM_ID_SLIDER_THRESHOLD:
                ShowCombineImage(FALSE);
                break;
            case ITEM_ID_COMBO_CHARMERGE_ROINUM:
                UpdateMergeROI();
                break;
            case ITEM_ID_COMBO_CHARIGNORE_ROINUM:
                UpdateIgnoreROI();
                break;
            case ITEM_ID_MARK_INSPCTIONMODE:
                RefreshSpecCtrl(m_pVisionPara->m_nMarkInspMode);
                m_pVisionInspOpParaDlg
                    ->InitializeBtnCtrl(); //Inspection Mode 변경하였다면 Button도 다시 Refresh되야 한다
                break;
            case ITEM_ID_MARK_ANGLE:
            {
                Ipvm::Image8u RotateImage;
                PreImageViewSeq(RotateImage);

                CString FrameToString
                    = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
                m_imageLotView->SetImage(RotateImage, FrameToString);

                FrameToString.Empty();

                break;
            }
            case ITEM_ID_MARK_ALIGNAREAIGNORE:
            {
                BOOL bReadOnly = m_pVisionPara->m_bAlignAreaIgnore == TRUE ? FALSE : TRUE;
                m_propertyGrid->FindItem(ITEM_ID_MARK_IGNOREAREADILATECOUNT)->SetReadOnly(bReadOnly);
                ShowCombineImage(m_pVisionPara->sMarkAlgoParam.nThresholdMode);
                break;
            }
            case ITEM_ID_MARK_IGNOREAREADILATECOUNT:
                ShowCombineImage(m_pVisionPara->sMarkAlgoParam.nThresholdMode);
                break;
            case ITEM_ID_COMBO_BLACK_IMAGE_USE:
                ShowCombineImage(m_pVisionPara->sMarkAlgoParam.nThresholdMode);
                break;
            case ITEM_ID_BUTTON_ALGORITHM_PARAM:
            {
                CDlgMarkAlgorithmParameter DlgMarkAlgorithmParameter(m_pVisionPara);
                DlgMarkAlgorithmParameter.DoModal();
                break;
            }
            case ITEM_ID_BUTTON_IMAGECOMBINE:
            {
                auto* proc = m_pVisionInsp;

                if (::Combine_SetParameter(
                        *proc, proc->GetImageFrameIndex(0), false, &proc->m_VisionPara->m_ImageProcMangePara)
                    == IDOK)
                {
                    ShowCombineImage(FALSE);
                }
                break;
            }
            case ITEM_ID_BUTTON_SPLIT_CHAR_TEACH:
            {
                CharSplitTeach();
                break;
            }
            case ITEM_ID_BUTTON_TEACH:
            {
                MarkTeach();
                break;
            }
            case ITEM_ID_BUTTON_INSPECTION_TEST:
            {
                InspectionTest();
                break;
            }
            case ITEM_ID_COMBO_LOCATOR_TARGETROI_USE:
                SetROI();
                break;
            case ITEM_ID_SELECT_USE_MAP:
            {
                SetRoiParameter();

                RefreshMarkDataInfo();

                if (m_pVisionPara->m_nROISettingMethod != ROISettingMethod_Manual)
                {
                    m_pVisionPara->m_nIgnoreROINum = (long)sMarkMapInfo_px.vecIgnoreROI_Info.size();
                    m_pVisionPara->m_teach_merge_num = (long)sMarkMapInfo_px.vecMergeROI_Info.size();

                    RefreshData();
                }

                if (m_pVisionPara->m_nROISettingMethod == ROISettingMethod_Manual)
                    m_propertyGrid->FindItem(ITEM_ID_MARK_ID_SELECT_ENABLE_MARK_MAPDATA)->SetReadOnly(TRUE);
                else if (m_pVisionPara->m_nROISettingMethod == ROISettingMethod_Mapdata)
                    m_propertyGrid->FindItem(ITEM_ID_MARK_ID_SELECT_ENABLE_MARK_MAPDATA)->SetReadOnly(FALSE);

                break;
            }
            case ITEM_ID_MARK_ID_IMPORT_MAP:
            {
                ClickedButtonImportMarkROI_MapData();
                DrawMarkMapROI(m_bBtnSplitePush);
                break;
            }
            case ITEM_ID_MARK_ID_EXPORT_MAP:
            {
                ClickedButtonExportMarkROI_MapData();
                break;
            }
            case ITEM_ID_MARK_ID_SELECT_ENABLE_MARK_MAPDATA:
            {
                ClickButtonSelectEnableMarkMapData(false);
                break;
            }
            case ITEM_ID_MARK_ID_SHOW_MAP:
            {
                DrawMarkMapROI(m_bBtnSplitePush);
                break;
            }
            default:
                break;
        }

        m_pVisionInspOpParaDlg->RefreshData();
    }
    else if (wparam == XTP_PGN_SELECTION_CHANGED)
    {
        switch (Item->GetID())
        {
            case ITEM_ID_SLIDER_THRESHOLD:
                if (auto* Control = Item->GetInplaceControls()->GetAt(0))
                {
                    auto* Slider = dynamic_cast<CXTPPropertyGridInplaceSlider*>(Control);

                    if (Slider)
                    {
                        Slider->GetSliderCtrl()->SetPageSize(8);
                    }
                }
                ShowCombineImage(FALSE);
                break;
            default:
                break;
        }

        m_pVisionInspOpParaDlg->RefreshData();
    }

    return 0;
}

BOOL CDlgVisionInspectionMark::PreImageViewSeq(Ipvm::Image8u& o_DebugViewImage)
{
    m_pVisionInsp->m_bodyAlignResult = m_pVisionInsp->GetAlignInfo();
    if (m_pVisionInsp->m_bodyAlignResult == nullptr)
        return FALSE;

    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return FALSE;

    const auto& scale = m_pVisionInsp->getScale();
    const auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;

    auto centerPoint = m_pVisionInsp->m_bodyAlignResult->m_center;
    float fBodyangle_deg = m_pVisionInsp->m_bodyAlignResult->m_angle_rad * (float)ITP_RAD_TO_DEG;
    float fMarkParaAngle_deg = (float)m_pVisionInsp->m_VisionPara->m_nMarkAngle_deg;
    float imageAngle_deg = fBodyangle_deg + fMarkParaAngle_deg;

    Ipvm::Rect32s rtInspROI_MapData;
    Ipvm::Point32r2 ptCenter;
    float fAngle_deg = 0.f;

    if (m_pVisionPara->m_nROISettingMethod != ROISettingMethod_Manual)
    {
        rtInspROI_MapData = scale.convert_BCUToPixel(m_pVisionSpec->m_rtMarkTeachROI_BCU, centerPoint);
        ptCenter.Set(sMarkMapInfo_px.sTeachROI_Info.fOffsetX, sMarkMapInfo_px.sTeachROI_Info.fOffsetY);
        fAngle_deg = -sMarkMapInfo_px.sTeachROI_Info.fAngle;
    }
    else
    {
        rtInspROI_MapData = Ipvm::Rect32s(image);
        ptCenter = centerPoint;
        fAngle_deg = imageAngle_deg;
    }

    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(o_DebugViewImage))
    {
        return FALSE;
    }

    /// 검사를 한번 하고 들어가자.
    m_pVisionInsp->DoInspection(true);

    /// Image Copy
    Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), o_DebugViewImage);

    /// Image Rotate
    Ipvm::ImageProcessing::RotateLinearInterpolation(
        image, rtInspROI_MapData, ptCenter, fAngle_deg, Ipvm::Point32r2(0.f, 0.f), o_DebugViewImage);

    return TRUE;
}

void CDlgVisionInspectionMark::RefreshSpecCtrl(long i_nInspectionType)
{
    m_procCommonSpecDlg->ResetInitGrid();

    //Disable 시킬항목 정한다.
    switch (i_nInspectionType)
    {
        case Mark_InspMode_Normal:
            m_procCommonSpecDlg->SelectDisableCoulmn(MARK_INSPECTION_MARK_COUNT);
            break;
        case Mark_InspMode_Simple:
            for (long nDisableIndex = MARK_INSPECTION_MARK_POS; nDisableIndex < MARK_INSPECTION_END; nDisableIndex++)
                m_procCommonSpecDlg->SelectDisableCoulmn(nDisableIndex);
            break;
        default:
            break;
    }
}

void CDlgVisionInspectionMark::OnBnClickedButtonInspect()
{
    // Spec Update
    //m_pVisionInspSpecDlg->UpdateSpecData();
    m_imageLotView->Overlay_RemoveAll();

    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    CString FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
    m_imageLotView->SetImage(image, FrameToString);

    FrameToString.Empty();

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();

    // 검사 결과 화면 갱신
    m_procCommonResultDlg->Refresh();

    // 검사 결과 탭 보여주기
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
}

void CDlgVisionInspectionMark::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDlgVisionInspectionMark::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionMark::ShowImage(BOOL bAutoThreshold, long nMultiID)
{
    UNREFERENCED_PARAMETER(nMultiID);

    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();

    if (image.GetMem() == nullptr)
        return;

    if (SystemConfig::GetInstance().m_nCurrentAccessMode == _OPERATOR)
    {
        m_pVisionInspOpParaDlg->ShowCombineImage(bAutoThreshold);
    }
    else
    {
        ShowCombineImage(bAutoThreshold);
    }

    SetROI(m_pVisionPara->m_nROISettingMethod);
}

LRESULT CDlgVisionInspectionMark::OnRoiChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    long nMarkTeachViewIndex(0);
    m_pVisionInsp->m_visionUnit.GetSwitchMarkTeachViewIndex(nMarkTeachViewIndex);

    if (SystemConfig::GetInstance().m_nCurrentAccessMode == _OPERATOR || nMarkTeachViewIndex == _OPERATOR)
    {
        m_pVisionInspOpParaDlg->GetROI(m_pVisionPara->m_nROISettingMethod);

        BOOL bAutoThreshold = FALSE;
        if (m_pVisionInsp->m_VisionPara->sMarkAlgoParam.nThresholdMode != ENUM_THRESHOLD_MANUAL)
        {
            bAutoThreshold = TRUE;
        }

        m_pVisionInspOpParaDlg->ShowCombineImage(bAutoThreshold);
    }
    else
    {
        GetROI(m_pVisionPara->m_nROISettingMethod);

        BOOL bAutoThreshold = FALSE;
        if (m_pVisionInsp->m_VisionPara->sMarkAlgoParam.nThresholdMode != ENUM_THRESHOLD_MANUAL)
            bAutoThreshold = TRUE;

        ShowCombineImage(bAutoThreshold);
    }

    return 0;
}

LRESULT CDlgVisionInspectionMark::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

void CDlgVisionInspectionMark::GetROI()
{
    const int nSizeX = m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const int nSizeY = m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Rect32s imageRoi(0, 0, nSizeX, nSizeY);

    const auto& scale = m_pVisionInsp->getScale();
    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    /// Mark Teach ROI
    if (true)
    {
        Ipvm::Rect32s roi;
        m_imageLotView->ROI_Get(_T("Mark Teach ROI"), roi);

        m_pVisionSpec->m_rtMarkTeachROI_BCU = scale.convert_PixelToBCU(roi, imageCenter);
    }

    /// Merge ROI
    long nMergeNum = (long)m_pVisionPara->m_teach_merge_infos.size();
    for (long nIndex = 0; nIndex < nMergeNum; nIndex++)
    {
        CString key;
        key.Format(_T("M%d"), nIndex);

        auto& roi = m_pVisionPara->m_teach_merge_infos[nIndex].m_roi;
        m_imageLotView->ROI_Get(key, roi);
        roi = roi - Ipvm::Conversion::ToPoint32s2(imageCenter);

        key.Empty();
    }

    /// Ignore ROI
    long nIgnoreNum = (long)m_pVisionPara->m_vecrtUserIgnore.size();
    for (long nIndex = 0; nIndex < nIgnoreNum; nIndex++)
    {
        CString key;
        key.Format(_T("I%d"), nIndex);

        m_imageLotView->ROI_Get(key, m_pVisionPara->m_vecrtUserIgnore[nIndex]);
        m_pVisionPara->m_vecrtUserIgnore[nIndex]
            = m_pVisionPara->m_vecrtUserIgnore[nIndex] - Ipvm::Conversion::ToPoint32s2(imageCenter);

        key.Empty();
    }

    /// Locater Target ROI
    if (m_pVisionPara->m_bLocatorTargetROI)
    {
        m_imageLotView->ROI_Get(_T("L1"), m_pVisionSpec->m_rtLocatorTargetROI[0]);
        m_pVisionSpec->m_rtLocatorTargetROI[0]
            = m_pVisionSpec->m_rtLocatorTargetROI[0] - Ipvm::Conversion::ToPoint32s2(imageCenter);

        m_imageLotView->ROI_Get(_T("L2"), m_pVisionSpec->m_rtLocatorTargetROI[1]);
        m_pVisionSpec->m_rtLocatorTargetROI[1]
            = m_pVisionSpec->m_rtLocatorTargetROI[1] - Ipvm::Conversion::ToPoint32s2(imageCenter);
    }

    RefreshMarkDataInfo();
}

void CDlgVisionInspectionMark::SetROI_MapData()
{
    const long nSizeX = m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const long nSizeY = m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    const auto& scale = m_pVisionInsp->getScale();
    const auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;
    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    /// Mark Teach ROI
    if (true)
    {
        FPI_RECT frtTemp;
        MakeMarkROI(sMarkMapInfo_px.sTeachROI_Info, frtTemp);

        m_pVisionSpec->m_rtMarkTeachROI_BCU = scale.convert_PixelToBCU(frtTemp.GetCRect(), imageCenter);
    }

    /// Merge ROI
    long nMergeNum = (long)sMarkMapInfo_px.vecMergeROI_Info.size();
    m_pVisionPara->m_teach_merge_infos.clear();
    m_pVisionPara->m_teach_merge_infos.resize(nMergeNum);
    for (long nIndex = 0; nIndex < nMergeNum; nIndex++)
    {
        FPI_RECT frtTemp;
        MakeMarkROI(sMarkMapInfo_px.vecMergeROI_Info[nIndex], frtTemp);

        m_pVisionPara->m_teach_merge_infos[nIndex].m_roi
            = frtTemp.GetCRect() - Ipvm::Conversion::ToPoint32s2(imageCenter);
    }

    /// Ignore ROI
    long nIgnoreNum = (long)sMarkMapInfo_px.vecIgnoreROI_Info.size();
    m_pVisionPara->m_vecrtUserIgnore.clear();
    m_pVisionPara->m_vecrtUserIgnore.resize(nIgnoreNum);
    for (long nIndex = 0; nIndex < nIgnoreNum; nIndex++)
    {
        FPI_RECT frtTemp;
        MakeMarkROI(sMarkMapInfo_px.vecIgnoreROI_Info[nIndex], frtTemp);

        m_pVisionPara->m_vecrtUserIgnore[nIndex] = frtTemp.GetCRect() - Ipvm::Conversion::ToPoint32s2(imageCenter);
    }

    ///// Locater Target ROI
    //if (m_pVisionPara->m_bLocatorTargetROI)
    //{
    //	m_imageLotView->ROI_Get(_T("L1"), m_pVisionSpec->m_rtLocatorTargetROI[0]);
    //	m_pVisionSpec->m_rtLocatorTargetROI[0] = m_pVisionSpec->m_rtLocatorTargetROI[0] - imageCenter;

    //	m_imageLotView->ROI_Get(_T("L2"), m_pVisionSpec->m_rtLocatorTargetROI[1]);
    //	m_pVisionSpec->m_rtLocatorTargetROI[1] = m_pVisionSpec->m_rtLocatorTargetROI[1] - imageCenter;
    //}
}

void CDlgVisionInspectionMark::SetROI()
{
    const int nSizeX = m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const int nSizeY = m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Rect32s imageRoi(0, 0, nSizeX, nSizeY);
    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    m_pVisionInsp->m_bAutoTeachMode
        = FALSE; //kircheis_AutoTeach // 여기에 왔다는건 AutoTeach가 아니라는거다.. 혹시 몰라 초기화
    m_imageLotView->ROI_RemoveAll();

    /// Mark Teach ROI
    const auto& scale = m_pVisionInsp->getScale();
    Ipvm::Rect32s rtROI = scale.convert_BCUToPixel(m_pVisionSpec->m_rtMarkTeachROI_BCU, imageCenter);
    rtROI &= imageRoi;
    m_imageLotView->ROI_Add(_T("Mark Teach ROI"), _T("Mark Teach ROI"), rtROI, RGB(0, 255, 0), TRUE, TRUE);

    //// Merge ROI 추가.
    long nMergeNum = (long)m_pVisionPara->m_teach_merge_infos.size();
    for (long nIndex = 0; nIndex < nMergeNum; nIndex++)
    {
        CString key;
        key.Format(_T("M%d"), nIndex);

        auto& roi = m_pVisionPara->m_teach_merge_infos[nIndex].m_roi;

        CString strName;
        strName.Format(_T("Mark Merge_%d"), nIndex);
        Ipvm::Rect32s rtMergeROI(roi + Ipvm::Conversion::ToPoint32s2(imageCenter));
        m_imageLotView->ROI_Add(key, strName, rtMergeROI, RGB(0, 0, 255), TRUE, TRUE);

        key.Empty();
        strName.Empty();
    }

    //// Ignore ROI 추가.
    long nIgnoreNum = (long)m_pVisionPara->m_vecrtUserIgnore.size();
    for (long nIndex = 0; nIndex < nIgnoreNum; nIndex++)
    {
        CString key;
        key.Format(_T("I%d"), nIndex);

        CString strName;
        strName.Format(_T("Mark Ignore_%d"), nIndex);
        Ipvm::Rect32s rtIgnoreROI(
            m_pVisionPara->m_vecrtUserIgnore[nIndex] + Ipvm::Conversion::ToPoint32s2(imageCenter));
        m_imageLotView->ROI_Add(key, strName, rtIgnoreROI, RGB(255, 0, 0), TRUE, TRUE);

        key.Empty();
        strName.Empty();
    }

    //// Locater ROI 추가.
    if (m_pVisionPara->m_bLocatorTargetROI)
    {
        Ipvm::Rect32s LocatorTargetROI
            = m_pVisionSpec->m_rtLocatorTargetROI[0] + Ipvm::Conversion::ToPoint32s2(imageCenter);
        LocatorTargetROI &= imageRoi;
        m_imageLotView->ROI_Add(_T("L1"), _T("Locator Target 1"), LocatorTargetROI, RGB(0, 255, 0), TRUE, TRUE);

        LocatorTargetROI = m_pVisionSpec->m_rtLocatorTargetROI[1] + Ipvm::Conversion::ToPoint32s2(imageCenter);
        LocatorTargetROI &= imageRoi;
        m_imageLotView->ROI_Add(_T("L2"), _T("Locator Target 2"), LocatorTargetROI, RGB(0, 255, 0), TRUE, TRUE);
    }

    /// splite ROI
    if (m_bBtnSplitePush)
    {
        /// Teching이 안되어 있으면 아무짓도 하지 말자.
        long nCharNum = m_pVisionInsp->m_VisionTempSpec->m_nCharNum;
        std::vector<Ipvm::Rect32s>& vecrtCharPosition = m_pVisionInsp->m_result->m_vecrtCharPositionforCalcImage;
        if (nCharNum <= 0 || nCharNum != vecrtCharPosition.size())
            return;

        m_imageLotView->Overlay_RemoveAll();

        m_pVisionPara->m_vecrtUserSplite.clear();
        for (long i = 0; i < nCharNum; i++)
        {
            Ipvm::Rect32s rtChar = m_pVisionSpec->m_plTeachCharROI[i] + Ipvm::Conversion::ToPoint32s2(imageCenter);

            BOOL bSplite = FALSE;
            for (long nSpliteID = 0; nSpliteID < (long)m_vecptSplitePoint.size(); nSpliteID++)
            {
                if (rtChar.PtInRect(m_vecptSplitePoint[nSpliteID]))
                {
                    bSplite = TRUE;
                    break;
                }
            }

            if (bSplite == FALSE)
            {
                m_imageLotView->Overlay_AddRectangle(rtChar, RGB(0, 255, 0));
            }
            else
            {
                m_imageLotView->Overlay_AddRectangle(rtChar, RGB(255, 0, 0));
                rtChar = rtChar - Ipvm::Conversion::ToPoint32s2(imageCenter);
                m_pVisionPara->m_vecrtUserSplite.push_back(rtChar);
            }
        }
    }

    m_imageLotView->ROI_Show(TRUE);
    m_imageLotView->Overlay_Show(TRUE);
}

void CDlgVisionInspectionMark::UpdateMergeROI()
{
    GetROI(m_pVisionPara->m_nROISettingMethod);

    long nCountMerge = max(0, m_pVisionPara->m_teach_merge_num);
    long nCountMergeOld = (long)m_pVisionPara->m_teach_merge_infos.size();

    /// 바뀌었다면 갱신
    if (nCountMerge != nCountMergeOld)
    {
        m_pVisionPara->m_teach_merge_infos.resize(nCountMerge);

        if (nCountMergeOld < nCountMerge)
        {
            for (long nCount = nCountMergeOld; nCount < nCountMerge; nCount++)
            {
                m_pVisionPara->m_teach_merge_infos[nCount].m_roi.SetRect(-50, -50, 50, 50);
            }
        }
    }

    SetROI(m_pVisionPara->m_nROISettingMethod);
}

void CDlgVisionInspectionMark::UpdateIgnoreROI()
{
    GetROI(m_pVisionPara->m_nROISettingMethod);

    long nCurrIgnoreNum = m_pVisionPara->m_nIgnoreROINum;
    long nOldIgnoreNum = (long)m_pVisionPara->m_vecrtUserIgnore.size();

    /// 바뀌었다면 갱신
    if (nCurrIgnoreNum != nOldIgnoreNum)
    {
        m_pVisionPara->m_vecrtUserIgnore.resize(nCurrIgnoreNum);

        if (nOldIgnoreNum < nCurrIgnoreNum)
        {
            m_pVisionPara->m_vecrtUserIgnore.resize(nCurrIgnoreNum);

            for (long nCount = nOldIgnoreNum; nCount < nCurrIgnoreNum; nCount++)
            {
                m_pVisionPara->m_vecrtUserIgnore[nCount].SetRect(-50, -50, 50, 50);
            }
        }
    }

    SetROI(m_pVisionPara->m_nROISettingMethod);
}

void CDlgVisionInspectionMark::ShowCombineImage(BOOL bAutoThreshold)
{
    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    Ipvm::Image8u combineImage;
    Ipvm::Image8u rotateImage;
    Ipvm::Image8u procImage;

    auto& visionReusableMemory = m_pVisionInsp->getReusableMemory();

    if (!visionReusableMemory.GetInspByteImage(combineImage))
        return;
    if (!visionReusableMemory.GetInspByteImage(rotateImage))
        return;
    if (!visionReusableMemory.GetInspByteImage(procImage))
        return;

    if (!CippModules::GrayImageProcessingManage(visionReusableMemory, &image, false, Ipvm::Rect32s(image),
            m_pVisionPara->m_ImageProcMangePara, combineImage))
    {
        return;
    }

    CString FrameToString;

    if (m_pVisionPara->m_ImageProcMangePara.isCombine())
    {
        FrameToString.Format(_T("Combined Image **"));
    }
    else
    {
        FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
    }

    m_pVisionInsp->m_bodyAlignResult = m_pVisionInsp->GetAlignInfo();

    if (m_pVisionInsp->m_bodyAlignResult == nullptr)
        return;

    const long nSizeX = m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const long nSizeY = m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    auto centerPoint = m_pVisionInsp->m_bodyAlignResult->m_center;
    float fBodyangle_deg = m_pVisionInsp->m_bodyAlignResult->m_angle_rad * (float)ITP_RAD_TO_DEG;
    float fMarkParaAngle_deg = (float)m_pVisionPara->m_nMarkAngle_deg;
    float imageAngle_deg = fBodyangle_deg + fMarkParaAngle_deg;

    const auto& scale = m_pVisionInsp->getScale();
    Ipvm::Rect32s rtMarkSearchROI = scale.convert_BCUToPixel(m_pVisionSpec->m_rtMarkTeachROI_BCU, imageCenter);

    Ipvm::Rect32s rtInspROI_MapData;
    Ipvm::Point32r2 ptCenter;
    float fAngle_deg = 0.f;

    auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;

    if (m_pVisionPara->m_nROISettingMethod != ROISettingMethod_Manual)
    {
        rtInspROI_MapData = rtMarkSearchROI;
        ptCenter.Set(sMarkMapInfo_px.sTeachROI_Info.fOffsetX, sMarkMapInfo_px.sTeachROI_Info.fOffsetY);
        fAngle_deg = sMarkMapInfo_px.sTeachROI_Info.fAngle;
    }
    else
    {
        rtInspROI_MapData = Ipvm::Rect32s(image);
        ptCenter = centerPoint;
        fAngle_deg = imageAngle_deg;
    }

    if (bAutoThreshold) //원본영상에서 Threshold값 뽑고
    {
        BYTE nThresholdValue = (BYTE)m_pVisionSpec->m_nMarkThreshold;
        BYTE nLowMean(0);
        BYTE nHighMean(0);

        Ipvm::Image8u CalcThresholdValueImage;
        if (!visionReusableMemory.GetInspByteImage(CalcThresholdValueImage))
            return;

        Ipvm::ImageProcessing::Copy(combineImage, Ipvm::Rect32s(combineImage), CalcThresholdValueImage);

        if (m_pVisionPara->m_bAlignAreaIgnore)
            m_pVisionInsp->GetIgnoreImageByDebugInfo(combineImage, rtMarkSearchROI, CalcThresholdValueImage);

        std::vector<BYTE> vecbyIgnoreZero(0);
        long nImageSIzeX = CalcThresholdValueImage.GetSizeX();
        long nIndex(0), nIndexY(0);
        BYTE* pbyCalcImage = CalcThresholdValueImage.GetMem();
        for (long ny = rtMarkSearchROI.m_top; ny < rtMarkSearchROI.m_bottom; ny++)
        {
            nIndexY = nImageSIzeX * ny;
            for (long nx = rtMarkSearchROI.m_left; nx < rtMarkSearchROI.m_right; nx++)
            {
                nIndex = nIndexY + nx;
                if (pbyCalcImage[nIndex] > 0)
                    vecbyIgnoreZero.push_back(pbyCalcImage[nIndex]);
            }
        }
        long nBufSize = (long)vecbyIgnoreZero.size();
        if (nBufSize <= 0)
            return;

        Ipvm::Image8u OtsuImageBuf(nBufSize, 1, &vecbyIgnoreZero[0], nBufSize);

        Ipvm::ImageProcessing::GetThresholdOtsu(OtsuImageBuf, rtMarkSearchROI, nLowMean, nHighMean, nThresholdValue);
        m_pVisionSpec->m_nMarkThreshold = nThresholdValue;
    }

    /// Image PreProcess
    if (!m_pVisionInsp->ImplProcessing(
            combineImage, Ipvm::Rect32s(combineImage), m_pVisionPara->m_nMarkPreProcMode, procImage))
        return;

    /// Image Rotate
    Ipvm::ImageProcessing::RotateLinearInterpolation(
        procImage, rtInspROI_MapData, ptCenter, fAngle_deg, Ipvm::Point32r2(0.f, 0.f), rotateImage);

    m_ThresholdImage.Create(procImage.GetSizeX(), procImage.GetSizeY());
    Ipvm::ImageProcessing::Copy(combineImage, Ipvm::Rect32s(combineImage), m_ThresholdImage);

    Ipvm::ImageProcessing::BinarizeGreaterEqual(
        rotateImage, rtMarkSearchROI, (BYTE)min(255, max(0, m_pVisionSpec->m_nMarkThreshold)), m_ThresholdImage);

    if (m_pVisionPara->m_bWhiteBackGround)
    {
        Ipvm::ImageProcessing::BitwiseNot(rtMarkSearchROI, m_ThresholdImage);
    }

    if (m_pVisionPara->m_bAlignAreaIgnore)
        m_pVisionInsp->GetIgnoreImageByDebugInfo(m_ThresholdImage, rtMarkSearchROI, m_ThresholdImage);

    m_imageLotView->SetImage(m_ThresholdImage, FrameToString);

    FrameToString.Empty();
}

void CDlgVisionInspectionMark::CharSplitTeach()
{
    GetROI(m_pVisionPara->m_nROISettingMethod);
    ///=========Ignore 영역 초기화============///

    //버튼 클릭시 무조건 Ignore는 초기화 된다.
    if (!m_bBtnSplitePush)
    {
        m_bBtnSplitePush = !m_bBtnSplitePush;
        m_vecptSplitePoint.clear();
        m_pVisionPara->m_vecrtUserSplite.clear();

        /// Threshold Value 저장.
        m_pVisionInsp->m_VisionSpec[0]->m_nMarkThreshold = m_pVisionSpec->m_nMarkThreshold;
        m_pVisionInsp->m_bMarkSpliteOptionTeach = FALSE;
        BOOL bTeach = m_pVisionInsp->DoTeach(true);

        if (!bTeach)
        {
            m_pVisionInsp->m_bMarkSpliteOptionTeach = FALSE;
            m_bBtnSplitePush = FALSE;

            CString strMessage;
            strMessage.Format(_T("Can not get the Reference Data \n Please!!. Check Parameter."));
            SimpleMessage(strMessage);
            strMessage.Empty();
            return;
        }

        SetROI(m_pVisionPara->m_nROISettingMethod);

        Ipvm::Image8u RotateImage;
        PreImageViewSeq(RotateImage);

        m_imageLotView->SetImage(RotateImage);
    }
    else
    {
        m_bBtnSplitePush = !m_bBtnSplitePush;

        /// Threshold Value 저장.
        m_pVisionInsp->m_VisionSpec[0]->m_nMarkThreshold = m_pVisionSpec->m_nMarkThreshold;
        m_pVisionInsp->m_bMarkSpliteOptionTeach = TRUE;
        BOOL bTeach = m_pVisionInsp->DoTeach(true);

        SetROI(m_pVisionPara->m_nROISettingMethod);

        m_vecptSplitePoint.clear();
        m_pVisionPara->m_vecrtUserSplite.clear();

        Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
        m_imageLotView->SetImage(image);

        if (!bTeach)
        {
            CString strMessage;
            strMessage.Format(_T("Can not get the Reference Data \n Please!!. Check Parameter."));
            SimpleMessage(strMessage);
            strMessage.Empty();
        }
    }
}

void CDlgVisionInspectionMark::MarkTeach()
{
    BeginWaitCursor(); //kircheis_UIB
    m_vecptSplitePoint.clear();
    m_pVisionPara->m_vecrtUserSplite.clear();

    /// Threshold Value 저장.
    m_pVisionInsp->m_VisionSpec[0]->m_nMarkThreshold = m_pVisionSpec->m_nMarkThreshold;
    m_pVisionInsp->m_bMarkSpliteOptionTeach = FALSE;
    BOOL bTeach = m_pVisionInsp->DoTeach(true);
    m_pVisionInsp->m_bTeach = bTeach;
    m_bBtnSplitePush = FALSE;

    SetROI(m_pVisionPara->m_nROISettingMethod);

    if (!bTeach)
    {
        EndWaitCursor(); //kircheis_UIB
        SimpleMessage(_T("Teach Failed!!"));
        return; //kircheis_HwaMark
    }

    EndWaitCursor(); //kircheis_UIB

    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
    m_imageLotView->SetImage(image);
}

void CDlgVisionInspectionMark::InspectionTest()
{
    BeginWaitCursor(); //kircheis_UIB
    OnBnClickedButtonInspect();
    EndWaitCursor(); //kircheis_UIB
}

LRESULT CDlgVisionInspectionMark::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp->m_visionUnit.GetPreviousVisionProcessing(m_pVisionInsp),
        false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    SetROI(m_pVisionPara->m_nROISettingMethod);

    return 0L;
}

void CDlgVisionInspectionMark::RefreshData()
{
    m_propertyGrid->Refresh();
}

void CDlgVisionInspectionMark::MakeMarkInfoPixel()
{
    m_pVisionInsp->m_result->InitializeMarkInfoforPixel(m_pVisionInsp->getScale(), *m_pVisionPara);
}

void CDlgVisionInspectionMark::ClickedButtonImportMarkROI_MapData()
{
    CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Comma separated values file format (*.csv)|*.csv||"));

    if (dlg.DoModal() != IDOK)
        return;

    ReadMarkMapInfoFromCSV(dlg.GetPathName());
    MakeMarkInfoPixel();

    ConvertionMarkMapInfo(m_pVisionInsp->getScale(), m_sMarkMapReadDataInfo_um, m_sMarkMapReadDataInfo_px);

    ClickButtonSelectEnableMarkMapData(true);

    auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;

    m_pVisionPara->m_nIgnoreROINum = (long)sMarkMapInfo_px.vecIgnoreROI_Info.size();
    m_pVisionPara->m_teach_merge_num = (long)sMarkMapInfo_px.vecMergeROI_Info.size();

    RefreshData();
}

void CDlgVisionInspectionMark::ClickedButtonExportMarkROI_MapData()
{
    CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Comma separated values file format (*.csv)|*.csv||"));

    if (dlg.DoModal() != IDOK)
        return;

    m_pVisionPara->WriteMarkMapInfoToCSV(dlg.GetPathName());
}

void CDlgVisionInspectionMark::DrawMarkMapROI(const BOOL isSplitTeach)
{
    m_imageLotView->Overlay_RemoveAll();

    std::vector<FPI_RECT> vecrtMarkTeachROI(0);
    std::vector<FPI_RECT> vecrtMarkMergeROI(0);
    std::vector<FPI_RECT> vecrtMarkIgnoreROI(0);

    FPI_RECT frtTemp;

    auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;
    MakeMarkROI(sMarkMapInfo_px.sTeachROI_Info, frtTemp);

    vecrtMarkTeachROI.push_back(frtTemp);

    if (vecrtMarkTeachROI.size() <= 0)
        return;

    for (int i = 0; i < sMarkMapInfo_px.vecMergeROI_Info.size(); i++)
    {
        MakeMarkROI(sMarkMapInfo_px.vecMergeROI_Info[i], frtTemp);

        vecrtMarkMergeROI.push_back(frtTemp);
    }

    for (int i = 0; i < sMarkMapInfo_px.vecIgnoreROI_Info.size(); i++)
    {
        MakeMarkROI(sMarkMapInfo_px.vecIgnoreROI_Info[i], frtTemp);

        vecrtMarkIgnoreROI.push_back(frtTemp);
    }

    m_imageLotView->Overlay_AddRectangles(vecrtMarkTeachROI, RGB(0, 255, 0));

    if (vecrtMarkMergeROI.size() > 0)
        m_imageLotView->Overlay_AddRectangles(vecrtMarkMergeROI, RGB(0, 0, 255));

    if (vecrtMarkIgnoreROI.size() > 0)
        m_imageLotView->Overlay_AddRectangles(vecrtMarkIgnoreROI, RGB(255, 0, 0));

    /// splite ROI
    if (isSplitTeach)
    {
        m_imageLotView->Overlay_RemoveAll();

        const long nSizeX = m_pVisionInsp->getImageLotInsp().GetImageSizeX();
        const long nSizeY = m_pVisionInsp->getImageLotInsp().GetImageSizeY();

        Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

        /// Teching이 안되어 있으면 아무짓도 하지 말자.
        long nCharNum = m_pVisionInsp->m_VisionTempSpec->m_nCharNum;
        std::vector<Ipvm::Rect32s>& vecrtCharPosition = m_pVisionInsp->m_result->m_vecrtCharPositionforCalcImage;
        if (nCharNum <= 0 || nCharNum != vecrtCharPosition.size())
            return;

        m_pVisionPara->m_vecrtUserSplite.clear();
        for (long i = 0; i < nCharNum; i++)
        {
            Ipvm::Rect32s rtChar = m_pVisionSpec->m_plTeachCharROI[i] + Ipvm::Conversion::ToPoint32s2(imageCenter);

            BOOL bSplite = FALSE;
            for (long nSpliteID = 0; nSpliteID < (long)m_vecptSplitePoint.size(); nSpliteID++)
            {
                if (rtChar.PtInRect(m_vecptSplitePoint[nSpliteID]))
                {
                    bSplite = TRUE;
                    break;
                }
            }

            if (bSplite == FALSE)
            {
                m_imageLotView->Overlay_AddRectangle(rtChar, RGB(0, 255, 0));
            }
            else
            {
                m_imageLotView->Overlay_AddRectangle(rtChar, RGB(255, 0, 0));
                rtChar = rtChar - Ipvm::Conversion::ToPoint32s2(imageCenter);
                m_pVisionPara->m_vecrtUserSplite.push_back(rtChar);
            }
        }
    }

    SetROI_MapData();
    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionMark::MakeMarkROI(sMarkROI_Info i_sROI_Info, FPI_RECT& o_frtResultROI)
{
    float fMarkHalfWidth = i_sROI_Info.fWidth / 2.f;
    float fMarkHalfLength = i_sROI_Info.fLength / 2.f;
    float fMarkAngle = -i_sROI_Info.fAngle;

    FPI_RECT sfrtMarkROI;
    Ipvm::Point32r2 fCenter(i_sROI_Info.fOffsetX, i_sROI_Info.fOffsetY);
    Ipvm::Rect32r frtROI;
    frtROI.m_left = (fCenter.m_x - fMarkHalfWidth);
    frtROI.m_top = (fCenter.m_y - fMarkHalfLength);
    frtROI.m_right = (fCenter.m_x + fMarkHalfWidth);
    frtROI.m_bottom = (fCenter.m_y + fMarkHalfLength);

    sfrtMarkROI = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
        Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));

    if (fMarkAngle != 0)
    {
        float fAngle = fMarkAngle * ITP_DEG_TO_RAD;

        // 시계 반대 방향이 +
        sfrtMarkROI = sfrtMarkROI.Rotate(fAngle);
    }

    o_frtResultROI = sfrtMarkROI;
}

void CDlgVisionInspectionMark::SetRoiParameter()
{
    if (m_pVisionPara->m_nROISettingMethod == ROISettingMethod_Manual)
    {
        m_propertyGrid->GetItem(ITEM_ID_MARK_ID_IMPORT_MAP + 3)->SetReadOnly(TRUE);
        //m_propertyGrid->GetItem(ITEM_ID_MARK_ID_EXPORT_MAP + 3)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(ITEM_ID_MARK_ID_SHOW_MAP + 3)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(ITEM_ID_COMBO_CHARMERGE_ROINUM + 3)->SetReadOnly(FALSE);
        m_propertyGrid->GetItem(ITEM_ID_COMBO_CHARIGNORE_ROINUM + 3)->SetReadOnly(FALSE);
        m_propertyGrid->GetItem(ITEM_ID_MARK_ANGLE + 5)->SetReadOnly(FALSE);

        SetROI(m_pVisionPara->m_nROISettingMethod);

        m_imageLotView->Overlay_RemoveAll();
        m_imageLotView->ROI_Show(TRUE);
    }
    else
    {
        m_propertyGrid->GetItem(ITEM_ID_MARK_ID_IMPORT_MAP + 3)->SetReadOnly(FALSE);
        //m_propertyGrid->GetItem(ITEM_ID_MARK_ID_EXPORT_MAP + 3)->SetReadOnly(FALSE);
        m_propertyGrid->GetItem(ITEM_ID_MARK_ID_SHOW_MAP + 3)->SetReadOnly(FALSE);
        m_propertyGrid->GetItem(ITEM_ID_COMBO_CHARMERGE_ROINUM + 3)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(ITEM_ID_COMBO_CHARIGNORE_ROINUM + 3)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(ITEM_ID_MARK_ANGLE + 5)->SetReadOnly(TRUE);

        GetROI(m_pVisionPara->m_nROISettingMethod);
        SetROI(m_pVisionPara->m_nROISettingMethod);

        m_imageLotView->ROI_Show(FALSE);
    }

    m_imageLotView->Overlay_Show(TRUE);
}

void CDlgVisionInspectionMark::SetROI(long nMode)
{
    if (nMode == ROISettingMethod_Manual)
    {
        SetROI();
    }
    else
    {
        DrawMarkMapROI(m_bBtnSplitePush);
    }
}

void CDlgVisionInspectionMark::GetROI(long nMode)
{
    if (nMode == ROISettingMethod_Manual)
    {
        GetROI();
    }
    else
    {
        SetROI_MapData();
    }
}

void CDlgVisionInspectionMark::SetGroupData_um(
    const std::vector<sMarkReadDataInfo>& i_vecsReadDataInfo, sReadMarkMapInfo& o_sGroupInfo_um)
{
    const long nDataSize = (long)i_vecsReadDataInfo.size();

    if (nDataSize <= 0)
        return;

    o_sGroupInfo_um.vecTeachROI_Info.clear();
    o_sGroupInfo_um.vecMergeROI_Info.clear();
    o_sGroupInfo_um.vecIgnoreROI_Info.clear();

    for (long nRepeatidx = 0; nRepeatidx < nDataSize; nRepeatidx++)
    {
        sMarkROI_Info sTemp;

        sTemp.strID = i_vecsReadDataInfo[nRepeatidx].strID;
        sTemp.strName = i_vecsReadDataInfo[nRepeatidx].strName;
        sTemp.fOffsetX = i_vecsReadDataInfo[nRepeatidx].fOffsetX * 1000.f;
        sTemp.fOffsetY = i_vecsReadDataInfo[nRepeatidx].fOffsetY * 1000.f;
        sTemp.fWidth = i_vecsReadDataInfo[nRepeatidx].fWidth * 1000.f;
        sTemp.fLength = i_vecsReadDataInfo[nRepeatidx].fLength * 1000.f;
        sTemp.fAngle = i_vecsReadDataInfo[nRepeatidx].fAngle;

        if (i_vecsReadDataInfo[nRepeatidx].strType.CompareNoCase(g_szMarkROITypeName[MARK_ROI_TEACH]) == 0)
        {
            sTemp.nType = MARK_ROI_TEACH;
            o_sGroupInfo_um.vecTeachROI_Info.push_back(sTemp);
        }
        else if (i_vecsReadDataInfo[nRepeatidx].strType.CompareNoCase(g_szMarkROITypeName[MARK_ROI_MERGE]) == 0)
        {
            sTemp.nType = MARK_ROI_MERGE;
            o_sGroupInfo_um.vecMergeROI_Info.push_back(sTemp);
        }
        else if (i_vecsReadDataInfo[nRepeatidx].strType.CompareNoCase(g_szMarkROITypeName[MARK_ROI_IGNORE]) == 0)
        {
            sTemp.nType = MARK_ROI_IGNORE;
            o_sGroupInfo_um.vecIgnoreROI_Info.push_back(sTemp);
        }
        else
        {
            continue;
        }
    }
}

void CDlgVisionInspectionMark::ReadMarkMapInfoFromCSV(CString strFileFullPath)
{
    std::vector<sMarkReadDataInfo> vecsMarkMapReadDataInfo;

    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(strFileFullPath);
    bool headerRead = false;

    char buffer[1000];

    enum fixColumn
    {
        FX_ID,
        FX_NAME,
        FX_X,
        FX_Y,
        FX_WIDTH,
        FX_LENGTH,
        FX_ANGLE,
        FX_TYPE,
        FX_END,
    };

    long columnLink[FX_END];
    long headerColumnCount(0);
    long rowIndex(0);

    CString strErrorMessage;
    while (infile.getline(buffer, 1000))
    {
        rowIndex++;

        std::basic_string<char> temp(buffer);

        temp_string_size_type findStart = 0;
        temp_string_size_type findEnd = 0;

        if (!headerRead)
        {
            //---------------------------------------------------------------------------------------
            // Read Header
            //---------------------------------------------------------------------------------------

            std::map<CStringA, long> headerIndexMap;
            long index = 0;
            while (1)
            {
                findEnd = temp.find_first_of(',', findStart);
                CStringA text;
                if (findEnd == std::basic_string<char>::npos)
                {
                    text.SetString(buffer + findStart, CAST_INT(temp.length() - findStart));
                    headerIndexMap[text] = index;
                    break;
                }
                else
                {
                    text.SetString(buffer + findStart, CAST_INT(findEnd - findStart));
                    headerIndexMap[text] = index;
                    findStart = findEnd + 1;
                    index++;
                }
            }

            // 헤더 정보중 필수 항목들이 모두 들어왔는지 확인한다

            for (long item = 0; item < FX_END; item++)
            {
                CStringA columnName;
                switch (item)
                {
                    case FX_ID:
                        columnName = _T("MarkID");
                        break;
                    case FX_NAME:
                        columnName = _T("Name");
                        break;
                    case FX_X:
                        columnName = _T("CenterPosX(mm)");
                        break;
                    case FX_Y:
                        columnName = _T("CenterPosY(mm)");
                        break;
                    case FX_WIDTH:
                        columnName = _T("ROI Width(mm)");
                        break;
                    case FX_LENGTH:
                        columnName = _T("ROI Height(mm)");
                        break;
                    case FX_ANGLE:
                        columnName = _T("Angle(DEG)");
                        break;
                    case FX_TYPE:
                        columnName = _T("Type");
                        break;
                }

                if (columnName.IsEmpty())
                {
                    strErrorMessage.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
                    ::AfxMessageBox(strErrorMessage);
                    return;
                }

                columnLink[item] = headerIndexMap[columnName];
            }

            headerColumnCount = long(headerIndexMap.size());
            headerRead = true;
            continue;
        }

        std::vector<CStringA> dataList;
        while (1)
        {
            findEnd = temp.find_first_of(',', findStart);
            CStringA text;
            if (findEnd == std::basic_string<char>::npos)
            {
                text.SetString(buffer + findStart, CAST_INT(temp.length() - findStart));
                dataList.push_back(text);
                break;
            }
            else
            {
                text.SetString(buffer + findStart, CAST_INT(findEnd - findStart));
                dataList.push_back(text);
                findStart = findEnd + 1;
            }
        }

        if (dataList.size())
        {
            if (dataList.size() != headerColumnCount)
            {
                // 헤더 컬럼과 데이터 컴럼의 수가 다르다
                strErrorMessage.Format(_T("The number of columns is different. (%d Line)"), rowIndex);
                return;
            }

            sMarkReadDataInfo sMarkROIDataInfo;
            sMarkROIDataInfo.strID = dataList[columnLink[FX_ID]];
            sMarkROIDataInfo.strName = dataList[columnLink[FX_NAME]];
            sMarkROIDataInfo.fOffsetX = CAST_FLOAT(::atof(dataList[columnLink[FX_X]]));
            sMarkROIDataInfo.fOffsetY = CAST_FLOAT(::atof(dataList[columnLink[FX_Y]]));
            sMarkROIDataInfo.fWidth = CAST_FLOAT(::atof(dataList[columnLink[FX_WIDTH]]));
            sMarkROIDataInfo.fLength = CAST_FLOAT(::atof(dataList[columnLink[FX_LENGTH]]));
            sMarkROIDataInfo.fAngle = CAST_FLOAT(::atof(dataList[columnLink[FX_ANGLE]]));
            sMarkROIDataInfo.strType = dataList[columnLink[FX_TYPE]];

            vecsMarkMapReadDataInfo.push_back(sMarkROIDataInfo);
        }
    }

    SetGroupData_um(vecsMarkMapReadDataInfo, m_sMarkMapReadDataInfo_um);
}

void CDlgVisionInspectionMark::SetReadMapDataInfo_px(sReadMarkMapInfo i_sMarkMapInfo)
{
    m_sMarkMapReadDataInfo_px.InitData();

    m_sMarkMapReadDataInfo_px = i_sMarkMapInfo;
}

void CDlgVisionInspectionMark::ClickButtonSelectEnableMarkMapData(bool i_bImportMode)
{
    bool bIsTeachROI(false);
    if (m_sMarkMapReadDataInfo_px.vecTeachROI_Info.size() <= 0)
        bIsTeachROI = true;

    CDlgSelectEnableMarkMapData VisionMapDataEditorDlg(
        m_pVisionInsp, m_sMarkMapReadDataInfo_px, bIsTeachROI, i_bImportMode, this);

    if (bIsTeachROI)
        ::SimpleMessage(_T("[Warring] Cannot find Teach ROI in MapData."));

    if (VisionMapDataEditorDlg.DoModal() == IDOK)
    {
        auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;
        auto& sMarkMapInfo_um = m_pVisionPara->m_sMarkMapInfo_um;
        sMarkMapInfo_px.Clear();

        sMarkMapInfo_px = VisionMapDataEditorDlg.GetEnableMarkMapInfo();
        ConvertionMarkMapInfo(m_pVisionInsp->getScale(), sMarkMapInfo_px, FALSE, sMarkMapInfo_um);
    }
}

void CDlgVisionInspectionMark::RefreshMarkDataInfo()
{
    const long nSizeX = m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const long nSizeY = m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    sReadMarkMapInfo sManualMarkDataInfo_px;
    sMarkROI_Info sDefalutMarkROI_Info;

    //Teach ROI
    const auto& scale = m_pVisionInsp->getScale();
    auto Teachroi = scale.convert_BCUToPixel(m_pVisionSpec->m_rtMarkTeachROI_BCU, imageCenter);

    sDefalutMarkROI_Info.strID = _T("T1");
    sDefalutMarkROI_Info.strName = _T("MarkTeachROI");
    sDefalutMarkROI_Info.fOffsetX = CAST_FLOAT(Teachroi.CenterPoint().m_x);
    sDefalutMarkROI_Info.fOffsetY = CAST_FLOAT(Teachroi.CenterPoint().m_y);
    sDefalutMarkROI_Info.fWidth = CAST_FLOAT(Teachroi.Width());
    sDefalutMarkROI_Info.fLength = CAST_FLOAT(Teachroi.Height());
    sDefalutMarkROI_Info.fAngle = CAST_FLOAT(m_pVisionPara->m_nMarkAngle_deg);
    sDefalutMarkROI_Info.nType = MarkROIType::MARK_ROI_TEACH;

    sManualMarkDataInfo_px.vecTeachROI_Info.push_back(sDefalutMarkROI_Info);

    //Merge ROI
    for (long nMergeROI_idx = 0; nMergeROI_idx < m_pVisionPara->m_teach_merge_infos.size(); nMergeROI_idx++)
    {
        sDefalutMarkROI_Info.Clear();

        auto Mergeroi
            = m_pVisionPara->m_teach_merge_infos[nMergeROI_idx].m_roi + Ipvm::Conversion::ToPoint32s2(imageCenter);
        sDefalutMarkROI_Info.strID.Format(_T("M%d"), nMergeROI_idx);
        sDefalutMarkROI_Info.strName.Format(_T("MergeROI_%d"), nMergeROI_idx);
        sDefalutMarkROI_Info.fOffsetX = CAST_FLOAT(Mergeroi.CenterPoint().m_x);
        sDefalutMarkROI_Info.fOffsetY = CAST_FLOAT(Mergeroi.CenterPoint().m_y);
        sDefalutMarkROI_Info.fWidth = CAST_FLOAT(Mergeroi.Width());
        sDefalutMarkROI_Info.fLength = CAST_FLOAT(Mergeroi.Height());
        sDefalutMarkROI_Info.fAngle = CAST_FLOAT(m_pVisionPara->m_nMarkAngle_deg);
        sDefalutMarkROI_Info.nType = MarkROIType::MARK_ROI_MERGE;

        sManualMarkDataInfo_px.vecMergeROI_Info.push_back(sDefalutMarkROI_Info);
    }

    //IgnoreROI
    for (long nIgnoreROI_idx = 0; nIgnoreROI_idx < m_pVisionPara->m_vecrtUserIgnore.size(); nIgnoreROI_idx++)
    {
        sDefalutMarkROI_Info.Clear();

        auto Ignoreroi = m_pVisionPara->m_vecrtUserIgnore[nIgnoreROI_idx] + Ipvm::Conversion::ToPoint32s2(imageCenter);
        sDefalutMarkROI_Info.strID.Format(_T("I%d"), nIgnoreROI_idx);
        sDefalutMarkROI_Info.strName.Format(_T("IgnoreROI_%d"), nIgnoreROI_idx);
        sDefalutMarkROI_Info.fOffsetX = CAST_FLOAT(Ignoreroi.CenterPoint().m_x);
        sDefalutMarkROI_Info.fOffsetY = CAST_FLOAT(Ignoreroi.CenterPoint().m_y);
        sDefalutMarkROI_Info.fWidth = CAST_FLOAT(Ignoreroi.Width());
        sDefalutMarkROI_Info.fLength = CAST_FLOAT(Ignoreroi.Height());
        sDefalutMarkROI_Info.fAngle = CAST_FLOAT(m_pVisionPara->m_nMarkAngle_deg);
        sDefalutMarkROI_Info.nType = MarkROIType::MARK_ROI_IGNORE;

        sManualMarkDataInfo_px.vecIgnoreROI_Info.push_back(sDefalutMarkROI_Info);
    }

    SetReadMapDataInfo_px(sManualMarkDataInfo_px);

    auto& sMarkMapInfo_px = m_pVisionInsp->m_result->m_sMarkMapInfo_px;
    auto& sMarkMapInfo_um = m_pVisionPara->m_sMarkMapInfo_um;

    sMarkMapInfo_um.Clear();
    sMarkMapInfo_px.Clear();

    sMarkMapInfo_px.sTeachROI_Info = sManualMarkDataInfo_px.vecTeachROI_Info[0];
    sMarkMapInfo_px.vecMergeROI_Info = sManualMarkDataInfo_px.vecMergeROI_Info;
    sMarkMapInfo_px.vecIgnoreROI_Info = sManualMarkDataInfo_px.vecIgnoreROI_Info;

    ConvertionMarkMapInfo(scale, sMarkMapInfo_px, FALSE, sMarkMapInfo_um);
}
