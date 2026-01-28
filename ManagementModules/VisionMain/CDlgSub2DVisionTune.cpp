//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CDlgSub2DVisionTune.h"

//CPP_2_________________________________ This project's headers
//#include "VisionMain.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../Preprocessor.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Point8u3.h>
#include <Ipvm/Gadget/Miscellaneous.h>
#include <Ipvm/Widget/ImageView.h>
#include <Ipvm/Widget/ProfileView.h>

//CPP_5_________________________________ Standard library headers
#include <Windows.h>
#include <fstream>
#include <numeric>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RT_GRID_SEPERATOR 5
#define RT_PROFILEVIEW_HEIGHT 250
#define RT_GRID_FUNCTION_HEIGHT 110 // 1번 Grid Control 높이
#define RT_GRID_SCAN_HEIGHT 760 // 2번 Grid Control 높이
#define RT_GRID_VEIFY_HEIGHT 440 // 3번 Grid Control 높이
#define RT_GRID_SPEC_HEIGHT 350 // Grid Spec

#define PROPERTY_GRID_ITEM_FIRST_ID 60
#define PROPERTY_GRID_ITEM_ILLUM_RESULT_FIRST_ID 200

#define UM_GRAB_LIVE_EVENT (WM_USER + 1054)

#define VERIFY_ILLUM_DEFAULT_GV 100
#define VERIFY_ILLUM_OBLIQUE_DEFAULT_SPEC_PERCENT 80
#define VERIFY_ILLUM_COAXIAL_DEFAULT_SPEC_PERCENT 60

#define GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X 10
#define GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y 10

#define DIFLATE_FROM_FULL_IMAGE_X_DEFAULT_PX 400
#define DIFLATE_FROM_FULL_IMAGE_Y_DEFAULT_PX 400

#define GRAY_TARGET_SBU_AREA_SIDE_COUNT_X 15
#define GRAY_TARGET_SBU_AREA_SIDE_COUNT_Y 6

#define DIFLATE_FROM_FULL_IMAGE_X_SIDE_PX 196
#define DIFLATE_FROM_FULL_IMAGE_Y_SIDE_PX 250

#define CHANNEL_REVERSE 8

enum PropertyTabID
{
    TAB_ID_CALIBRATION = 0,
    TAB_ID_VERIFY,
};

enum PropertyTargetID
{
    TARGET_ID_GRAY = 0,
    TARGET_ID_MIRROR,
};

enum PropertyLiveID
{
    ITEM_ID_UNIFORMITY_LIVE_UPDATE_OFF = 0,
    ITEM_ID_UNIFORMITY_LIVE_UPDATE_ON
};

enum ColorResultID
{
    ColorResultID_Red = 0,
    ColorResultID_Green,
    ColorResultID_Blue,
};

enum PropertyGridItemID
{
    ITEM_ID_FUNCTION_TYPE = 1,
    ITEM_ID_SCAN,
    ITEM_ID_STOP,
    ITEM_ID_SAVE_PARAMETER,
    ITEM_ID_SAVE_RESULT,
    ITEM_ID_LIVE_ON,
    ITEM_ID_LIVE_OFF,
    ITEM_ID_SCAN_VERIFY_FOR_GV,
    ITEM_ID_SCAN_VERIFY_FOR_MS,
    ITEM_ID_SCAN_VERIFY,
    ITEM_ID_STOP_VERIFY,
    ITEM_ID_VERIFY_REALTIME_DISPLAY,
    ITEM_ID_VERIFY_LIVE_ON,
    ITEM_ID_VERIFY_LIVE_OFF,
    ITEM_ID_ILLUM_VERIFICATION_SI,
    ITEM_ID_SAVECURRENT_ILLUMINATION_DATA,
    ITEM_ID_SELECT_COLOR_TYPE_UNIFORMITY_RESULT,
    ITEM_ID_TARGET_TYPE,
    ITEM_ID_CUR_SIDE_MODULE_TYPE,
    ITEM_END, //mc_조명은 ITEM_END이후 index를 따라간다 이후에 추가되는 idx는 위에다가 추가
    ITEM_ID_ILLUM_IDX = ITEM_END + PROPERTY_GRID_ITEM_FIRST_ID,
};

IMPLEMENT_DYNAMIC(CDlgSub2DVisionTune, CDialog)

CDlgSub2DVisionTune::CDlgSub2DVisionTune(CWnd* pParent /*=NULL*/, const long& vision_type)
    : CDialog(IDD_DIALOG_ILLUM_CALIBRATION_TUNING, pParent)
    , m_vision_type(vision_type)
    , m_imageView(nullptr)
    , histogram_view(nullptr)
    , intensity_profile_view(nullptr)
    , grid_Function(nullptr)
    , grid_Calibration(nullptr)
    , grid_Verify(nullptr)
    , use_illumation_Count(getUse_illum_count(vision_type))
    , function_type(TAB_ID_VERIFY)
    , calibration_target_type(TARGET_ID_GRAY)
    , m_cur_side_module(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    , UV_channel_id(SystemConfig::GetInstance().m_nNgrvUVchID)
    , IR_channel_id(SystemConfig::GetInstance().m_nNgrvIRchID)
    , simulationMode(SystemConfig::GetInstance().IsHardwareExist() == TRUE ? false : true)
    , uniformity_live_update_mode(ITEM_ID_UNIFORMITY_LIVE_UPDATE_OFF)
    , view_color_image_RGB_idx(-1)
    , scan_time(0)
    , stop_calibration(true)
    , isLive_mode(false)
    , isLiveScan_mode(false)
    , mono_Image(new Ipvm::Image8u)
    , color_Image(new Ipvm::Image8u3)
{
    view_color_image_RGB_idx = vision_type == VISIONTYPE_NGRV_INSP ? ColorResultID_Red : -1;

    verify_grabed_images.clear();
    verify_grabed_color_images.clear();

    for (long ch = 0; ch < LED_ILLUM_CHANNEL_MAX; ch++)
    {
        illumination_times_ms_live[ch] = 0.f;
        verify_ref_illumination_ms[ch] = 0.f; //mc_이거는 Vision System참조해서 바로 넣을수 있도록 한다
        verify_ref_illumination_gv[ch] = VERIFY_ILLUM_DEFAULT_GV;
        verify_illumination_spec_values_percent[ch]
            = VERIFY_ILLUM_OBLIQUE_DEFAULT_SPEC_PERCENT; //동축은 별도로 줘야한다

        //동축 예외처리
        switch (vision_type)
        {
            case VISIONTYPE_NGRV_INSP:
            {
                //?
            }
            break;
            case VISIONTYPE_SIDE_INSP:
            {
                if (ch >= LED_ILLUM_CHANNEL_SIDE_OBLIQUE && ch <= LED_ILLUM_CHANNEL_SIDE_DEFAULT)
                    verify_illumination_spec_values_percent[ch] = VERIFY_ILLUM_COAXIAL_DEFAULT_SPEC_PERCENT;
            }
            break;
            case VISIONTYPE_2D_INSP:
            {
                if (ch >= LED_ILLUM_CHANNEL_OBLIQUE && ch <= LED_ILLUM_CHANNEL_DEFAULT)
                    verify_illumination_spec_values_percent[ch] = VERIFY_ILLUM_COAXIAL_DEFAULT_SPEC_PERCENT;
            }
            break;
            default:
                break;
        }
    }

    resultIntensities_color_gv->clear();
    resultIntensities_mono_gv.clear();
}

CDlgSub2DVisionTune::~CDlgSub2DVisionTune()
{
    delete m_imageView;
    delete histogram_view;
    delete intensity_profile_view;
    delete grid_Function;
    delete grid_Calibration;
    delete grid_Verify;
}

void CDlgSub2DVisionTune::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_TIMER, scanTime);
    DDX_Control(pDX, IDC_STATIC_STATUS, procStatus);
    DDX_Control(pDX, IDC_TAB_CHANNEL, tabChannel);
    DDX_Control(pDX, IDC_GRID_RESULT, gridResult);
    DDX_Control(pDX, IDC_VISION_TUNE_GRID_SPEC, gridSpec);
}

BEGIN_MESSAGE_MAP(CDlgSub2DVisionTune, CDialog)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CDlgSub2DVisionTune::OnGridNotify)
ON_MESSAGE(UM_GRAB_LIVE_EVENT, &CDlgSub2DVisionTune::OnGrabEndEvent)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CHANNEL, &CDlgSub2DVisionTune::OnTcnSelchangeTabResult)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_VISION_TUNE_GRID_SPEC, &CDlgSub2DVisionTune::On2DGridDataChanged)
ON_NOTIFY(GVN_STATECHANGED, IDC_VISION_TUNE_GRID_SPEC, &CDlgSub2DVisionTune::On2DGridDataChanged)
END_MESSAGE_MAP()

BOOL CDlgSub2DVisionTune::OnInitDialog()
{
    CDialog::OnInitDialog();

    //Window Size
    InitWindowSize();

    SetPropertyGrid_General();
    SetPropertyGrid_Calibration();
    SetPropertyGrid_Verify();

    for (int chIdx = 0; chIdx < use_illumation_Count; chIdx++)
    {
        CString strChannel("");
        strChannel.Format(_T("Ch%d"), chIdx + 1);
        if (m_vision_type == VISIONTYPE_NGRV_INSP && (chIdx == UV_channel_id || chIdx == IR_channel_id))
        {
            continue;
        }

        tabChannel.InsertItem(chIdx, strChannel);

        strChannel.Empty();
    }

    gridResult.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    gridResult.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    gridResult.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    gridSpec.ShowWindow(FALSE);
    gridResult.ShowWindow(FALSE);
    tabChannel.ShowWindow(FALSE);
    m_imageView->HideImageOverlay();

    //SetTimer(1, 600, NULL);

    grid_Calibration->ShowWindow(function_type == TAB_ID_CALIBRATION ? SW_SHOW : SW_HIDE);
    switch (function_type)
    {
        case TAB_ID_CALIBRATION:
            change_scan_function(TRUE);
            break;

        case TAB_ID_VERIFY:
            change_scan_function(FALSE);
            break;
    }

    return TRUE;
}

void CDlgSub2DVisionTune::InitWindowSize()
{
    CRect rtParentClient(0, 0, 0, 0);
    CRect rtImageView(0, 0, 0, 0);
    CRect rt_pGrid_Function(0, 0, 0, 0);
    CRect rt_pGrid_Scan(0, 0, 0, 0);
    CRect rt_pGrid_Verify(0, 0, 0, 0);

    CRect rtClient(0, 0, 0, 0);
    CRect rtHistogramView(0, 0, 0, 0);
    CRect rtIntensityProfileView(0, 0, 0, 0);
    CRect rtStatic_Timer(0, 0, 0, 0);
    CRect rtStatic_Status(0, 0, 0, 0);
    GetClientRect(&rtClient);

    // Rect Child Window
    GetParent()->GetClientRect(rtParentClient);
    MoveWindow(rtParentClient);

    // Rect ImageView
    GetParent()->GetClientRect(rtImageView);
    rtImageView.top = rtImageView.top + 30;
    //rtImageView.left = rtImageView.left + long(430 * g_HardwareConfig.GetMonitorUIFactor());
    rtImageView.left = rtImageView.left + 430;

    rtImageView.bottom = rtImageView.bottom - 175 - RT_PROFILEVIEW_HEIGHT;
    //rtImageView.right = rtImageView.right - 175;
    rtImageView.right = rtImageView.right - 50;
    m_imageView = new Ipvm::ImageView(GetSafeHwnd(), Ipvm::FromMFC(rtImageView));

    // Rect CStatic Time, Status
    rtStatic_Timer.top = rtClient.top + 30;
    rtStatic_Timer.left = rtClient.left + 5;
    //rtStatic_Timer.right = rtStatic_Timer.left + long(425 * g_HardwareConfig.GetMonitorUIFactor());
    //rtStatic_Timer.bottom = rtStatic_Timer.top + long(15 * g_HardwareConfig.GetMonitorUIFactor());
    rtStatic_Timer.right = rtStatic_Timer.left + 425;
    rtStatic_Timer.bottom = rtStatic_Timer.top + 15;

    scanTime.MoveWindow(rtStatic_Timer);
    rtStatic_Status.top = rtStatic_Timer.bottom + 3;
    rtStatic_Status.left = rtClient.left + 5;
    rtStatic_Status.right = rtStatic_Status.left + 425;
    rtStatic_Status.bottom = rtStatic_Status.top + 15;
    //rtStatic_Status.right = rtStatic_Status.left + long(425 * g_HardwareConfig.GetMonitorUIFactor());
    //rtStatic_Status.bottom = rtStatic_Status.top + long(15 * g_HardwareConfig.GetMonitorUIFactor());

    procStatus.MoveWindow(rtStatic_Status);

    rtHistogramView.top = rtImageView.bottom + 5;
    //rtHistogramView.left = rtClient.left + long(430 * g_HardwareConfig.GetMonitorUIFactor());
    rtHistogramView.left = rtClient.left + 430;
    rtHistogramView.right = rtHistogramView.left + 550;
    //rtHistogramView.right = rtHistogramView.left + long(550 * g_HardwareConfig.GetMonitorUIFactor());
    //rtHistogramView.bottom = rtHistogramView.top + long(RT_PROFILEVIEW_HEIGHT * g_HardwareConfig.GetMonitorUIFactor());
    rtHistogramView.bottom = rtHistogramView.top + RT_PROFILEVIEW_HEIGHT;

    rtIntensityProfileView.top = rtImageView.bottom + 5;
    rtIntensityProfileView.left = rtHistogramView.right + 5;
    rtIntensityProfileView.right = rtImageView.right;
    rtIntensityProfileView.bottom = rtIntensityProfileView.top + RT_PROFILEVIEW_HEIGHT;
    //rtIntensityProfileView.bottom
    //    = rtIntensityProfileView.top + long(RT_PROFILEVIEW_HEIGHT * g_HardwareConfig.GetMonitorUIFactor());

    histogram_view = new Ipvm::ProfileView(GetSafeHwnd(), Ipvm::FromMFC(rtHistogramView));
    histogram_view->HideSummary();
    histogram_view->HideCursor();
    histogram_view->SetColor_GraphBackground(RGB(0, 0, 0));
    histogram_view->SetColor_GridBackground(RGB(0, 0, 0));
    histogram_view->SetColor_GridTopHorizontalAxisLine(RGB(0, 0, 0));
    histogram_view->SetColor_GridBottomHorizontalAxisLine(RGB(128, 128, 128));
    histogram_view->SetColor_GridLeftVerticalAxisLine(RGB(128, 128, 128));
    histogram_view->SetColor_GridRightVerticalAxisLine(RGB(0, 0, 0));
    histogram_view->SetColor_GridHorizontalLine(RGB(0, 0, 0));
    histogram_view->SetColor_GridVerticalLine(RGB(0, 0, 0));
    histogram_view->SetColor_HorizontalScaleText(RGB(255, 255, 255));
    histogram_view->SetColor_VerticalScaleText(RGB(255, 255, 255));

    intensity_profile_view = new Ipvm::ProfileView(GetSafeHwnd(), Ipvm::FromMFC(rtIntensityProfileView));
    intensity_profile_view->HideSummary();
    intensity_profile_view->HideCursor();
    intensity_profile_view->SetColor_GraphBackground(RGB(0, 0, 0));
    intensity_profile_view->SetColor_GridBackground(RGB(0, 0, 0));
    intensity_profile_view->SetColor_GridTopHorizontalAxisLine(RGB(0, 0, 0));
    intensity_profile_view->SetColor_GridBottomHorizontalAxisLine(RGB(128, 128, 128));
    intensity_profile_view->SetColor_GridLeftVerticalAxisLine(RGB(128, 128, 128));
    intensity_profile_view->SetColor_GridRightVerticalAxisLine(RGB(0, 0, 0));
    intensity_profile_view->SetColor_GridHorizontalLine(RGB(0, 0, 0));
    intensity_profile_view->SetColor_GridVerticalLine(RGB(0, 0, 0));
    intensity_profile_view->SetColor_HorizontalScaleText(RGB(255, 255, 255));
    intensity_profile_view->SetColor_VerticalScaleText(RGB(255, 255, 255));

    // Rect PropertyGrid Function
    GetParent()->GetClientRect(rt_pGrid_Function);
    rt_pGrid_Function.top = rt_pGrid_Function.top + 70;
    rt_pGrid_Function.left = rt_pGrid_Function.left;
    rt_pGrid_Function.right = rtImageView.left - RT_GRID_SEPERATOR;
    rt_pGrid_Function.bottom = rt_pGrid_Function.top + RT_GRID_FUNCTION_HEIGHT;
    //rt_pGrid_Function.bottom
    //    = rt_pGrid_Function.top + long(RT_GRID_FUNCTION_HEIGHT * g_HardwareConfig.GetMonitorUIFactor());
    //
    // Rect PropertyGrid Calibration
    GetParent()->GetClientRect(rt_pGrid_Scan);
    rt_pGrid_Scan.top = rt_pGrid_Function.bottom + 5;
    rt_pGrid_Scan.left = rt_pGrid_Scan.left;
    rt_pGrid_Scan.right = rtImageView.left - RT_GRID_SEPERATOR;
    //rt_pGrid_Scan.bottom = rt_pGrid_Scan.top + long(RT_GRID_SCAN_HEIGHT * g_HardwareConfig.GetMonitorUIFactor());
    rt_pGrid_Scan.bottom = rt_pGrid_Scan.top + RT_GRID_SCAN_HEIGHT;
    // Rect PropertyGrid Verify
    GetParent()->GetClientRect(rt_pGrid_Verify);
    rt_pGrid_Verify.top = rt_pGrid_Function.bottom + 5;
    rt_pGrid_Verify.left = rt_pGrid_Verify.left;
    rt_pGrid_Verify.right = rtImageView.left - RT_GRID_SEPERATOR;
    //rt_pGrid_Verify.bottom = rt_pGrid_Verify.top + long(RT_GRID_VEIFY_HEIGHT * g_HardwareConfig.GetMonitorUIFactor());
    rt_pGrid_Verify.bottom = rt_pGrid_Verify.top + RT_GRID_VEIFY_HEIGHT;

    CRect rt_pGrid_result;
    CRect rt_pGrid_Spec;

    rt_pGrid_result.top = rtImageView.bottom + RT_GRID_SEPERATOR + 22;
    rt_pGrid_result.left = rt_pGrid_Scan.right + RT_GRID_SEPERATOR;
    rt_pGrid_result.right = rtImageView.right; // rtImageView.left - RT_GRID_SEPERATOR;
    //rt_pGrid_result.bottom = rt_pGrid_result.top + long(250 * g_HardwareConfig.GetMonitorUIFactor());
    //rt_pGrid_result.bottom = rt_pGrid_result.top + 250;
    rt_pGrid_result.bottom = rt_pGrid_result.top + 350;
    gridResult.MoveWindow(rt_pGrid_result);

    GetParent()->GetClientRect(rt_pGrid_Spec);
    rt_pGrid_Spec.top = rt_pGrid_Verify.bottom + 5;
    rt_pGrid_Spec.left = rt_pGrid_Spec.left;
    rt_pGrid_Spec.right = rtImageView.left - RT_GRID_SEPERATOR;
    //rt_pGrid_Spec.bottom = rt_pGrid_Spec.top + long(RT_GRID_SPEC_HEIGHT * g_HardwareConfig.GetMonitorUIFactor());
    rt_pGrid_Spec.bottom = rt_pGrid_Spec.top + RT_GRID_SPEC_HEIGHT;
    gridSpec.MoveWindow(rt_pGrid_Spec);

    // Tab Control Rect
    CRect rtTab = rt_pGrid_result;

    rtTab.top = rt_pGrid_result.top - 22;
    //rtTab.bottom = rtTab.top + long(20 * g_HardwareConfig.GetMonitorUIFactor());
    rtTab.bottom = rtTab.top + 20;
    tabChannel.MoveWindow(rtTab);
    tabChannel.AdjustRect(FALSE, rtTab);

    // Create Property Grid 1
    grid_Function = new CXTPPropertyGrid;
    grid_Function->Create(rt_pGrid_Function, this, 0);
    grid_Function->ShowHelp(FALSE);
    // Create Property Grid 2
    grid_Calibration = new CXTPPropertyGrid;
    grid_Calibration->Create(rt_pGrid_Scan, this, 0);
    grid_Calibration->ShowHelp(FALSE);
    // Create Property Grid 3
    grid_Verify = new CXTPPropertyGrid;
    grid_Verify->Create(rt_pGrid_Verify, this, 0);
    grid_Verify->ShowHelp(FALSE);
}

const long CDlgSub2DVisionTune::getUse_illum_count(const long& probeID)
{
    switch (probeID)
    {
        case enVisionType::VISIONTYPE_2D_INSP:
        {
            const long illum_ring_count
                = SystemConfig::GetInstance().GetExistRingillumination() == true ? LED_ILLUM_CHANNEL_RING_MAX : 0;
            return LED_ILLUM_CHANNEL_DEFAULT + illum_ring_count;
        }
        case enVisionType::VISIONTYPE_NGRV_INSP:
            return LED_ILLUM_NGRV_CHANNEL_MAX;
        case enVisionType::VISIONTYPE_SIDE_INSP:
            return LED_ILLUM_CHANNEL_SIDE_DEFAULT;
        default:
            break;
    }

    return -1;
}

void CDlgSub2DVisionTune::SetPropertyGrid_General()
{
    grid_Function->ResetContent();

    if (auto* category = grid_Function->AddCategory(_T("General")))
    {
        if (auto* item = (CXTPPropertyGridItemEnum*)category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Function"), function_type, &function_type)))
        {
            item->GetConstraints()->AddConstraint(_T("Calibration"), TAB_ID_CALIBRATION);
            item->GetConstraints()->AddConstraint(_T("Uniformity"), TAB_ID_VERIFY);
            item->SetConstraintEdit(FALSE);
            item->SetID(ITEM_ID_FUNCTION_TYPE);
            item->SetReadOnly(TRUE); //고정
        }

        if (auto* item = (CXTPPropertyGridItemEnum*)category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Target"), calibration_target_type, &calibration_target_type)))
        {
            item->GetConstraints()->AddConstraint(_T("Gray Target"), TARGET_ID_GRAY);
            item->GetConstraints()->AddConstraint(_T("Optical Mirror"), TARGET_ID_MIRROR);
            item->SetConstraintEdit(FALSE);
            item->SetID(ITEM_ID_TARGET_TYPE);
            item->SetReadOnly(TRUE); //고정
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Save settings"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_SAVE_PARAMETER);
            item->SetReadOnly(TRUE);
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Save result"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_SAVE_RESULT);
            item->SetReadOnly(TRUE);
        }

        category->Expand();
    }

    if (m_vision_type == enVisionType::VISIONTYPE_SIDE_INSP)
    {
        if (auto* category = grid_Function->AddCategory(_T("Side Vision Module")))
        {
            if (auto* item = (CXTPPropertyGridItemEnum*)category->AddChildItem(
                    new CXTPPropertyGridItemEnum(_T("Cur Side Module"), m_cur_side_module, &m_cur_side_module)))
            {
                item->GetConstraints()->AddConstraint(_T("Front"), enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                item->GetConstraints()->AddConstraint(_T("Rear"), enSideVisionModule::SIDE_VISIONMODULE_REAR);
                item->SetConstraintEdit(FALSE);
                item->SetID(ITEM_ID_CUR_SIDE_MODULE_TYPE);
            }

            category->Expand();
        }
    }

    grid_Function->SetViewDivider(0.60);
    grid_Function->HighlightChangedItems(TRUE);
    grid_Function->Refresh();
}

void CDlgSub2DVisionTune::SetPropertyGrid_Calibration()
{
    grid_Calibration->ResetContent();

    if (auto* category = grid_Calibration->AddCategory(_T("Illumination Setting")))
    {
        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Live ON"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_LIVE_ON);
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Live OFF"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_LIVE_OFF);
            item->SetHidden(TRUE);
        }

        for (long idx = 0; idx < use_illumation_Count; idx++)
        {
            if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                    GetChannelNames(m_vision_type, idx), illumination_times_ms_live[idx], _T("%.3lf ms"),
                    illumination_times_ms_live[idx], 0.f, 65.f, 0.002f)))
            {
                item->SetID(ITEM_ID_ILLUM_IDX + idx);

                if (m_vision_type == VISIONTYPE_NGRV_INSP) //NGRV Vision
                {
                    if (idx == UV_channel_id || idx == IR_channel_id)
                    {
                        item->SetReadOnly(TRUE);
                        continue;
                    }
                }

                if (auto* metric = item->GetMetrics(TRUE))
                {
                    metric->m_clrFore = GetChannelColors(m_vision_type, idx, UV_channel_id, IR_channel_id);
                }
                if (auto* metric = item->GetMetrics(FALSE))
                {
                    metric->m_clrFore = GetChannelColors(m_vision_type, idx, UV_channel_id, IR_channel_id);
                }
            }
        }

        category->Expand();
    }

    if (auto* category = grid_Calibration->AddCategory(_T("Scan")))
    {
        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Scan"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_SCAN);
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Stop"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_STOP);
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Verification"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_ILLUM_VERIFICATION_SI);
        }

        if (auto* item
            = category->AddChildItem(new CCustomItemButton(_T("Current illumination Data Save"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_SAVECURRENT_ILLUMINATION_DATA);
            item->SetReadOnly(TRUE); //초기에 값이없는데 쓸려고하면 안되지
        }

        category->Expand();
    }

    grid_Calibration->SetViewDivider(0.60);
    grid_Calibration->HighlightChangedItems(TRUE);
    grid_Calibration->Refresh();
}

void CDlgSub2DVisionTune::SetPropertyGrid_Verify()
{
    grid_Verify->ResetContent();

    if (auto* category = grid_Verify->AddCategory(_T("Verify Illumination Values")))
    {
        if (auto* item = (CXTPPropertyGridItemEnum*)category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Real time display"), uniformity_live_update_mode, &uniformity_live_update_mode)))
        {
            item->GetConstraints()->AddConstraint(_T("Not Use"), ITEM_ID_UNIFORMITY_LIVE_UPDATE_OFF);
            item->GetConstraints()->AddConstraint(_T("Use"), ITEM_ID_UNIFORMITY_LIVE_UPDATE_ON);
            item->SetConstraintEdit(FALSE);
            item->SetID(ITEM_ID_VERIFY_REALTIME_DISPLAY);
        }

        if (m_vision_type == VISIONTYPE_NGRV_INSP)
        {
            if (auto* item = (CXTPPropertyGridItemEnum*)category->AddChildItem(new CXTPPropertyGridItemEnum(
                    _T("Select color type Uniformity result"), view_color_image_RGB_idx, &view_color_image_RGB_idx)))
            {
                item->GetConstraints()->AddConstraint(_T("Color_[R]"), ColorResultID_Red);
                item->GetConstraints()->AddConstraint(_T("Color_[G]"), ColorResultID_Green);
                item->GetConstraints()->AddConstraint(_T("Color_[B]"), ColorResultID_Blue);
                item->SetConstraintEdit(FALSE);
                item->SetID(ITEM_ID_SELECT_COLOR_TYPE_UNIFORMITY_RESULT);
            }
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Live ON"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_VERIFY_LIVE_ON);
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Live OFF"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_VERIFY_LIVE_OFF);
            item->SetHidden(TRUE);
        }

        for (long idx = 0; idx < use_illumation_Count; idx++)
        {
            if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                    GetChannelNames(m_vision_type, idx), illumination_times_ms_live[idx], _T("%.3lf ms"),
                    illumination_times_ms_live[idx], 0.f, 65.f, 0.002f)))
            {
                item->SetID(PROPERTY_GRID_ITEM_ILLUM_RESULT_FIRST_ID + idx);

                if (m_vision_type == VISIONTYPE_NGRV_INSP)
                {
                    if (idx == UV_channel_id || idx == IR_channel_id)
                    {
                        item->SetReadOnly(TRUE);
                        continue;
                    }
                }

                if (auto* metric = item->GetMetrics(TRUE))
                {
                    metric->m_clrFore = GetChannelColors(m_vision_type, idx, UV_channel_id, IR_channel_id);
                }
                if (auto* metric = item->GetMetrics(FALSE))
                {
                    metric->m_clrFore = GetChannelColors(m_vision_type, idx, UV_channel_id, IR_channel_id);
                }
            }
        }
        category->Expand();
    }

    if (auto* category = grid_Verify->AddCategory(_T("Scan")))
    {
        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Scan Uniformity"), TRUE, FALSE)))
        {
            item->SetID(ITEM_ID_SCAN_VERIFY);
        }

        //if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Scan Uniformity for GV"), TRUE, FALSE)))
        //{
        //    item->SetID(ITEM_ID_SCAN_VERIFY_FOR_GV);
        //}

        //if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Scan Uniformity for Time(ms)"), TRUE, FALSE)))
        //{
        //    item->SetID(ITEM_ID_SCAN_VERIFY_FOR_MS);
        //}

        //if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Stop"), TRUE, FALSE)))
        //{
        //    item->SetID(ITEM_ID_STOP_VERIFY);
        //}

        category->Expand();
    }

    grid_Verify->SetViewDivider(0.60);
    grid_Verify->HighlightChangedItems(TRUE);
    grid_Verify->Refresh();
    grid_Verify->ShowWindow(FALSE);
}

LPCWSTR CDlgSub2DVisionTune::GetChannelNames(const long& probeID, const long& channel_index)
{
    switch (probeID)
    {
        case VISIONTYPE_2D_INSP:
        {
            if (channel_index > LED_ILLUM_CHANNEL_MAX)
                return _T("");
            else
                return g_illuminationChannelNames[channel_index];
        }
        break;
        case VISIONTYPE_NGRV_INSP:
        {
            if (channel_index > LED_ILLUM_NGRV_CHANNEL_MAX)
                return _T("");
            else
                return g_illuminationChannelNames_NGRV[channel_index];
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            if (channel_index > LED_ILLUM_CHANNEL_SIDE_DEFAULT)
                return _T("");
            else
                return g_illuminationChannelNames_SIDE[channel_index];
        }
        break;
        default:
            break;
    }

    return _T("");
}

COLORREF CDlgSub2DVisionTune::GetChannelColors(
    const long& vision_type, const long& channel_index, const long& UV_channel_idx, const long& IR_channel_idx)
{
    switch (vision_type)
    {
        case VISIONTYPE_2D_INSP:
        {
            if (channel_index > LED_ILLUM_CHANNEL_MAX)
                return RGB(0, 0, 0);
            else
                return g_illuminationChannelColors[channel_index];
        }
        break;
        case VISIONTYPE_NGRV_INSP:
        {
            if (channel_index > LED_ILLUM_NGRV_CHANNEL_MAX)
                return RGB(0, 0, 0);
            else
            {
                //Vision에서 별도로 UV하고 IR Channel을 선택할 수 있다
                if (channel_index == UV_channel_idx)
                    return RGB(159, 13, 183);
                else if (channel_index == IR_channel_idx)
                    return RGB(220, 60, 0);
                else
                    return g_illuminationChannelColors_NGRV[channel_index];
            }
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            if (channel_index > LED_ILLUM_CHANNEL_SIDE_DEFAULT)
                return RGB(0, 0, 0);
            else
                return g_illuminationChannelColors_SIDE[channel_index];
        }
        break;
        default:
            break;
    }

    return RGB(0, 0, 0);
}

LRESULT CDlgSub2DVisionTune::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_LIVE_ON:
                liveOnOff(true);
                SetReadOnlyButton_Live(TRUE);
                UpdateLiveButtons();
                break;
            case ITEM_ID_LIVE_OFF:
                liveOnOff(false);
                SetReadOnlyButton_Live(FALSE);
                UpdateLiveButtons();
                break;
            case ITEM_ID_SCAN:
                //StartScan_Calibration(m_nProbeID);
                break;
            case ITEM_ID_STOP:
                //StopScan_Calibration();
                break;
            case ITEM_ID_SAVE_PARAMETER:
                break;
            case ITEM_ID_SAVE_RESULT:
                //SaveResult_Calibration();
                break;
            case ITEM_ID_SCAN_VERIFY_FOR_GV:
                StartScan_Uniformity(m_vision_type, true);
                break;
            case ITEM_ID_SCAN_VERIFY_FOR_MS:
                StartScan_Uniformity(m_vision_type, false);
                break;
            case ITEM_ID_SCAN_VERIFY:
                StartScan_Uniformity_Single(m_vision_type);
                break;
            case ITEM_ID_STOP_VERIFY:
                //StopScan_Verify();
                break;
            case ITEM_ID_VERIFY_LIVE_ON:
                liveOnOff(true);
                SetReadOnlyButton_Live(TRUE);
                UpdateLiveButtons();
                break;
            case ITEM_ID_VERIFY_LIVE_OFF:
                liveOnOff(false);
                SetReadOnlyButton_Live(FALSE);
                UpdateLiveButtons();
                break;
            case ITEM_ID_ILLUM_VERIFICATION_SI:
                //m_bRevision = true;
                //StartScan_Calibration(m_nProbeID);
                break;
            case ITEM_ID_SAVECURRENT_ILLUMINATION_DATA:
                //SaveCurrent_illumCalibration_Data();
                break;
        }
    }
    else
    {
        std::array<float, 16> illumValues_ms = {
            0,
        };
        memcpy(&illumValues_ms[0], illumination_times_ms_live, sizeof(illumination_times_ms_live));

        SyncController::GetInstance().SetIllumiParameter(
            0, illumValues_ms, false, false, false, enSideVisionModule(m_cur_side_module));

        if (isLive_mode == false)
            SyncController::GetInstance().TurnOffLight(enSideVisionModule(m_cur_side_module));
        else
            SyncController::GetInstance().TurnOnLight(0, enSideVisionModule(m_cur_side_module));
    }

    switch (item->GetID())
    {
        case ITEM_ID_SELECT_COLOR_TYPE_UNIFORMITY_RESULT:
            SetGridCtrl_Result(m_vision_type, view_color_image_RGB_idx, tabChannel.GetCurSel());
            break;
        case ITEM_ID_FUNCTION_TYPE:
        {
            grid_Calibration->ShowWindow(function_type == TAB_ID_CALIBRATION ? SW_SHOW : SW_HIDE);

            switch (function_type)
            {
                case TAB_ID_CALIBRATION:
                    change_scan_function(TRUE);
                    break;

                case TAB_ID_VERIFY:
                    change_scan_function(FALSE);
                    break;
            }
        }
        break;
        case ITEM_ID_TARGET_TYPE:
        case ITEM_ID_CUR_SIDE_MODULE_TYPE:
            SetPropertyGrid_Calibration();
            break;
    }

    if (m_vision_type == VISIONTYPE_NGRV_INSP && item->GetID() >= ITEM_ID_ILLUM_IDX) //NGRV Vision일 경우 체크
    {
        float fillumSum = 0.f; // Live 조명 값들의 합
        float fInputLimit = 0.f;
        float fLimit_UVIR = 500.f; // Float일 경우 계산 시 소수점이 생겨 정확이 500의 값이 아니게 될 경우가 있어 +1 설정
        float fLimit_without_UVIR
            = 200.f; // Float일 경우 계산 시 소수점이 생겨 정확이 200의 값이 아니게 될 경우가 있어 +1 설정

        BOOL bUseUVIR = FALSE;
        std::vector<float> vecfillumCheck; // 조명 값 Buffer Vector

        for (long idx = 0; idx < use_illumation_Count; idx++)
        {
            //	UV 혹은 IR 조명을 사용할 경우
            if ((UV_channel_id >= 0 && UV_channel_id < use_illumation_Count) && IR_channel_id >= 0
                && IR_channel_id < use_illumation_Count)
            {
                if (illumination_times_ms_live[UV_channel_id] > 0 || illumination_times_ms_live[IR_channel_id] > 0)
                {
                    bUseUVIR = TRUE;
                }
            }

            fillumSum += illumination_times_ms_live[idx];

            vecfillumCheck.push_back(illumination_times_ms_live[idx]);
        }

        if (bUseUVIR == TRUE) //	UV, IR 조명을 사용할 경우 500 msec
        {
            fInputLimit = fLimit_UVIR;
        }
        else // UV, IR 조명을 사용하지 않을 경우 200 msec
        {
            fInputLimit = fLimit_without_UVIR;
        }

        if (fillumSum > fInputLimit + 0.1f) // 각 Channel 별 노출 시간 총 합이 설정한 한계치보다 높은 경우
        {
            AfxMessageBox(_T("Total Input Exposure Time is Too high, Please check Limit Value\nWith UV & IR : ")
                          _T("500msec\nWithout UV & IR : 200msec"));

            long idx = item->GetID()
                - ITEM_ID_ILLUM_IDX; //	지금 수정한 Column ID에서 CH01 노출시간 입력 Column ID를 빼면 현재 Column을 구할 수 있음

            if (idx < 0) // 0 보다 적으면 노출시간 입력 Column이 아닌 것으로 판단하여 그냥 빠져나간다.
            {
                //
            }
            else // 그렇지 않으면 현재 해당 Channel에 설정되어 있는 값을 한계치로 쓸 수 있는 만큼 올려준다.
            {
                illumination_times_ms_live[idx] = illumination_times_ms_live[idx] - (fillumSum - fInputLimit) < 0
                    ? 0
                    : illumination_times_ms_live[idx] - (fillumSum - fInputLimit);
                CString strValue("");
                strValue.Format(_T("%.3lf ms"), illumination_times_ms_live[idx]);
                item->SetID(ITEM_ID_ILLUM_IDX + idx);
                item->SetValue(strValue);

                strValue.Empty();
            }
        }
        vecfillumCheck.clear();
    }

    return 0;
}

LRESULT CDlgSub2DVisionTune::OnGrabEndEvent(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    if (m_vision_type == VISIONTYPE_NGRV_INSP)
    {
        if (simulationMode == true)
            TestSimulation_Live();
        else
            FrameGrabber::GetInstance().get_live_image(*color_Image);

        m_imageView->SetImage(*color_Image);
    }
    else
    {
        if (simulationMode == true)
            TestSimulation_Live();
        else
            FrameGrabber::GetInstance().get_live_image(*mono_Image);

        m_imageView->SetImage(*mono_Image);
    }

    if (uniformity_live_update_mode == ITEM_ID_UNIFORMITY_LIVE_UPDATE_ON)
    {
        if (m_vision_type == VISIONTYPE_NGRV_INSP)
            CalculateVerify_color_realtime(m_vision_type, *color_Image);
        else
            CalculateVerify_mono_realtime(m_vision_type, *mono_Image);
    }
    else
    {
        view_avg_gv(m_vision_type);
    }

    return 1L;
}

void CDlgSub2DVisionTune::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 0)
    {
        scan_time++;
        CString strTime;
        int second = scan_time % 60;
        int minute = scan_time / 60 % 60;
        int hour = scan_time / 60 / 60 % 24;
        strTime.Format(_T("Scan Time: [%02d:%02d:%02d]"), hour, minute, second);
        scanTime.SetWindowText(strTime);
    }
    else if (nIDEvent == 1)
    {
        UpdateLiveButtons();
        //if (!stop_calibration)
        //{
        //    ((CXTPPropertyGridItemEnum*)grid_Function->FindItem(ITEM_ID_FUNCTION_TYPE))->SetReadOnly(1);
        //}
        //else
        //{
        //    ((CXTPPropertyGridItemEnum*)grid_Function->FindItem(ITEM_ID_FUNCTION_TYPE))->SetReadOnly(0);
        //}
    }
    CDialog::OnTimer(nIDEvent);
}

void CDlgSub2DVisionTune::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = tabChannel.GetCurSel();

    SetGridCtrl_Result(m_vision_type, view_color_image_RGB_idx, nTab);

    if (pResult)
    {
        *pResult = 0;
    }
}

void CDlgSub2DVisionTune::UpdateLiveButtons()
{
    if (isLive_mode)
    {
        grid_Calibration->FindItem(ITEM_ID_LIVE_ON)->SetHidden(TRUE);
        grid_Calibration->FindItem(ITEM_ID_LIVE_OFF)->SetHidden(FALSE);

        grid_Verify->FindItem(ITEM_ID_VERIFY_LIVE_ON)->SetHidden(TRUE);
        grid_Verify->FindItem(ITEM_ID_VERIFY_LIVE_OFF)->SetHidden(FALSE);
    }
    else
    {
        grid_Calibration->FindItem(ITEM_ID_LIVE_ON)->SetHidden(FALSE);
        grid_Calibration->FindItem(ITEM_ID_LIVE_OFF)->SetHidden(TRUE);

        grid_Verify->FindItem(ITEM_ID_VERIFY_LIVE_ON)->SetHidden(FALSE);
        grid_Verify->FindItem(ITEM_ID_VERIFY_LIVE_OFF)->SetHidden(TRUE);
    }
}

void CDlgSub2DVisionTune::SetReadOnlyButton_Scan(const BOOL i_bReadOnly)
{
    ((CCustomItemButton*)grid_Function->FindItem(ITEM_ID_SAVE_RESULT))->SetReadOnly(i_bReadOnly);
    ((CCustomItemButton*)grid_Calibration->FindItem(ITEM_ID_SCAN))->SetReadOnly(i_bReadOnly);
    ((CCustomItemButton*)grid_Calibration->FindItem(ITEM_ID_ILLUM_VERIFICATION_SI))->SetReadOnly(i_bReadOnly);
    ((CCustomItemButton*)grid_Calibration->FindItem(ITEM_ID_SAVECURRENT_ILLUMINATION_DATA))->SetReadOnly(i_bReadOnly);

    if (m_vision_type == VISIONTYPE_NGRV_INSP)
    {
        ((CCustomItemButton*)grid_Calibration->FindItem(ITEM_ID_STOP))->SetReadOnly(i_bReadOnly);
    }
}

void CDlgSub2DVisionTune::SetReadOnlyButton_Live(const BOOL i_bReadOnly)
{
    if (m_vision_type == VISIONTYPE_SIDE_INSP)
        grid_Function->FindItem(ITEM_ID_CUR_SIDE_MODULE_TYPE)->SetReadOnly(i_bReadOnly);

    ((CCustomItemButton*)grid_Calibration->FindItem(ITEM_ID_SCAN))->SetReadOnly(i_bReadOnly);
    ((CCustomItemButton*)grid_Calibration->FindItem(ITEM_ID_ILLUM_VERIFICATION_SI))->SetReadOnly(i_bReadOnly);
    ((CCustomItemButton*)grid_Calibration->FindItem(ITEM_ID_STOP))->SetReadOnly(i_bReadOnly);
}

void CDlgSub2DVisionTune::liveOnOff(const bool& enable)
{
    if (enable == true)
    {
        std::array<float, 16> illumValues_ms = {
            0,
        };
        memcpy(&illumValues_ms[0], illumination_times_ms_live, sizeof(illumination_times_ms_live));

        SyncController::GetInstance().SetIllumiParameter(
            0, illumValues_ms, false, false, IllumMirrorCalType_GrayTargetOnly, enSideVisionModule(m_cur_side_module));
        SyncController::GetInstance().TurnOnLight(0, enSideVisionModule(m_cur_side_module));

        FrameGrabber::GetInstance().live_on(
            GetSafeHwnd(), UM_GRAB_LIVE_EVENT, 500L, enSideVisionModule(m_cur_side_module));

        isLive_mode = true;
        isLiveScan_mode = false;
    }
    else
    {
        FrameGrabber::GetInstance().live_off();
        SyncController::GetInstance().TurnOffLight(enSideVisionModule(m_cur_side_module));

        isLive_mode = false;
        isLiveScan_mode = false;
    }
}

void CDlgSub2DVisionTune::change_scan_function(bool bShow)
{
    HWND hWnd_HistogramView = histogram_view->GetSafeHwnd();
    HWND hWnd_IntensityView = intensity_profile_view->GetSafeHwnd();

    if (bShow)
    {
        //auto image = m_image;
        //m_imageView->SetImage(*image);
        m_imageView->HideImageOverlay();
        m_imageView->HideWindowOverlay();
        ::ShowWindow(hWnd_HistogramView, SW_SHOW);
        ::ShowWindow(hWnd_IntensityView, SW_SHOW);
        grid_Verify->ShowWindow(!bShow);
        gridResult.ShowWindow(!bShow);
        tabChannel.ShowWindow(!bShow);
        gridSpec.ShowWindow(!bShow);
        //((CCustomItemButton*)grid_Function->FindItem(ITEM_ID_SAVE_RESULT))->SetHidden(0);
    }
    else
    {
        m_imageView->ShowImageOverlay();
        m_imageView->ShowWindowOverlay();
        ::ShowWindow(hWnd_HistogramView, SW_HIDE);
        ::ShowWindow(hWnd_IntensityView, SW_HIDE);
        grid_Verify->ShowWindow(!bShow);
        gridResult.ShowWindow(!bShow);
        tabChannel.ShowWindow(!bShow);
        gridSpec.ShowWindow(!bShow);
        SetGridCtrl_Result(m_vision_type, view_color_image_RGB_idx);
        SetGridCtrl_Spec(m_vision_type);

        //((CCustomItemButton*)grid_Function->FindItem(ITEM_ID_SAVE_RESULT))->SetHidden(TRUE);
        //((CCustomItemButton*)grid_Verify->FindItem(ITEM_ID_SCAN_VERIFY_FOR_GV))->SetReadOnly(FALSE);
        //((CCustomItemButton*)grid_Verify->FindItem(ITEM_ID_SCAN_VERIFY_FOR_MS))->SetReadOnly(FALSE);
        //((CCustomItemButton*)grid_Verify->FindItem(ITEM_ID_STOP_VERIFY))->SetReadOnly(FALSE);
    }
}

void CDlgSub2DVisionTune::SetGridCtrl_Result(const long& vision_type, const int& color_idx, const long& channel)
{
    const long data_num = vision_type == VISIONTYPE_NGRV_INSP ? (long)resultIntensities_color_gv->size()
                                                              : (long)resultIntensities_mono_gv.size();
    const long result_num
        = vision_type == VISIONTYPE_NGRV_INSP ? (long)verify_results_color_gv.size() : (long)verify_results_gv.size();

    if (data_num <= 0 || result_num <= 0)
        return;

    m_imageView->ClearImageOverlay();
    m_imageView->ClearWindowOverlay();

    CString textIdx("");
    CString textValue("");

    long rowIndex = 1;
    long roiIndex = 0;
    long nPercent = 100;

    if (channel == 0)
    {
        tabChannel.SetCurSel(channel);
    }

    // NGRV Uniformity Channel 설정
    std::vector<long> vecnChannelindex;
    vecnChannelindex.clear();

    if (vision_type == VISIONTYPE_NGRV_INSP)
    {
        // 먼저 IR, UV가 몇 번 채널을 사용하고 있는지 확인한다.
        long nChannel_UV = UV_channel_id;
        long nChannel_IR = IR_channel_id;

        for (long nIndex = 0; nIndex < use_illumation_Count; nIndex++)
        {
            // UV 및 IR은 조명 균일도를 검사하지 않으며, CH10 및 CH11은 NGRV Vision에서 사용하지 않기 때문에 뺀 나머지를 사용 가능 Channel Index로 가져간다
            if (nIndex == nChannel_UV)
            {
                continue;
            }
            if (nIndex == nChannel_IR)
            {
                continue;
            }

            vecnChannelindex.push_back(nIndex);
        }
        ////////////////////////////////////////////////////////////////

        if (verify_grabed_color_images.size() > 0)
        {
            long nRealChannel = vecnChannelindex[channel];
            m_imageView->SetImage(verify_grabed_color_images[nRealChannel]);
        }
    }
    else
    {
        if (verify_grabed_images.size() > 0)
        {
            m_imageView->SetImage(verify_grabed_images[channel]);
        }
    }

    m_imageView->ClearImageOverlay();

    const long sub_area_num_X = vision_type == VISIONTYPE_SIDE_INSP ? GRAY_TARGET_SBU_AREA_SIDE_COUNT_X
                                                                    : GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X;
    const long sub_area_num_Y = vision_type == VISIONTYPE_SIDE_INSP ? GRAY_TARGET_SBU_AREA_SIDE_COUNT_Y
                                                                    : GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y;

    gridResult.SetRowCount(sub_area_num_X + 1);
    gridResult.SetColumnCount(sub_area_num_Y + 1);

    gridResult.SetColumnWidth(0, 45);
    gridResult.SetItemText(0, 0, _T(""));
    gridResult.SetItemState(0, 0, GVIS_READONLY);
    gridResult.SetItemBkColour(0, 0, RGB(200, 200, 250));

    for (long chIDX = 0; chIDX < sub_area_num_X; chIDX++)
    {
        CString title;
        title.Format(_T("%d"), chIDX + 1);
        gridResult.SetColumnWidth(1 + chIDX, 125);
        gridResult.SetItemText(0, 1 + chIDX, title);
        gridResult.SetItemState(0, 1 + chIDX, GVIS_READONLY);
        gridResult.SetItemBkColour(0, 1 + chIDX, RGB(200, 200, 250));
    }

    // 실제 사용하는 Channel이 일반 Vision 인지, NGRV Vision인지 확인 하여 총 채널 개수 및 현재 채널을 설정한다.
    long nRealChannel = vision_type == VISIONTYPE_NGRV_INSP ? vecnChannelindex[channel] : channel;

    //Color index가 존재하면서 Color 이미지면 Color 결과를 이용한다.
    auto& calc = vision_type == VISIONTYPE_NGRV_INSP && color_idx >= 0
        ? resultIntensities_color_gv[color_idx][nRealChannel]
        : resultIntensities_mono_gv[nRealChannel];
    auto& result = vision_type == VISIONTYPE_NGRV_INSP && color_idx >= 0 ? verify_results_color_gv[color_idx]
                                                                         : verify_results_gv;
    for (long indexX = 0; indexX < sub_area_num_X; indexX++)
    {
        for (long indexY = 0; indexY < sub_area_num_Y; indexY++)
        {
            textIdx.Format(_T("%d"), rowIndex);
            gridResult.SetItemText(rowIndex, 0, textIdx);
            gridResult.SetItemState(rowIndex, 0, GVIS_READONLY);
            gridResult.SetItemBkColour(rowIndex, 0, RGB(200, 200, 250));

            //auto& calc = m_resultIntensities[n_Channel];

            //if (m_vecTotalResult.size() != 0 && m_vecTotalResult[n_Channel].m_fMaxIntensity != 0)
            if (result.size() != 0 && result[nRealChannel].intensity_max != 0)
            {
                //if (calc[roiIndex] == m_vecTotalResult[n_Channel].m_fMaxIntensity)
                if (calc[roiIndex] == result[nRealChannel].intensity_max)
                    nPercent = 100;
                else
                    nPercent = long(((calc[roiIndex] * 100) / result[nRealChannel].intensity_max));
                //nPercent = long(((calc[roiIndex] * 100) / m_vecTotalResult[n_Channel].m_fMaxIntensity));
            }
            textValue.Format(_T("%.2f(%d%%)"), calc[roiIndex], nPercent);

            if (nPercent < verify_illumination_spec_values_percent[nRealChannel])
            {
                if (result_areas.size() > 0)
                    m_imageView->AddImageOverlay(result_areas[roiIndex], RGB(255, 0, 0));

                gridResult.SetItemBkColour(1 + indexY, rowIndex, RGB(255, 0, 0));
            }
            else if (nPercent == 100 && calc[roiIndex] != 0)
            {
                if (result_areas.size() > 0)
                {
                    Ipvm::Rect32s rtMax;
                    rtMax = result_areas[roiIndex];
                    rtMax.DeflateRect(10, 10);
                    m_imageView->AddImageOverlay(rtMax, RGB(0, 255, 0));
                }

                gridResult.SetItemBkColour(1 + indexY, rowIndex, RGB(0, 255, 0));
            }
            else
            {
                gridResult.SetItemBkColour(1 + indexY, rowIndex, RGB(255, 255, 255));
            }

            // Set ROI Min Intensity
            //if (m_vecTotalResult.size() != 0 && m_vecTotalResult[n_Channel].m_fMinIntensity != 0)
            if (result.size() != 0 && result[nRealChannel].intensity_min != 0)
            {
                //if (calc[roiIndex] == m_vecTotalResult[n_Channel].m_fMinIntensity)
                if (calc[roiIndex] == result[nRealChannel].intensity_min)
                {
                    if (nPercent < verify_illumination_spec_values_percent[nRealChannel])
                    {
                        if (result_areas.size() > 0)
                        {
                            Ipvm::Rect32s rtMin;
                            rtMin = result_areas[roiIndex];
                            rtMin.DeflateRect(10, 10);
                            m_imageView->AddImageOverlay(rtMin, RGB(0, 0, 255));
                        }
                        gridResult.SetItemBkColour(1 + indexY, rowIndex, RGB(0, 0, 255));
                    }
                }
            }

            gridResult.SetItemText(1 + indexY, rowIndex, textValue);
            gridResult.SetItemState(1 + indexY, rowIndex, GVIS_READONLY);
            roiIndex++;
        }
        rowIndex++;
    }

    //if (m_vecTotalResult.size() != 0 && m_vecTotalResult[n_Channel].m_fMinIntensity != 0)
    if (result.size() != 0 && result[nRealChannel].intensity_min != 0)
    {
        CString value_min("");
        CString value_max("");
        CString valse_avg("");
        value_max.Format(_T("Max Value : %.3lf(%.1lf%%)"), result[nRealChannel].intensity_max,
            result[nRealChannel].intensity_max_percent);
        value_min.Format(_T("Min Value : %.3lf(%.1lf%%)"), result[nRealChannel].intensity_min,
            result[nRealChannel].intensity_min_percent);
        valse_avg.Format(_T("Total Avg : %.3lf"), result[nRealChannel].intensity_avg);

        m_imageView->AddWindowOverlay(Ipvm::Point32s2(10, 10), value_max, RGB(255, 255, 0), 20L);
        m_imageView->AddWindowOverlay(Ipvm::Point32s2(10, 35), value_min, RGB(255, 255, 0), 20L);
        m_imageView->AddWindowOverlay(Ipvm::Point32s2(10, 60), valse_avg, RGB(255, 255, 0), 20L);
        m_imageView->ShowWindowOverlay();

        value_min.Empty();
        value_max.Empty();
        valse_avg.Empty();
    }
    m_imageView->ShowImageOverlay();

    gridResult.Invalidate(FALSE);
}

void CDlgSub2DVisionTune::SetGridCtrl_Spec(const long& vision_type)
{
    CString textIdx("");
    CString channelReferenceValue_ms("");
    CString channelReferenceValue_gv("");
    CString specVaue("");

    gridSpec.SetRowCount(use_illumation_Count + 1);
    gridSpec.SetColumnCount(4);

    gridSpec.SetColumnWidth(0, 70);
    gridSpec.SetItemState(0, 0, GVIS_READONLY);
    gridSpec.SetItemBkColour(0, 0, RGB(200, 200, 250));

    //gridSpec.SetColumnWidth(1, 180);
    gridSpec.SetColumnWidth(1, 155);
    gridSpec.SetItemText(0, 1, _T("Illum Times(ms)"));
    gridSpec.SetItemState(0, 1, GVIS_READONLY);
    gridSpec.SetItemBkColour(0, 1, RGB(200, 200, 250));

    //gridSpec.SetColumnWidth(2, 180);
    gridSpec.SetColumnWidth(2, 105);
    gridSpec.SetItemText(0, 2, _T("Calc Intensity(gv)"));
    gridSpec.SetItemState(0, 2, GVIS_READONLY);
    gridSpec.SetItemBkColour(0, 2, RGB(200, 200, 250));

    //gridSpec.SetColumnWidth(3, 180);
    gridSpec.SetColumnWidth(3, 90);
    gridSpec.SetItemText(0, 3, _T("Spec Ratio(%)"));
    gridSpec.SetItemState(0, 3, GVIS_READONLY);
    gridSpec.SetItemBkColour(0, 3, RGB(200, 200, 250));

    for (long chIDX = 0; chIDX < use_illumation_Count; chIDX++)
    {
        channelReferenceValue_ms.Format(_T("%.2f ms"), verify_ref_illumination_ms[chIDX]);
        channelReferenceValue_gv.Format(_T("%.2f"), verify_ref_illumination_gv[chIDX]);
        specVaue.Format(_T("%.2f"), verify_illumination_spec_values_percent[chIDX]);

        textIdx.Format(_T("CH%d"), chIDX + 1);
        gridSpec.SetItemText(chIDX + 1, 0, textIdx);
        gridSpec.SetItemState(chIDX + 1, 0, GVIS_READONLY);
        gridSpec.SetItemBkColour(chIDX + 1, 0, RGB(200, 200, 250));

        if (vision_type == VISIONTYPE_NGRV_INSP)
        {
            if (chIDX == UV_channel_id)
            {
                gridSpec.SetItemText(chIDX + 1, 1, _T("Not In Use For NGRV (UV)"));
                gridSpec.SetItemState(chIDX + 1, 1, GVIS_READONLY);
                gridSpec.SetItemBkColour(chIDX + 1, 1, RGB(217, 217, 217));
                verify_ref_illumination_ms[chIDX] = 0.0;

                gridSpec.SetItemText(chIDX + 1, 2, _T("0.00"));
                gridSpec.SetItemState(chIDX + 1, 2, GVIS_READONLY);
                gridSpec.SetItemBkColour(chIDX + 1, 2, RGB(217, 217, 217));
                verify_ref_illumination_gv[chIDX] = 0.0;

                gridSpec.SetItemText(chIDX + 1, 3, _T("0.00"));
                gridSpec.SetItemState(chIDX + 1, 3, GVIS_READONLY);
                gridSpec.SetItemBkColour(chIDX + 1, 3, RGB(217, 217, 217));
                verify_illumination_spec_values_percent[chIDX] = 0.0;
            }
            else if (chIDX == IR_channel_id)
            {
                gridSpec.SetItemText(chIDX + 1, 1, _T("Not In Use For NGRV (IR)"));
                gridSpec.SetItemState(chIDX + 1, 1, GVIS_READONLY);
                gridSpec.SetItemBkColour(chIDX + 1, 1, RGB(217, 217, 217));
                verify_ref_illumination_ms[chIDX] = 0.0;

                gridSpec.SetItemText(chIDX + 1, 2, _T("0.00"));
                gridSpec.SetItemState(chIDX + 1, 2, GVIS_READONLY);
                gridSpec.SetItemBkColour(chIDX + 1, 2, RGB(217, 217, 217));
                verify_ref_illumination_gv[chIDX] = 0.0;

                gridSpec.SetItemText(chIDX + 1, 3, _T("0.00"));
                gridSpec.SetItemState(chIDX + 1, 3, GVIS_READONLY);
                gridSpec.SetItemBkColour(chIDX + 1, 3, RGB(217, 217, 217));
                verify_illumination_spec_values_percent[chIDX] = 0.0;
            }
            else
            {
                //if (chIDX == UV_channel_id)
                //{
                //    gridSpec.SetItemText(chIDX + 1, 1, _T("Not In Use For NGRV"));
                //    gridSpec.SetItemState(chIDX + 1, 1, GVIS_READONLY);
                //    gridSpec.SetItemBkColour(chIDX + 1, 1, RGB(217, 217, 217));
                //    m_scanPara.m_verifyRefIlluminationTimes_ms[chIDX] = 0.0;

                //    gridSpec.SetItemText(chIDX + 1, 2, _T("0.00"));
                //    gridSpec.SetItemState(chIDX + 1, 2, GVIS_READONLY);
                //    gridSpec.SetItemBkColour(chIDX + 1, 2, RGB(217, 217, 217));
                //    m_scanPara.m_verifyImageIntensity[chIDX] = 0.0;

                //    gridSpec.SetItemText(chIDX + 1, 3, _T("0.00"));
                //    gridSpec.SetItemState(chIDX + 1, 3, GVIS_READONLY);
                //    gridSpec.SetItemBkColour(chIDX + 1, 3, RGB(217, 217, 217));
                //    m_scanPara.m_verifySpecValues[chIDX] = 0.0;
                //}
                //else if (chIDX == IR_channel_id)
                //{
                //    gridSpec.SetItemText(chIDX + 1, 1, _T("Not In Use For NGRV"));
                //    gridSpec.SetItemState(chIDX + 1, 1, GVIS_READONLY);
                //    gridSpec.SetItemBkColour(chIDX + 1, 1, RGB(217, 217, 217));
                //    m_scanPara.m_verifyRefIlluminationTimes_ms[chIDX] = 0.0;

                //    gridSpec.SetItemText(chIDX + 1, 2, _T("0.00"));
                //    gridSpec.SetItemState(chIDX + 1, 2, GVIS_READONLY);
                //    gridSpec.SetItemBkColour(chIDX + 1, 2, RGB(217, 217, 217));
                //    m_scanPara.m_verifyImageIntensity[chIDX] = 0.0;

                //    gridSpec.SetItemText(chIDX + 1, 3, _T("0.00"));
                //    gridSpec.SetItemState(chIDX + 1, 3, GVIS_READONLY);
                //    gridSpec.SetItemBkColour(chIDX + 1, 3, RGB(217, 217, 217));
                //    m_scanPara.m_verifySpecValues[chIDX] = 0.0;
                //}
                //else
                //{
                gridSpec.SetItemText(chIDX + 1, 1, channelReferenceValue_ms);
                gridSpec.SetItemState(chIDX + 1, 1, GVIS_READONLY);
                gridSpec.SetItemBkColour(chIDX + 1, 1, RGB(217, 217, 217));

                gridSpec.SetItemText(chIDX + 1, 2, channelReferenceValue_gv);

                gridSpec.SetItemText(chIDX + 1, 3, specVaue);
                //}
            }
        }
        else
        {
            gridSpec.SetItemText(chIDX + 1, 1, channelReferenceValue_ms);
            gridSpec.SetItemState(chIDX + 1, 1, GVIS_READONLY);
            //m_gridSpec.SetItemState(chIDX + 1, 1, GVIS_READONLY);
            //m_gridSpec.SetItemBkColour(chIDX + 1, 1, RGB(217, 217, 217));

            gridSpec.SetItemText(chIDX + 1, 2, channelReferenceValue_gv);
            gridSpec.SetItemState(chIDX + 1, 2, GVIS_READONLY);

            gridSpec.SetItemText(chIDX + 1, 3, specVaue);
        }
    }

    gridSpec.Invalidate(FALSE);
}

void CDlgSub2DVisionTune::On2DGridDataChanged(NMHDR* pNotifyStruct, LRESULT* result)
{
    UNREFERENCED_PARAMETER(pNotifyStruct);

    CString str("");
    CString strchannelRef_ms("");

    auto& imageillum_ms = verify_ref_illumination_ms;
    auto& imageIntensity = verify_ref_illumination_gv;
    auto& specValues = verify_illumination_spec_values_percent;

    for (long n_chIndex = 0; n_chIndex < use_illumation_Count; n_chIndex++)
    {
        str = gridSpec.GetItemText(n_chIndex + 1, 1);
        imageillum_ms[n_chIndex] = (float)_ttof(str);

        if (imageillum_ms[n_chIndex] < 0.f)
        {
            imageillum_ms[n_chIndex] = 0.f;
        }
        else if (imageillum_ms[n_chIndex] > 65.f)
        {
            imageillum_ms[n_chIndex] = 65.f;
        }

        strchannelRef_ms.Format(_T("%.2f ms"), imageillum_ms[n_chIndex]);
        gridSpec.SetItemText(n_chIndex + 1, 1, strchannelRef_ms);

        str = gridSpec.GetItemText(n_chIndex + 1, 2);
        imageIntensity[n_chIndex] = (float)_ttof(str);

        if (imageIntensity[n_chIndex] < 0)
        {
            imageIntensity[n_chIndex] = 0;
            gridSpec.SetItemText(n_chIndex + 1, 2, _T("0"));
        }
        else if (imageIntensity[n_chIndex] > 255)
        {
            imageIntensity[n_chIndex] = 255;
            gridSpec.SetItemText(n_chIndex + 1, 2, _T("255"));
        }

        str = gridSpec.GetItemText(n_chIndex + 1, 3);
        specValues[n_chIndex] = (float)_ttof(str);

        str.Empty();
        strchannelRef_ms.Empty();
    }

    Invalidate(FALSE);

    *result = 0;
}

void CDlgSub2DVisionTune::StartScan_Uniformity_Single(const long& vision_type)
{
    if (isLive_mode == true)
        liveOnOff(false);

    m_imageView->ClearImageOverlay();

    if (vision_type == VISIONTYPE_NGRV_INSP)
    {
        verify_results_color_gv.clear();
        verify_results_color_gv.resize(color_split_idx::color_split_idx_End);

        for (long color_type = color_split_idx::color_split_idx_Start;
            color_type < color_split_idx::color_split_idx_End; color_type++)
            resultIntensities_color_gv[color_type].clear();

        verify_grabed_color_images.clear();
    }
    else
    {
        verify_results_gv.clear();
        resultIntensities_mono_gv.clear();
        verify_grabed_images.clear();
    }

    if (simulationMode == true)
    {
        TestSimulation();
        return;
    }

    auto& illum_info = illumination_times_ms_live;
    bool success(false);
    for (long channel = 0; channel < use_illumation_Count; channel++)
    {
        std::array<float, 16> illuminations_ms;
        illuminations_ms.fill(0.f);

        illuminations_ms[channel] = illum_info[channel];

        success = grab_Uniformity_images(vision_type, channel, illuminations_ms, m_cur_side_module, false);

        if (success == false)
        {
            CString error("");
            error.Format(_T("Grabed Fail %d ch"), channel);
            ::SimpleMessage(error);
            return;
        }
    }

    if (success)
    {
        if (vision_type == VISIONTYPE_NGRV_INSP)
            CalculateVerify_color(vision_type, verify_grabed_color_images);
        else
            CalculateVerify_mono(vision_type, verify_grabed_images);
    }

    SaveReport_Verify(vision_type);

    ////auto& illum_info = isusing_gv_mode == true ? verify_ref_illumination_gv : verify_ref_illumination_ms;
    //auto& illum_info = illumination_times_ms_live;
    //if (grab_Uniformity_images(vision_type, illum_info, use_illumation_Count, cur_side_module, false) == true)
    //{
    //    if (vision_type == VISIONTYPE_NGRV_INSP)
    //        CalculateVerify_color(vision_type, verify_grabed_color_images);
    //    else
    //        CalculateVerify_mono(vision_type, verify_grabed_images);
    //}

    //SaveReport_Verify(vision_type);
}

void CDlgSub2DVisionTune::StartScan_Uniformity(const long& vision_type, const bool& isusing_gv_mode)
{
    if (isLive_mode == true)
        liveOnOff(false);

    ((CCustomItemButton*)grid_Verify->FindItem(ITEM_ID_SCAN_VERIFY_FOR_GV))->SetReadOnly(TRUE);
    ((CCustomItemButton*)grid_Verify->FindItem(ITEM_ID_SCAN_VERIFY_FOR_MS))->SetReadOnly(TRUE);

    m_imageView->ClearImageOverlay();

    if (vision_type == VISIONTYPE_NGRV_INSP)
    {
        verify_results_color_gv.clear();
        verify_results_color_gv.resize(color_split_idx::color_split_idx_End);

        for (long color_type = color_split_idx::color_split_idx_Start;
            color_type < color_split_idx::color_split_idx_End; color_type++)
            resultIntensities_color_gv[color_type].clear();

        verify_grabed_color_images.clear();
    }
    else
    {
        verify_results_gv.clear();
        resultIntensities_mono_gv.clear();
        verify_grabed_images.clear();
    }

    if (simulationMode == true)
    {
        TestSimulation();
        return;
    }

    auto& illum_info = isusing_gv_mode == true ? verify_ref_illumination_gv : verify_ref_illumination_ms;
    bool success(false);
    for (long channel = 0; channel < use_illumation_Count; channel++)
    {
        std::array<float, 16> illuminations_ms;
        illuminations_ms.fill(0.f);

        illuminations_ms[channel] = illum_info[channel];

        success = grab_Uniformity_images(vision_type, channel, illuminations_ms, m_cur_side_module, isusing_gv_mode);

        if (success == false)
        {
            CString error("");
            error.Format(_T("Grabed Fail %d ch"), channel);
            ::SimpleMessage(error);
            return;
        }
    }

    if (success)
    {
        if (vision_type == VISIONTYPE_NGRV_INSP)
            CalculateVerify_color(vision_type, verify_grabed_color_images);
        else
            CalculateVerify_mono(vision_type, verify_grabed_images);
    }

    SaveReport_Verify(vision_type);
}

bool CDlgSub2DVisionTune::grab_Uniformity_images(const long& vision_type, const long& channel,
    const std::array<float, 16>& illums_ms, const int& cur_side_module, const bool& isuse_gv_mode)
{
    UNREFERENCED_PARAMETER(isuse_gv_mode);

    const int& idx_grabber = 0;
    const int& idx_camera = cur_side_module;

    //Live상황에서의 연산은 따로 만들예정
    const long image_size_X = FrameGrabber::GetInstance().get_grab_image_width();
    const long image_size_Y = FrameGrabber::GetInstance().get_grab_image_height();

    CString msgProcess("");

    //if (isuse_gv_mode == true)
    //    msgProcess.Format(_T("Illumination Intensity : %.2lf gv, Channel : %d"), illum_ms[channel], channel);
    //else
    msgProcess.Format(_T("Illumination Time : %.2lf(ms), Channel : %d"), illums_ms, channel);

    procStatus.SetWindowText(msgProcess);

    SyncController::GetInstance().SetIllumiParameter(
        0, illums_ms, false, false, false, enSideVisionModule(cur_side_module));

    switch (vision_type)
    {
        case VISIONTYPE_2D_INSP:
        case VISIONTYPE_SIDE_INSP:
        {
            Ipvm::Image8u image_mono(image_size_X, image_size_Y);
            image_mono.FillZero();
            BYTE* imageMem = image_mono.GetMem();

            SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule(cur_side_module));

            FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &imageMem, 1, nullptr);

            SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule(cur_side_module));

            FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr);

            //image Push back
            verify_grabed_images.push_back(image_mono);
        }
        break;
        case VISIONTYPE_NGRV_INSP:
        {
            Ipvm::Image8u image_mono(image_size_X, image_size_Y);
            image_mono.FillZero();
            Ipvm::Image8u3 image_color(image_size_X, image_size_Y);
            image_color.FillZero();

            BYTE* imageMem = image_mono.GetMem();

            // NGRV Grab Retry 설정 : 최대 3회 실시 - 2023.03.09_JHB
            long Idx(0);
            bool grab_success = FALSE;
            CString grab_fail_log("");

            SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

            do
            {
                FrameGrabber::GetInstance().StartGrab2D(0, 0, &imageMem, 1, nullptr);
                SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                grab_success = FrameGrabber::GetInstance().wait_grab_end(0, 0, nullptr);
                Idx++;

                grab_fail_log.Format(_T("NGRV Grab Fail_%d"), Idx);
                DevelopmentLog::AddLog(DevelopmentLog::Type::Event, grab_fail_log);
            }
            while (grab_success == FALSE && Idx < 3);

            bool bChannelReverse = channel == CHANNEL_REVERSE ? true : false;
            FrameGrabber::GetInstance().DoColorConversion(image_mono, image_color, false, bChannelReverse);

            //image push back
            verify_grabed_color_images.push_back(image_color);
        }
        break;
        default:
            return false;
    }

    msgProcess.Empty();

    return true;
}

float CDlgSub2DVisionTune::get_use_gv_illum_ms(const long& channel_id, const float& value_gv)
{
    float illumTime_ms(0.f);
    auto& sysConfig = SystemConfig::GetInstance();
    if (channel_id <= 12)
    {
        //보정 정보가 없으면...? gv로 했을때 어떻게 해야해..??
        if (sysConfig.m_vecIllumFittingRef.size() <= 0)
            return 0.f;

        auto lineEqRef = sysConfig.m_vecIllumFittingRef[channel_id];
        illumTime_ms = float((lineEqRef.m_c / lineEqRef.m_b) + value_gv) / (-lineEqRef.m_a / lineEqRef.m_b);
    }

    return illumTime_ms;
}

void CDlgSub2DVisionTune::CalculateVerify_mono_realtime(const long& vision_type, const Ipvm::Image8u& image)
{
    CalculateVerify_mono(vision_type, image);
}

void CDlgSub2DVisionTune::CalculateVerify_color_realtime(const long& vision_type, const Ipvm::Image8u3& image)
{
    CalculateVerify_color(vision_type, image);
}

void CDlgSub2DVisionTune::CalculateVerify_mono(const long& vision_type, const Ipvm::Image8u& image)
{
    if (image.GetMem() == nullptr)
        return;

    m_imageView->ClearImageOverlay();

    static const long calc_sub_area_count_X = vision_type == VISIONTYPE_SIDE_INSP
        ? GRAY_TARGET_SBU_AREA_SIDE_COUNT_X
        : GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X;

    static const long calc_sub_area_count_Y = vision_type == VISIONTYPE_SIDE_INSP
        ? GRAY_TARGET_SBU_AREA_SIDE_COUNT_Y
        : GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y;
    static const long calc_diflate_size_X = vision_type == VISIONTYPE_SIDE_INSP ? DIFLATE_FROM_FULL_IMAGE_X_SIDE_PX
                                                                                : DIFLATE_FROM_FULL_IMAGE_X_DEFAULT_PX;

    static const long calc_diflate_size_Y = vision_type == VISIONTYPE_SIDE_INSP ? DIFLATE_FROM_FULL_IMAGE_Y_SIDE_PX
                                                                                : DIFLATE_FROM_FULL_IMAGE_Y_DEFAULT_PX;

    std::vector<Ipvm::Rect32s> calc_area;
    VerifyResult vec_Result;

    double tmpMax(0);
    double tmpMin(1000);
    long maxPainX(0);
    long maxPainY(0);
    long minPainX(0);
    long minPainY(0);

    Ipvm::Image8u tmp_calc_image = image;

    CalcSubAreas(vision_type, tmp_calc_image, calc_sub_area_count_X, calc_sub_area_count_Y, calc_diflate_size_X,
        calc_diflate_size_Y, calc_area);

    for (long index = 0; index < long(calc_area.size()); index++)
    {
        Ipvm::Image8u calcImage(tmp_calc_image, calc_area[index]);

        double meanValue(0.);

        Ipvm::ImageProcessing::GetMean(calcImage, Ipvm::Rect32s(calcImage), meanValue);

        if (tmpMax < meanValue)
        {
            maxPainX = long((index + calc_sub_area_count_X) / calc_sub_area_count_X);
            maxPainY = long((index + calc_sub_area_count_X + 1) % calc_sub_area_count_X);
            tmpMax = meanValue;
        }

        if (tmpMin > meanValue)
        {
            minPainX = long((index + calc_sub_area_count_Y) / calc_sub_area_count_Y);
            minPainY = long((index + calc_sub_area_count_Y + 1) % calc_sub_area_count_Y);
            tmpMin = meanValue;
        }
    }

    double avgValue = 0.;
    Ipvm::ImageProcessing::GetMean(tmp_calc_image, Ipvm::Rect32s(tmp_calc_image), avgValue);
    vec_Result.intensity_avg = avgValue;

    if (maxPainY == 0)
        maxPainY = 10;
    vec_Result.intensity_max = tmpMax;
    vec_Result.intensity_max_percent = 100.;
    vec_Result.pane_X_max = maxPainX;
    vec_Result.pane_Y_max = maxPainY;

    if (minPainY == 0)
        minPainY = 10;
    vec_Result.intensity_min = tmpMin;
    vec_Result.intensity_min_percent = long((tmpMin * 100) / tmpMax);
    vec_Result.pane_X_min = minPainX;
    vec_Result.pane_Y_min = minPainY;

    //if (vec_Result.intensity_min_percent < verify_illumination_spec_values_percent[channelIndex])
    //{
    //    vec_Result.isPass = false;
    //}
    //else
    //{
    //    vec_Result.isPass = true;
    //}

    for (auto rtUniformity : calc_area)
        m_imageView->AddImageOverlay(rtUniformity, RGB(0, 0, 255));

    m_imageView->ShowImageOverlay();
}

void CDlgSub2DVisionTune::CalculateVerify_color(const long& vision_type, const Ipvm::Image8u3& image)
{
    m_imageView->ClearImageOverlay();

    static const long calc_sub_area_count_X = GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X;
    static const long calc_sub_area_count_Y = GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y;

    static const long calc_diflate_size_X = DIFLATE_FROM_FULL_IMAGE_X_DEFAULT_PX;
    static const long calc_diflate_size_Y = DIFLATE_FROM_FULL_IMAGE_Y_DEFAULT_PX;

    std::vector<Ipvm::Rect32s> calc_area;
    VerifyResult vec_Result;

    double tmpMax(0);
    double tmpMin(1000);
    long maxPainX(0);
    long maxPainY(0);
    long minPainX(0);
    long minPainY(0);

    //Color image면 여기서 Split한다
    Ipvm::Image8u tmp_calc_image;

    //예외처리일시 어떻게 할까나..
    if (!GetSplitGrayImage(image, color_split_idx(view_color_image_RGB_idx), tmp_calc_image))
        return;

    CalcSubAreas(vision_type, tmp_calc_image, calc_sub_area_count_X, calc_sub_area_count_Y, calc_diflate_size_X,
        calc_diflate_size_Y, result_areas);

    for (long index = 0; index < long(result_areas.size()); index++)
    {
        Ipvm::Image8u calcImage(tmp_calc_image, result_areas[index]);

        double meanValue(0.);

        Ipvm::ImageProcessing::GetMean(calcImage, Ipvm::Rect32s(calcImage), meanValue);

        if (tmpMax < meanValue)
        {
            maxPainX = long((index + calc_sub_area_count_X) / calc_sub_area_count_X);
            maxPainY = long((index + calc_sub_area_count_X + 1) % calc_sub_area_count_X);
            tmpMax = meanValue;
        }

        if (tmpMin > meanValue)
        {
            minPainX = long((index + calc_sub_area_count_Y) / calc_sub_area_count_Y);
            minPainY = long((index + calc_sub_area_count_Y + 1) % calc_sub_area_count_Y);
            tmpMin = meanValue;
        }
    }

    double avgValue = 0.;
    Ipvm::ImageProcessing::GetMean(tmp_calc_image, Ipvm::Rect32s(tmp_calc_image), avgValue);
    vec_Result.intensity_avg = avgValue;

    if (maxPainY == 0)
        maxPainY = 10;
    vec_Result.intensity_max = tmpMax;
    vec_Result.intensity_max_percent = 100.;
    vec_Result.pane_X_max = maxPainX;
    vec_Result.pane_Y_max = maxPainY;

    if (minPainY == 0)
        minPainY = 10;
    vec_Result.intensity_min = tmpMin;
    vec_Result.intensity_min_percent = long((tmpMin * 100) / tmpMax);
    vec_Result.pane_X_min = minPainX;
    vec_Result.pane_Y_min = minPainY;

    //if (vec_Result.intensity_min_percent < verify_illumination_spec_values_percent[channelIndex])
    //{
    //    vec_Result.isPass = false;
    //}
    //else
    //{
    //    vec_Result.isPass = true;
    //}

    for (auto rtUniformity : calc_area)
        m_imageView->AddImageOverlay(rtUniformity, RGB(0, 0, 255));

    m_imageView->ShowImageOverlay();
}

void CDlgSub2DVisionTune::CalculateVerify_mono(const long& vision_type, const std::vector<Ipvm::Image8u>& images)
{
    m_imageView->ClearImageOverlay();

    result_areas.clear();

    const long calc_image_num = (long)images.size();
    const long calc_sub_area_count_X = vision_type == VISIONTYPE_SIDE_INSP ? GRAY_TARGET_SBU_AREA_SIDE_COUNT_X
                                                                           : GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X;

    const long calc_sub_area_count_Y = vision_type == VISIONTYPE_SIDE_INSP ? GRAY_TARGET_SBU_AREA_SIDE_COUNT_Y
                                                                           : GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y;
    const long calc_diflate_size_X = vision_type == VISIONTYPE_SIDE_INSP ? DIFLATE_FROM_FULL_IMAGE_X_SIDE_PX
                                                                         : DIFLATE_FROM_FULL_IMAGE_X_DEFAULT_PX;

    const long calc_diflate_size_Y = vision_type == VISIONTYPE_SIDE_INSP ? DIFLATE_FROM_FULL_IMAGE_Y_SIDE_PX
                                                                         : DIFLATE_FROM_FULL_IMAGE_Y_DEFAULT_PX;

    for (long channelIndex = 0; channelIndex < calc_image_num; channelIndex++)
    {
        VerifyResult vec_Result;

        double tmpMax(0);
        double tmpMin(1000);
        long maxPainX(0);
        long maxPainY(0);
        long minPainX(0);
        long minPainY(0);

        Ipvm::Image8u tmp_calc_image = images[channelIndex];

        CalcSubAreas(vision_type, tmp_calc_image, calc_sub_area_count_X, calc_sub_area_count_Y, calc_diflate_size_X,
            calc_diflate_size_Y, result_areas);

        for (long index = 0; index < long(result_areas.size()); index++)
        {
            Ipvm::Image8u calcImage(tmp_calc_image, result_areas[index]);

            double meanValue(0.);

            Ipvm::ImageProcessing::GetMean(calcImage, Ipvm::Rect32s(calcImage), meanValue);

            resultIntensities_mono_gv[channelIndex][index] = meanValue;

            if (tmpMax < meanValue)
            {
                maxPainX = long((index + calc_sub_area_count_X) / calc_sub_area_count_X);
                maxPainY = long((index + calc_sub_area_count_X + 1) % calc_sub_area_count_X);
                tmpMax = meanValue;
            }

            if (tmpMin > meanValue)
            {
                minPainX = long((index + calc_sub_area_count_Y) / calc_sub_area_count_Y);
                minPainY = long((index + calc_sub_area_count_Y + 1) % calc_sub_area_count_Y);
                tmpMin = meanValue;
            }
        }

        double avgValue = 0.;
        Ipvm::ImageProcessing::GetMean(tmp_calc_image, Ipvm::Rect32s(tmp_calc_image), avgValue);
        vec_Result.intensity_avg = avgValue;

        if (maxPainY == 0)
            maxPainY = 10;
        vec_Result.intensity_max = tmpMax;
        vec_Result.intensity_max_percent = 100.;
        vec_Result.pane_X_max = maxPainX;
        vec_Result.pane_Y_max = maxPainY;

        if (minPainY == 0)
            minPainY = 10;
        vec_Result.intensity_min = tmpMin;
        vec_Result.intensity_min_percent = long((tmpMin * 100) / tmpMax);
        vec_Result.pane_X_min = minPainX;
        vec_Result.pane_Y_min = minPainY;

        if (vec_Result.intensity_min_percent < verify_illumination_spec_values_percent[channelIndex])
        {
            vec_Result.isPass = false;
        }
        else
        {
            vec_Result.isPass = true;
        }

        verify_results_gv.push_back(vec_Result);
    }

    SetGridCtrl_Result(vision_type, 0);

    for (auto rtUniformity : result_areas)
        m_imageView->AddImageOverlay(rtUniformity, RGB(0, 0, 255));

    m_imageView->ShowImageOverlay();
}

void CDlgSub2DVisionTune::CalculateVerify_color(const long& vision_type, const std::vector<Ipvm::Image8u3>& images)
{
    m_imageView->ClearImageOverlay();

    result_areas.clear();

    const long calc_image_num = (long)images.size();

    const long calc_color_num = color_split_idx::color_split_idx_End;
    const long calc_sub_area_count_X = GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X;
    const long calc_sub_area_count_Y = GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y;

    const long calc_diflate_size_X = DIFLATE_FROM_FULL_IMAGE_X_DEFAULT_PX;
    const long calc_diflate_size_Y = DIFLATE_FROM_FULL_IMAGE_Y_DEFAULT_PX;

    for (long channelIndex = 0; channelIndex < calc_image_num; channelIndex++)
    {
        for (long calc_color_idx = 0; calc_color_idx < calc_color_num; calc_color_idx++)
        {
            VerifyResult vec_Result;

            double tmpMax(0);
            double tmpMin(1000);
            long maxPainX(0);
            long maxPainY(0);
            long minPainX(0);
            long minPainY(0);

            //Color image면 여기서 Split한다
            Ipvm::Image8u tmp_calc_image;

            //예외처리일시 어떻게 할까나..
            if (!GetSplitGrayImage(images[channelIndex], color_split_idx(calc_color_idx), tmp_calc_image))
                continue;

            CalcSubAreas(vision_type, tmp_calc_image, calc_sub_area_count_X, calc_sub_area_count_Y, calc_diflate_size_X,
                calc_diflate_size_Y, result_areas);

            for (long index = 0; index < long(result_areas.size()); index++)
            {
                Ipvm::Image8u calcImage(tmp_calc_image, result_areas[index]);

                double meanValue(0.);

                Ipvm::ImageProcessing::GetMean(calcImage, Ipvm::Rect32s(calcImage), meanValue);

                resultIntensities_color_gv[color_split_idx(calc_color_idx)][channelIndex][index] = meanValue;

                if (tmpMax < meanValue)
                {
                    maxPainX = long((index + calc_sub_area_count_X) / calc_sub_area_count_X);
                    maxPainY = long((index + calc_sub_area_count_X + 1) % calc_sub_area_count_X);
                    tmpMax = meanValue;
                }

                if (tmpMin > meanValue)
                {
                    minPainX = long((index + calc_sub_area_count_Y) / calc_sub_area_count_Y);
                    minPainY = long((index + calc_sub_area_count_Y + 1) % calc_sub_area_count_Y);
                    tmpMin = meanValue;
                }
            }

            double avgValue = 0.;
            Ipvm::ImageProcessing::GetMean(tmp_calc_image, Ipvm::Rect32s(tmp_calc_image), avgValue);
            vec_Result.intensity_avg = avgValue;

            if (maxPainY == 0)
                maxPainY = 10;
            vec_Result.intensity_max = tmpMax;
            vec_Result.intensity_max_percent = 100.;
            vec_Result.pane_X_max = maxPainX;
            vec_Result.pane_Y_max = maxPainY;

            if (minPainY == 0)
                minPainY = 10;
            vec_Result.intensity_min = tmpMin;
            vec_Result.intensity_min_percent = long((tmpMin * 100) / tmpMax);
            vec_Result.pane_X_min = minPainX;
            vec_Result.pane_Y_min = minPainY;

            if (vec_Result.intensity_min_percent < verify_illumination_spec_values_percent[channelIndex])
            {
                vec_Result.isPass = false;
            }
            else
            {
                vec_Result.isPass = true;
            }

            verify_results_color_gv[calc_color_idx].push_back(vec_Result);
        }
    }

    SetGridCtrl_Result(vision_type, view_color_image_RGB_idx);

    for (auto rtUniformity : result_areas)
        m_imageView->AddImageOverlay(rtUniformity, RGB(0, 0, 255));

    m_imageView->ShowImageOverlay();
}

void CDlgSub2DVisionTune::CalcSubAreas(const long& vision_type, const Ipvm::Image8u& image, long areaCountX,
    long areaCountY, long deflateValueX, long deflateValueY, std::vector<Ipvm::Rect32s>& rois)
{
    Ipvm::Image8u CalcAreaimage;

    // deflateValueX, deflateValueY 영상 사이즈를 deflate한다
    long rectDeflateX = deflateValueX * 2;
    long rectDeflateY = deflateValueY * 2;

    const long rectSizeY = long((image.GetSizeY() - rectDeflateY) / areaCountY);
    const long rectSizeX = long((image.GetSizeX() - rectDeflateX) / areaCountX);

    rois.clear();

    bool bNeedimageResize = GetNeedimageResize(image, areaCountX, areaCountY);

    if (bNeedimageResize == true)
    {
        Resizeimage(image, areaCountX, areaCountY, CalcAreaimage);
    }
    else
        CalcAreaimage = image;

    const long calc_image_Size_X = CalcAreaimage.GetSizeX();
    const long calc_image_Size_Y = CalcAreaimage.GetSizeY();

    if (vision_type == VISIONTYPE_NGRV_INSP)
    {
        for (long nRectX = deflateValueX; nRectX < (calc_image_Size_X - deflateValueX); nRectX += rectSizeX)
        {
            for (long nRectY = deflateValueY; nRectY < (calc_image_Size_Y - deflateValueY); nRectY += rectSizeY)
            {
                rois.push_back(Ipvm::Rect32s(
                    (int32_t)nRectX, (int32_t)nRectY, (int32_t)(nRectX + rectSizeX), (int32_t)(nRectY + rectSizeY)));
            }
        }
    }
    else if (vision_type == VISIONTYPE_SIDE_INSP)
    {
        for (long nRectY = deflateValueY; nRectY < (calc_image_Size_Y - deflateValueY); nRectY += rectSizeY)
        {
            for (long nRectX = deflateValueX; nRectX < (calc_image_Size_X - deflateValueX); nRectX += rectSizeX)
            {
                rois.push_back(Ipvm::Rect32s(
                    (int32_t)nRectX, (int32_t)nRectY, (int32_t)(nRectX + rectSizeX), (int32_t)(nRectY + rectSizeY)));
            }
        }
    }
    else
    {
        for (long nRectY = deflateValueY; nRectY < (calc_image_Size_Y - deflateValueY); nRectY += rectSizeY)
        {
            for (long nRectX = deflateValueX; nRectX < (calc_image_Size_X - deflateValueX); nRectX += rectSizeX)
            {
                rois.push_back(Ipvm::Rect32s(
                    (int32_t)nRectY, (int32_t)nRectX, (int32_t)(nRectY + rectSizeY), (int32_t)(nRectX + rectSizeX)));
            }
        }
    }
}

bool CDlgSub2DVisionTune::GetNeedimageResize(const Ipvm::Image8u& image, const long& division_X, const long& division_Y)
{
    //나머지 몫이 0이 아니면 Resize가 필요한 것이다..
    float fRemainderX_Val = static_cast<float>(abs(image.GetSizeX() % division_X));
    float fRemainderY_Val = static_cast<float>(abs(image.GetSizeY() % division_Y));

    if (fRemainderX_Val > 0.f || fRemainderY_Val > 0.f)
        return true;
    else
        return false;
}

bool CDlgSub2DVisionTune::GetNeedimageResize(
    const Ipvm::Image8u3& image, const long& division_X, const long& division_Y)
{
    //나머지 몫이 0이 아니면 Resize가 필요한 것이다..
    float fRemainderX_Val = static_cast<float>(abs(image.GetSizeX() % division_X));
    float fRemainderY_Val = static_cast<float>(abs(image.GetSizeY() % division_Y));

    if (fRemainderX_Val >= 0.f || fRemainderY_Val >= 0.f)
        return true;
    else
        return false;
}

bool CDlgSub2DVisionTune::Resizeimage(
    const Ipvm::Image8u& image, const long& division_X, const long division_Y, Ipvm::Image8u& o_ResizeImage)
{
    //나머지 몫이 0이 아니면 Size값에서 빼준다
    float fRemainderX_Val = static_cast<float>(image.GetSizeX() % division_X);
    float fRemainderY_Val = static_cast<float>(image.GetSizeY() % division_Y);

    long nResizeimageSIzeX = (long)(image.GetSizeX() - fRemainderX_Val);
    long nResizeimageSIzeY = (long)(image.GetSizeY() - fRemainderY_Val);

    Ipvm::Image8u resizeImage(nResizeimageSIzeX, nResizeimageSIzeY);
    resizeImage.FillZero();

    if (Ipvm::ImageProcessing::ResizeLinearInterpolation(image, resizeImage) != Ipvm::Status::e_ok)
        return false;

    o_ResizeImage = resizeImage;

    return true;
}

bool CDlgSub2DVisionTune::Resizeimage(
    const Ipvm::Image8u3& image, const long& division_X, const long division_Y, Ipvm::Image8u3& o_ResizeImage)
{
    //나머지 몫이 0이 아니면 Size값에서 빼준다
    float fRemainderX_Val = static_cast<float>(image.GetSizeX() % division_X);
    float fRemainderY_Val = static_cast<float>(image.GetSizeY() % division_Y);

    long nResizeimageSIzeX = (long)(image.GetSizeX() - fRemainderX_Val);
    long nResizeimageSIzeY = (long)(image.GetSizeY() - fRemainderY_Val);

    Ipvm::Image8u3 resizeImage(nResizeimageSIzeX, nResizeimageSIzeY);
    resizeImage.FillZero();

    if (Ipvm::ImageProcessing::ResizeLinearInterpolation(image, resizeImage) != Ipvm::Status::e_ok)
        return false;

    o_ResizeImage = resizeImage;

    return true;
}

bool CDlgSub2DVisionTune::GetSplitGrayImage(
    const Ipvm::Image8u3& image_Color, const color_split_idx& spit_color_idx, Ipvm::Image8u& o_splitimage)
{
    const Ipvm::Point32s2 ptImageSize(image_Color.GetSizeX(), image_Color.GetSizeY());

    Ipvm::Image8u image_red(ptImageSize.m_x, ptImageSize.m_y);
    Ipvm::Image8u image_green(ptImageSize.m_x, ptImageSize.m_y);
    Ipvm::Image8u image_blue(ptImageSize.m_x, ptImageSize.m_y);
    Ipvm::Image8u image_Copy(ptImageSize.m_x, ptImageSize.m_y);

    //Initlize
    image_red.FillZero();
    image_green.FillZero();
    image_blue.FillZero();
    image_Copy.FillZero();

    if (Ipvm::ImageProcessing::SplitRGB(image_Color, Ipvm::Rect32s(image_Color), image_red, image_green, image_blue)
        != Ipvm::Status::e_ok)
        return false;

    auto splitimage = image_green;

    switch ((long)spit_color_idx)
    {
        case color_split_idx::color_split_idx_Red:
            splitimage = image_red;
            break;
        case color_split_idx::color_split_idx_Green:
            splitimage = image_green;
            break;
        case color_split_idx::color_split_idx_Blue:
            splitimage = image_blue;
            break;
        default:
            return false;
    }

    Ipvm::ImageProcessing::Copy(splitimage, Ipvm::Rect32s(splitimage), image_Copy);

    o_splitimage = image_Copy;

    return true;
}

void CDlgSub2DVisionTune::SaveReport_Verify(const long& vision_type)
{
    UNREFERENCED_PARAMETER(vision_type);
    static const bool isNGRV = static_cast<bool>(SystemConfig::GetInstance().IsVisionTypeNGRV());

    static LPCTSTR g_probeNames[]
        = {_T("BOTTOM_2D"), _T("BOTTOM_3D"), _T("TOP_2D"), _T("TOP_3D"), _T("NGRVorSIDE_1"), _T("SIDE_2")};

    // Report 폴더가 없으면 생성
    if (GetFileAttributes(DynamicSystemPath::get(DefineFolder::Log)) == INVALID_FILE_ATTRIBUTES)
    {
        Ipvm::CreateDirectories(DynamicSystemPath::get(DefineFolder::Log));
    }

    SYSTEMTIME time;
    GetLocalTime(&time);

    CString strFullTime;
    strFullTime.Format(_T("%04d/%02d/%02d %02d:%02d:%02d"), time.wYear, time.wMonth, time.wDay, time.wHour,
        time.wMinute, time.wSecond);

    CString strTimeFileName;
    strTimeFileName.Format(
        _T("_%04d%02d%02d_%02d%02d%02d"), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

    CString folderPath = DynamicSystemPath::get(DefineFolder::Log);
    CString pathName(
        folderPath + _T("VerifyIllum_") + SystemConfig::GetInstance().m_strVisionInfo + strTimeFileName + _T(".csv"));

    if (isNGRV)
        SaveReport_Uniformity_Color(pathName, strFullTime);
    else
        SaveReport_Uniformity_Gray(pathName, strFullTime);
}

void CDlgSub2DVisionTune::SaveReport_Uniformity_Gray(const CString i_strSavePath, const CString i_strSaveTime)
{
    CString strSavePath = i_strSavePath;
    CString strSaveFullTime = i_strSaveTime;

    CString strResult = _T("Pass");
    bool bResult = true;
    for (long chIDX = 0; chIDX < use_illumation_Count; chIDX++)
    {
        if (!verify_results_gv[chIDX].isPass)
        {
            strResult = _T("Reject");
            bResult = false;
            break;
        }
    }

    const long sub_area_num_X = m_vision_type == VISIONTYPE_SIDE_INSP ? GRAY_TARGET_SBU_AREA_SIDE_COUNT_X
                                                                      : GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X;
    const long sub_area_num_Y = m_vision_type == VISIONTYPE_SIDE_INSP ? GRAY_TARGET_SBU_AREA_SIDE_COUNT_Y
                                                                      : GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y;

    std::ofstream stream(strSavePath.GetBuffer());
    stream << "Date: " << CStringA(strSaveFullTime).GetBuffer();
    stream << std::endl;
    stream << "Total Result :" << CStringA(strResult).GetBuffer();
    stream << std::endl;
    stream << ","
           << "Uniformity Result : " << CStringA(strResult).GetBuffer();
    if (!bResult)
    {
        stream << "  Ch[ ";
        CString strRejectChannels = _T("");
        for (long chIDX = 0; chIDX < use_illumation_Count; chIDX++)
        {
            if (!verify_results_gv[chIDX].isPass)
            {
                CString strTmp = _T("");
                if (chIDX == use_illumation_Count - 1)
                {
                    strTmp.Format(_T("%d"), chIDX + 1);
                    strRejectChannels += strTmp;
                }
                else
                {
                    strTmp.Format(_T("%d "), chIDX + 1);
                    strRejectChannels += strTmp;
                }
            }
        }
        stream << CStringA(strRejectChannels).GetBuffer();
        stream << " ]";
    }
    stream << std::endl;
    stream << std::endl;

    for (long channel = 0; channel < use_illumation_Count; channel++)
    {
        strResult = _T("");
        if (verify_results_gv[channel].isPass)
        {
            strResult = _T("Pass");
        }
        else
        {
            strResult = _T("Reject");
        }
        stream << "[CH_" << (channel + 1) << "] Result : " << CStringA(strResult).GetBuffer();
        stream << std::endl;
        stream << ",Uniformity Result : " << CStringA(strResult).GetBuffer();
        stream << std::endl;
        stream << ",,Spec : " << verify_illumination_spec_values_percent[channel] << "%";
        stream << std::endl;
        stream << ",,Total Average Intensity : " << verify_results_gv[channel].intensity_avg;
        stream << std::endl;
        stream << ",,Min : Pane ( x:" << verify_results_gv[channel].pane_X_min
               << " y:" << verify_results_gv[channel].pane_Y_min << " )   " << verify_results_gv[channel].intensity_min
               << "( " << verify_results_gv[channel].intensity_min_percent << "% )";
        stream << std::endl;
        stream << ",,Max : Pane ( x:" << verify_results_gv[channel].pane_X_max
               << " y:" << verify_results_gv[channel].pane_Y_max << " )   " << verify_results_gv[channel].intensity_max
               << "( 100% )";
        stream << std::endl;

        long roiIndex = 0;
        stream << ",,,Each ROI Intensity map,,,,,,,,,,,,Each ROI Intensity ratio map";
        stream << std::endl;
        stream << ",,";
        for (long index = 0; index < sub_area_num_X; index++)
        {
            stream << "," << index + 1;
        }
        stream << ",,";
        for (long index = 0; index < sub_area_num_X; index++)
        {
            stream << "," << index + 1;
        }
        stream << std::endl;

        for (long roiX = 0; roiX < sub_area_num_X; roiX++)
        {
            stream << ",,";
            stream << roiX + 1;
            for (long roiY = 0; roiY < sub_area_num_Y; roiY++)
            {
                auto& calc = resultIntensities_mono_gv[channel];

                stream << "," << calc[roiX + (roiY * sub_area_num_Y)];
                roiIndex++;
            }

            stream << ",," << roiX + 1;
            for (long roiY = 0; roiY < sub_area_num_Y; roiY++)
            {
                auto& calc = resultIntensities_mono_gv[channel];

                long percent
                    = long((calc[roiX + (roiY * sub_area_num_Y)] * 100) / verify_results_gv[channel].intensity_max);
                stream << "," << percent << "%";
                roiIndex++;
            }
            stream << std::endl;
        }

        stream << std::endl;
        stream << std::endl;
    }
    stream << std::endl;

    stream.close();
}

void CDlgSub2DVisionTune::SaveReport_Uniformity_Color(const CString i_strSavePath, const CString i_strSaveTime)
{
    CString strSavePath = i_strSavePath;
    CString strSaveFullTime = i_strSaveTime;

    CString strResult = _T("Pass");
    bool bResult = true;
    for (long chIDX = 0; chIDX < use_illumation_Count; chIDX++)
    {
        //Ignore은 Channel은 제외하고 결과를 취합하여야 한다
        if (chIDX == UV_channel_id || chIDX == IR_channel_id)
        {
            continue;
        }

        //모든 Channel에 대하여 검색하여야 함
        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            if (!verify_results_color_gv[nColoridx][chIDX].isPass)
            {
                strResult = _T("Reject");
                bResult = false;
                break;
            }
        }
    }

    std::ofstream stream(strSavePath.GetBuffer());
    stream << "Date: " << CStringA(strSaveFullTime).GetBuffer();
    stream << std::endl;
    stream << "Total Result :" << CStringA(strResult).GetBuffer();
    stream << std::endl;
    stream << ","
           << "Uniformity Result : " << CStringA(strResult).GetBuffer();
    if (!bResult)
    {
        stream << "  Ch[ ";
        CString strRejectChannels = _T("");
        for (long chIDX = 0; chIDX < use_illumation_Count; chIDX++)
        {
            //Ignore은 Channel은 제외하고 결과를 취합하여야 한다
            if (chIDX == UV_channel_id || chIDX == IR_channel_id)
            {
                continue;
            }

            for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
                nColoridx++)
            {
                if (!verify_results_color_gv[nColoridx][chIDX].isPass)
                {
                    CString strColorname("");
                    switch (nColoridx)
                    {
                        case ColorResultID::ColorResultID_Red:
                            strColorname = _T("[Red]");
                            break;
                        case ColorResultID::ColorResultID_Green:
                            strColorname = _T("[Green]");
                            break;
                        case ColorResultID::ColorResultID_Blue:
                            strColorname = _T("[Blue]");
                            break;
                        default:
                            break;
                    }
                    CString strTmp = _T("");
                    if (chIDX == use_illumation_Count - 1)
                    {
                        strTmp.Format(_T("%d%s"), chIDX + 1, (LPCTSTR)strColorname);
                        strRejectChannels += strTmp;
                    }
                    else
                    {
                        strTmp.Format(_T("%d%s "), chIDX + 1, (LPCTSTR)strColorname);
                        strRejectChannels += strTmp;
                    }
                }
            }
        }

        stream << CStringA(strRejectChannels).GetBuffer();
        stream << " ]";
    }
    stream << std::endl;
    stream << std::endl;

    for (long channel = 0; channel < use_illumation_Count; channel++)
    {
        CString ArrstrChannelResult[color_split_idx::color_split_idx_End] = {
            _T(""),
        };

        //Ignore은 Channel은 제외하고 결과를 취합하여야 한다
        if (channel == UV_channel_id || channel == IR_channel_id)
        {
            continue;
        }

        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            CString strChannelResult = _T("");
            if (verify_results_color_gv[nColoridx][channel].isPass)
                strChannelResult = _T("Pass");
            else
                strChannelResult = _T("Reject");

            CString strColor = _T("");
            if (nColoridx == ColorResultID::ColorResultID_Red)
            {
                strColor = _T("Red");
            }
            else if (nColoridx == ColorResultID::ColorResultID_Green)
            {
                strColor = _T("Green");
            }
            else if (nColoridx == ColorResultID::ColorResultID_Blue)
            {
                strColor = _T("Blue");
            }
            else
            {
                strColor = _T("");
            }
            stream << "[CH_" << (channel + 1) << CStringA(strColor).GetBuffer()
                   << "] Result : " << CStringA(strChannelResult).GetBuffer() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,";

            ArrstrChannelResult[nColoridx] = strChannelResult;
        }
        stream << std::endl;

        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            stream << ",Uniformity Result : " << CStringA(ArrstrChannelResult[nColoridx]).GetBuffer()
                   << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,";
        }
        stream << std::endl;
        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            stream << ",,Spec : " << verify_illumination_spec_values_percent[channel] << "%"
                   << ",,,,,,,,,,,,,,,,,,,,,,,,,,,";
        }
        stream << std::endl;
        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            stream << ",,Total Average Intensity : " << verify_results_color_gv[nColoridx][channel].intensity_avg
                   << ",,,,,,,,,,,,,,,,,,,,,,,,,,,";
        }
        stream << std::endl;
        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            stream << ",,Min : Pane ( x:" << verify_results_color_gv[nColoridx][channel].pane_X_min
                   << " y:" << verify_results_color_gv[nColoridx][channel].pane_Y_min << " )   "
                   << verify_results_color_gv[nColoridx][channel].intensity_min << "( "
                   << verify_results_color_gv[nColoridx][channel].intensity_min_percent << "% )"
                   << ",,,,,,,,,,,,,,,,,,,,,,,,,,,";
        }
        stream << std::endl;
        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            stream << ",,Max : Pane ( x:" << verify_results_color_gv[nColoridx][channel].pane_X_max
                   << " y:" << verify_results_color_gv[nColoridx][channel].pane_Y_max << " )   "
                   << verify_results_color_gv[nColoridx][channel].intensity_max << "( 100% )"
                   << ",,,,,,,,,,,,,,,,,,,,,,,,,,,";
        }
        stream << std::endl;

        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            stream << ",,,Each ROI Intensity map,,,,,,,,,,,,Each ROI Intensity ratio map,,,,,,,,,,,,,,";
        }
        stream << std::endl;
        for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
            nColoridx++)
        {
            stream << ",,";
            for (long index = 0; index < GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X; index++)
            {
                stream << "," << index + 1;
            }
            stream << ",,";
            for (long index = 0; index < GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X; index++)
            {
                stream << "," << index + 1;
            }
            stream << ",,,,,";
        }
        stream << std::endl;

        long roiIndex = 0;
        for (long roiX = 0; roiX < GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_X; roiX++)
        {
            for (long nColoridx = ColorResultID::ColorResultID_Red; nColoridx <= ColorResultID::ColorResultID_Blue;
                nColoridx++)
            {
                stream << ",,";
                stream << roiX + 1;
                for (long roiY = 0; roiY < GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y; roiY++)
                {
                    auto& calc = resultIntensities_color_gv[nColoridx][channel];

                    stream << "," << calc[roiX + (roiY * GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y)];
                    roiIndex++;
                }

                stream << ",," << roiX + 1;
                for (long roiY = 0; roiY < GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y; roiY++)
                {
                    auto& calc = resultIntensities_color_gv[nColoridx][channel];

                    long percent = long((calc[roiX + (roiY * GRAY_TARGET_SBU_AREA_DEFAULT_COUNT_Y)] * 100)
                        / verify_results_color_gv[nColoridx][channel].intensity_max);
                    stream << "," << percent << "%";
                    roiIndex++;
                }
                stream << ",,,,,";
            }
            stream << std::endl;
        }

        stream << std::endl;
        stream << std::endl;
    }
    stream << std::endl;

    stream.close();
}

void CDlgSub2DVisionTune::TestSimulation()
{
    long test_imgae_size_X(0);
    long test_imgae_size_Y(0);

    switch (m_vision_type)
    {
        case VISIONTYPE_2D_INSP:
        {
            test_imgae_size_X = 5120;
            test_imgae_size_Y = 5120;
        }
        break;
        case VISIONTYPE_NGRV_INSP:
        {
            test_imgae_size_X = 9344;
            test_imgae_size_Y = 7000;
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            test_imgae_size_X = 8192;
            test_imgae_size_Y = 2000;
        }
        break;
        default:
            break;
    }

    for (int n_Channel = 0; n_Channel < use_illumation_Count; n_Channel++)
    {
        float fSimulationValue = verify_ref_illumination_gv[n_Channel];

        if (fSimulationValue <= 0.f)
            fSimulationValue = 1.f;

        CString msgProcess;
        msgProcess.Format(_T("Simulation Mode: Channel-%d"), n_Channel + 1);
        procStatus.SetWindowText(msgProcess);

        if (m_vision_type == VISIONTYPE_NGRV_INSP)
        {
            Ipvm::Image8u3 simulationImage_Color;
            simulationImage_Color.Create(test_imgae_size_X, test_imgae_size_Y);
            simulationImage_Color.FillZero();

            auto& image = simulationImage_Color;
            for (long y = 0; y < image.GetSizeY(); y++)
            {
                auto* image_y = image.GetMem(0, y);
                for (long x = 0; x < image.GetSizeX(); x++)
                {
                    image_y[x].m_x = BYTE(rand() % long(fSimulationValue * 2));
                    image_y[x].m_y = BYTE(rand() % long(fSimulationValue * 2));
                    image_y[x].m_z = BYTE(rand() % long(fSimulationValue * 2));
                }
            }

            m_imageView->SetImage(simulationImage_Color);
            verify_grabed_color_images.push_back(simulationImage_Color);
        }
        else
        {
            Ipvm::Image8u simulationImage;
            simulationImage.Create(test_imgae_size_X, test_imgae_size_Y);
            simulationImage.FillZero();

            auto& image = simulationImage;
            for (long y = 0; y < image.GetSizeY(); y++)
            {
                auto* image_y = image.GetMem(0, y);
                for (long x = 0; x < image.GetSizeX(); x++)
                {
                    image_y[x] = BYTE(rand() % long(fSimulationValue * 2));
                }
            }

            m_imageView->SetImage(simulationImage);
            verify_grabed_images.push_back(simulationImage);
        }
    }

    if (m_vision_type == VISIONTYPE_NGRV_INSP)
    {
        CalculateVerify_color(m_vision_type, verify_grabed_color_images);
    }
    else
    {
        CalculateVerify_mono(m_vision_type, verify_grabed_images);
    }

    SetGridCtrl_Result(m_vision_type, view_color_image_RGB_idx);
    SaveReport_Verify(m_vision_type);

    //((CCustomItemButton*)grid_Verify->FindItem(ITEM_ID_SCAN_VERIFY_FOR_GV))->SetReadOnly(FALSE);
    //((CCustomItemButton*)grid_Verify->FindItem(ITEM_ID_SCAN_VERIFY_FOR_MS))->SetReadOnly(FALSE);
    grid_Verify->Refresh();
}

void CDlgSub2DVisionTune::TestSimulation_Live()
{
    long test_imgae_size_X(0);
    long test_imgae_size_Y(0);

    switch (m_vision_type)
    {
        case VISIONTYPE_2D_INSP:
        {
            test_imgae_size_X = 5120;
            test_imgae_size_Y = 5120;
        }
        break;
        case VISIONTYPE_NGRV_INSP:
        {
            test_imgae_size_X = 9344;
            test_imgae_size_Y = 7000;
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            test_imgae_size_X = 8192;
            test_imgae_size_Y = 2000;
        }
        break;
        default:
            break;
    }

    float fSimulationValue(1.f);

    CString msgProcess(_T("Simulation Mode: Live Mode"));
    procStatus.SetWindowText(msgProcess);

    if (m_vision_type == VISIONTYPE_NGRV_INSP)
    {
        Ipvm::Image8u3 simulationImage_Color;
        simulationImage_Color.Create(test_imgae_size_X, test_imgae_size_Y);
        simulationImage_Color.FillZero();

        auto& image = simulationImage_Color;
        for (long y = 0; y < image.GetSizeY(); y++)
        {
            auto* image_y = image.GetMem(0, y);
            for (long x = 0; x < image.GetSizeX(); x++)
            {
                image_y[x].m_x = BYTE(rand() % long(fSimulationValue * 2));
                image_y[x].m_y = BYTE(rand() % long(fSimulationValue * 2));
                image_y[x].m_z = BYTE(rand() % long(fSimulationValue * 2));
            }
        }

        color_Image->Create(test_imgae_size_X, test_imgae_size_Y);
        Ipvm::ImageProcessing::Copy(simulationImage_Color, Ipvm::Rect32s(simulationImage_Color), *color_Image);
    }
    else
    {
        Ipvm::Image8u simulationImage;
        simulationImage.Create(test_imgae_size_X, test_imgae_size_Y);
        simulationImage.FillZero();

        auto& image = simulationImage;
        for (long y = 0; y < image.GetSizeY(); y++)
        {
            auto* image_y = image.GetMem(0, y);
            for (long x = 0; x < image.GetSizeX(); x++)
            {
                image_y[x] = BYTE(rand() % long(fSimulationValue * 2));
            }
        }

        mono_Image->Create(test_imgae_size_X, test_imgae_size_Y);
        Ipvm::ImageProcessing::Copy(simulationImage, Ipvm::Rect32s(simulationImage), *mono_Image);
    }
}

void CDlgSub2DVisionTune::view_avg_gv(const long& vision_type)
{
    m_imageView->ClearImageOverlay();

    long diflate_X_px(0);
    long diflate_Y_px(0);

    switch (vision_type)
    {
        case VISIONTYPE_2D_INSP:
        {
            diflate_X_px = DIFLATE_FROM_FULL_IMAGE_X_DEFAULT_PX;
            diflate_Y_px = DIFLATE_FROM_FULL_IMAGE_Y_DEFAULT_PX;
        }
        break;
        case VISIONTYPE_NGRV_INSP:
        {
            diflate_X_px = DIFLATE_FROM_FULL_IMAGE_X_DEFAULT_PX;
            diflate_Y_px = DIFLATE_FROM_FULL_IMAGE_Y_DEFAULT_PX;
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            diflate_X_px = DIFLATE_FROM_FULL_IMAGE_X_SIDE_PX;
            diflate_Y_px = DIFLATE_FROM_FULL_IMAGE_Y_SIDE_PX;
        }
        break;
        default:
            break;
    }

    // deflateValueX, deflateValueY 영상 사이즈를 deflate한다
    long rectDeflateX = diflate_X_px * 2;
    long rectDeflateY = diflate_Y_px * 2;

    Ipvm::Image8u calcImage;
    if (vision_type == VISIONTYPE_NGRV_INSP)
    {
        if (!GetSplitGrayImage(*color_Image, color_split_idx(view_color_image_RGB_idx), calcImage))
            return;
    }
    else
    {
        calcImage = *mono_Image;
    }

    const long image_size_X_px = calcImage.GetSizeX();
    const long image_size_Y_px = calcImage.GetSizeY();

    Ipvm::Rect32s calc_area((int32_t)rectDeflateX, (int32_t)rectDeflateY, (int32_t)(image_size_X_px - rectDeflateX),
        (int32_t)(image_size_Y_px - rectDeflateY));

    CString text("");
    double meanValue(0.);
    Ipvm::ImageProcessing::GetMean(calcImage, calc_area, meanValue);
    text.Format(_T("%.2f"), meanValue);
    m_imageView->AddImageOverlay(calc_area, RGB(0, 255, 0));
    m_imageView->AddImageOverlay(calc_area.CenterPoint(), text, RGB(255, 0, 0), 300);

    m_imageView->ShowImageOverlay();

    text.Empty();
}