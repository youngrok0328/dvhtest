//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionPrimaryUI.h"

//CPP_2_________________________________ This project's headers
#include "DlgMarkTeachTotalResultViewer.h"
#include "DlgReviewImageViewer.h"
#include "DlgSystemSetup.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../HardwareModules/dPI_Framegrabber/FrameGrabber.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h" //k
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/iPIS_MsgDefinitions.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../UserInterfaceModules/VisionCommonDialogTR/VisionCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialogTR/VisionCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialogTR/VisionCommonLogDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialogTR/VisionCommonResultDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialogTR/VisionCommonSpecDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialogTR/VisionCommonTextResultDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionMainAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_SPEC 100

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "version.lib")

enum
{
    UM_GRAB_END_EVENT = (WM_USER + 1043),
    UM_SHOW_DETAIL_RESULT,
    UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED,
};

enum enumSetupInfoTab
{
    TAB_INSPECTION_SPEC = 0,
    TAB_INSPECTION_RESULT,
    TAB_DETAIL_RESULT,
    TAB_DEBUG_INFO,
    TAB_TXT_RESULT,
    TAB_TXT_LOG,
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ORIGINAL_NGRV_IMAGE_SIZE_X_SPLIT_COLOR 9328;
#define ORIGINAL_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR 6992;
#define RESIZE_NGRV_IMAGE_SIZE_X_SPLIT_COLOR 2332;
#define RESIZE_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR 1748;

IMPLEMENT_DYNAMIC(CDlgVisionPrimaryUI, CDialog)

CDlgVisionPrimaryUI::CDlgVisionPrimaryUI(
    VisionMainAgent& visionMainAgent, VisionUnit& visionUnit, CWnd* pParent /*=NULL*/)
    : CDialog(IDD_DIALOG_PRIMARY, pParent)
    , m_visionMainAgent(visionMainAgent)
    , m_visionUnit(visionUnit)
    , m_imageLotView(nullptr)
    , m_imageLotViewRearSide(nullptr) // Rear Side UI Divide 2024.03.29
    , m_visionCommonSpecDlg(nullptr)
    , m_visionCommonResultDlg(nullptr)
    , m_visionCommonDetailResultDlg(nullptr)
    , m_visionCommonDebugInfoDlg(nullptr)
    , m_visionCommonTextResultDlg(nullptr)
    , m_visionCommonLogDlg(nullptr)
    , m_visionCommonResultDlgRear(nullptr)
    , m_visionCommonDetailResultDlgRear(nullptr)
    , m_visionCommonDebugInfoDlgRear(nullptr)
    , m_visionCommonTextResultDlgRear(nullptr)
    , m_pDlgMarkTeachTotalResultView(NULL) //kircheis_201611XX //TeachResultView
    , m_bJobTeachEnable(true)
    , m_nSideVisionSection(0)
    , m_nCurCommonTab(TAB_INSPECTION_SPEC)
{
    m_pReviewImageViewer = new CDlgReviewImageViewer(this);
    m_bMarkSkipMode = FALSE;
}

CDlgVisionPrimaryUI::~CDlgVisionPrimaryUI()
{
    if (m_visionCommonTextResultDlgRear != nullptr)
        delete m_visionCommonTextResultDlgRear;
    if (m_visionCommonDebugInfoDlgRear != nullptr)
        delete m_visionCommonDebugInfoDlgRear;
    if (m_visionCommonDetailResultDlgRear != nullptr)
        delete m_visionCommonDetailResultDlgRear;
    if (m_visionCommonResultDlgRear != nullptr)
        delete m_visionCommonResultDlgRear;

    delete m_visionCommonLogDlg;
    delete m_visionCommonTextResultDlg;
    delete m_visionCommonDebugInfoDlg;
    delete m_visionCommonDetailResultDlg;
    delete m_visionCommonResultDlg;
    delete m_visionCommonSpecDlg;
    delete m_imageLotView;
    delete m_imageLotViewRearSide; // Rear Side UI Divide 2024.03.29

    if (m_pDlgMarkTeachTotalResultView != NULL) //kircheis_201611XX //TeachResultView
    {
        m_pDlgMarkTeachTotalResultView->DestroyWindow();
        delete m_pDlgMarkTeachTotalResultView;
        m_pDlgMarkTeachTotalResultView = NULL;
    }

    delete m_pReviewImageViewer;
}

void CDlgVisionPrimaryUI::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_tabResult);
    DDX_Control(pDX, IDC_STATIC_JOB_FILE, m_Label_JobFile);
    DDX_Control(pDX, IDC_STATIC_JOB_FILE_NAME, m_Label_JobFileName);
    DDX_Control(pDX, IDC_STATIC_CONNECT1, m_Label_Connect1);
    DDX_Control(pDX, IDC_STATIC_CONNECT2, m_Label_Connect2);
    DDX_Control(pDX, IDC_STATIC_RESULT, m_LabelResult);
    DDX_Control(pDX, IDC_STATIC_SIDE_FRONT_PRIMARY, m_Label_Side_Front_Status);
    DDX_Control(pDX, IDC_STATIC_SIDE_REAR_PRIMARY, m_Label_Side_Rear_Status);
}

BEGIN_MESSAGE_MAP(CDlgVisionPrimaryUI, CDialog)
ON_WM_CREATE()
ON_WM_DESTROY()
ON_WM_TIMER()
ON_WM_SIZE()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionPrimaryUI::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_ADD_LOG_WARNING, &CDlgVisionPrimaryUI::OnAddLogWarning)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionPrimaryUI::OnShowDetailResult)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionPrimaryUI::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_GRAB_END_EVENT, &CDlgVisionPrimaryUI::OnGrabEndEvent)
ON_STN_CLICKED(IDC_STATIC_SIDE_FRONT_PRIMARY, &CDlgVisionPrimaryUI::OnStnClickedStaticSideFrontPrimary)
ON_STN_CLICKED(IDC_STATIC_SIDE_REAR_PRIMARY, &CDlgVisionPrimaryUI::OnStnClickedStaticSideRearPrimary)
END_MESSAGE_MAP()

// CDlgVisionPrimaryUI 메시지 처리기입니다.
BOOL CDlgVisionPrimaryUI::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 일반 비전과 Side 비전일떄의 UI를 설정해줌 - JHB_2024.04.15
    SetGeneralVisionMainUI();

    m_pDlgMarkTeachTotalResultView = new CDlgMarkTeachTotalResultViewer(this);
    m_pDlgMarkTeachTotalResultView->Create(CDlgMarkTeachTotalResultViewer::IDD);

    checkInstallerMonitorProgram();
    // 5분에 한번씩 Installer Monitor가 동작중인지 확인하자
    SetTimerFlag(Timer_Check_Installer_Monitor, 300000);

    // 1분에 한번씩 iGrab Board 온도를 확인하자
    //F/W와 Lib Version이 일치하고 Hardware 상태일 경우에만 온도 Check기능을 활성화 한다
    if (SystemConfig::GetInstance().Get_is_iGrabFirmware_and_LibraryVersion_Mismatch() == false
        && SystemConfig::GetInstance().IsHardwareExist() == TRUE) //Hardware 사용시 조건문 추가
        SetTimerFlag(Timer_iGrab_Board_Temperature_Check, 60000);

    UpdateLayout();
    ShowWindow(SW_SHOW);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionPrimaryUI::SetGeneralVisionMainUI()
{
    m_tabResult.InsertItem(0, _T("Spec."));
    m_tabResult.InsertItem(1, _T("Result"));
    m_tabResult.InsertItem(2, _T("Detail"));
    m_tabResult.InsertItem(3, _T("Debug"));
    m_tabResult.InsertItem(4, _T("ResultText"));
    m_tabResult.InsertItem(5, _T("Log"));

    // Window 크기와 상관없이 생성한다. 추후 UpdateLayout()에서 크기를 조정한다.
    CRect initRect(0, 0, 1, 1);
    m_imageLotView = new ImageLotView(initRect, m_visionUnit, true, GetSafeHwnd());
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    m_visionCommonSpecDlg = new VisionCommonSpecDlg(this, initRect, m_visionUnit);
    m_visionCommonResultDlg
        = new VisionCommonResultDlg(this, initRect, m_visionUnit, *m_imageLotView, UM_SHOW_DETAIL_RESULT);
    m_visionCommonDetailResultDlg = new VisionCommonDetailResultDlg(this, initRect, m_visionUnit, *m_imageLotView);
    m_visionCommonTextResultDlg = new VisionCommonTextResultDlg(this, initRect);
    m_visionCommonDebugInfoDlg
        = new VisionCommonDebugInfoDlg(this, initRect, m_visionUnit, *m_imageLotView, *m_visionCommonTextResultDlg);
    m_visionCommonLogDlg = new VisionCommonLogDlg(this, initRect);
    m_visionCommonSpecDlg->ShowWindow(SW_SHOW);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    m_Label_JobFile.SetFontSize(11);
    m_Label_JobFile.SetFontName(_T("Arial"));
    m_Label_JobFile.SetFontBold(TRUE);
    m_Label_JobFile.SetTextColor(RGB(0, 0, 0));
    m_Label_JobFile.SetBkColor(RGB(200, 200, 250));
    m_Label_JobFile.SetText(_T("Job Name"));
    m_Label_JobFile.SetAlignment(CNewLabel::HORZ_CENTER | CNewLabel::VERT_CENTER);

    m_Label_JobFileName.SetFontSize(11);
    m_Label_JobFileName.SetFontName(_T("Arial"));
    m_Label_JobFileName.SetFontBold(TRUE);
    m_Label_JobFileName.SetTextColor(RGB(0, 0, 0));
    m_Label_JobFileName.SetBkColor(RGB(255, 255, 255));
    m_Label_JobFileName.SetAlignment(CNewLabel::HORZ_CENTER | CNewLabel::VERT_CENTER);
    m_Label_JobFileName.SetText(_T("Not Selected"));

    m_Label_Connect1.SetFontSize(11);
    m_Label_Connect1.SetFontName(_T("Arial"));
    m_Label_Connect1.SetFontBold(TRUE);
    m_Label_Connect1.SetTextColor(RGB(0, 0, 0));
    m_Label_Connect1.SetBkColor(RGB(200, 200, 250));
    m_Label_Connect1.SetAlignment(CNewLabel::HORZ_CENTER | CNewLabel::VERT_CENTER);
    m_Label_Connect1.SetText(_T("Connect Status"));

    m_Label_Connect2.SetFontSize(11);
    m_Label_Connect2.SetFontName(_T("Arial"));
    m_Label_Connect2.SetFontBold(TRUE);
    m_Label_Connect2.SetTextColor(RGB(0, 0, 0));
    m_Label_Connect2.SetBkColor(RGB(255, 255, 255));
    m_Label_Connect2.SetAlignment(CNewLabel::HORZ_CENTER | CNewLabel::VERT_CENTER);
    m_Label_Connect2.SetText(_T("Disconnected"));

    m_LabelResult.SetFontSize(11);
    m_LabelResult.SetFontName(_T("Arial"));
    m_LabelResult.SetFontBold(TRUE);
    m_LabelResult.SetTextColor(RGB(0, 0, 0));
    m_LabelResult.SetBkColor(RGB(255, 255, 255));
    m_LabelResult.SetAlignment(CNewLabel::HORZ_CENTER | CNewLabel::VERT_CENTER);
    m_LabelResult.SetText(_T("/"));
    ////////////////////////////////////////////////////////////////////////////////////////////////////
}

int CDlgVisionPrimaryUI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_pReviewImageViewer->Create(IDD_DIALOG_REVIEWIMAGE_VIEWER, this);

    return 0;
}

void CDlgVisionPrimaryUI::OnDestroy()
{
    CDialog::OnDestroy();

    static const bool bIsSideVision = (m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision)
    {
        if (m_visionCommonTextResultDlgRear != nullptr)
            m_visionCommonTextResultDlgRear->DestroyWindow();
        if (m_visionCommonDebugInfoDlgRear != nullptr)
            m_visionCommonDebugInfoDlgRear->DestroyWindow();
        if (m_visionCommonResultDlgRear != nullptr)
            m_visionCommonResultDlgRear->DestroyWindow();
        if (m_visionCommonDetailResultDlgRear != nullptr)
            m_visionCommonDetailResultDlgRear->DestroyWindow();
    }

    m_visionCommonDebugInfoDlg->DestroyWindow();
    m_visionCommonLogDlg->DestroyWindow();
    m_visionCommonTextResultDlg->DestroyWindow();
    m_visionCommonResultDlg->DestroyWindow();
    m_visionCommonDetailResultDlg->DestroyWindow();
    m_visionCommonSpecDlg->DestroyWindow();
    m_pReviewImageViewer->DestroyWindow();
}

BOOL CDlgVisionPrimaryUI::ImageFileOpen()
{
    static const bool bIsSideVision = (m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (m_visionMainAgent.OpenImageFileAgent())
    {
        if (bIsSideVision)
        {
            m_imageLotViewRearSide->SetInspectionAreaInfo(m_visionMainAgent.getInspectionAreaInfo());
            m_imageLotViewRearSide->ShowCurrentImage(true);
            m_imageLotViewRearSide->Overlay_RemoveAll();
            m_imageLotViewRearSide->Overlay_Show(TRUE);
        }

        m_imageLotView->SetInspectionAreaInfo(m_visionMainAgent.getInspectionAreaInfo());
        m_imageLotView->ShowCurrentImage(true);
        m_imageLotView->Overlay_RemoveAll();
        m_imageLotView->Overlay_Show(TRUE);

        return TRUE;
    }

    return FALSE;
}

void CDlgVisionPrimaryUI::OnJobChanged()
{
    static const bool bIsSideVision = (m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);

    m_visionCommonResultDlg->ClearVisionInspectionResult();
    m_visionCommonDetailResultDlg->ClearVisionInspectionResult();

    m_visionCommonSpecDlg->Refresh();
    m_visionCommonResultDlg->Refresh();
    m_visionCommonDebugInfoDlg->Refresh();

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    m_imageLotView->UpdateState();

    if (bIsSideVision)
    {
        m_visionCommonResultDlgRear->ClearVisionInspectionResult();
        m_visionCommonDetailResultDlgRear->ClearVisionInspectionResult();

        m_visionCommonResultDlgRear->Refresh();
        m_visionCommonDebugInfoDlgRear->Refresh();

        m_imageLotViewRearSide->Overlay_RemoveAll();
        m_imageLotViewRearSide->Overlay_Show(TRUE);

        m_imageLotViewRearSide->UpdateState();
    }

    m_Label_JobFileName.SetText(m_visionUnit.GetJobFileName());

    m_imageLotView->OnJobChanged();
    if (bIsSideVision)
        m_imageLotViewRearSide->OnJobChanged();
}

void CDlgVisionPrimaryUI::OnBnClickedButtonInspect()
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("Clicked inspect button"));

    static const bool bIsSideVision = (SystemConfig::GetInstance().IsVisionTypeSide() == 1) ? true : false;
    if (bIsSideVision)
    {
        OnBnClickedButtonInspectSide();
        return;
    }

    Ipvm::TimeCheck FovTime;

    CString strSum = _T("");
    CString strTemp;
    COLORREF color;

    static const int32_t nOverlayNum = bIsSideVision ? 2 : 1;
    std::vector<VisionInspectionOverlayResult> vecOverlayResult(nOverlayNum);

    std::vector<ImageLotView*> vecpImageLotView(nOverlayNum);
    vecpImageLotView[0] = m_imageLotView;
    if (bIsSideVision)
        vecpImageLotView[1] = m_imageLotViewRearSide;

    m_visionCommonResultDlg->ClearVisionInspectionResult();
    m_visionCommonDetailResultDlg->ClearVisionInspectionResult();

    // Updata Spec Data
    m_visionCommonTextResultDlg->SetLogBoxText(_T(""));
    if (bIsSideVision)
        m_visionCommonTextResultDlgRear->SetLogBoxText(_T(""));
    // 선택된 Pane 만 검사하자
    // 모든 Pane을 검사해 봐야 Core가 이전 결과를 버려서 로그이외에 제대로 표시해주지도 못한다.
    long nPane = m_visionUnit.GetCurrentPaneID();

    Ipvm::TimeCheck Panetime;

    strTemp.Format(_T("\r\n<< Inspection Pane Num :%d >>\r\n "), nPane);
    strSum += strTemp;

    m_visionUnit.SetCurrentPaneID(nPane);

    //mc_Side Vision
    //검사결과 View 따로 담아야하나..
    //Side Vision이면 Front, Rear 검사, 그렇지 않은 Vision들은 무조건 Front규칙에 따른다

    for (long nRepeatidx = enSideVisionModule::SIDE_VISIONMODULE_START; nRepeatidx < nOverlayNum; nRepeatidx++)
    {
        vecpImageLotView[nRepeatidx]->Overlay_RemoveAll();
        //if (!m_visionUnit.RunInspection(nullptr, false, enSideVisionModule(m_visionMainAgent.GetSideVisionSection()))) //여기는 Main화면에서 Run inspection하면 타는 루프
        if (!m_visionUnit.RunInspection(
                nullptr, false, enSideVisionModule(nRepeatidx))) //여기는 Main화면에서 Run inspection하면 타는 루프
        {
            ;
            ;
        }
        vecOverlayResult[nRepeatidx].AddOverlay(m_visionUnit.GetInspectionOverlayResult());
        vecOverlayResult[nRepeatidx].Apply(vecpImageLotView[nRepeatidx]->GetCoreView());
        vecpImageLotView[nRepeatidx]->ShowCurrentImage();
    }

    float paneTime = CAST_FLOAT(Panetime.Elapsed_ms()); // Time Check 끝...

    strSum += m_visionUnit.GetLastInspection_Text(FALSE);

    m_visionCommonTextResultDlg->SetLogBoxText(strSum);

    // Dialog 가 있으면 화면 갱신
    m_visionCommonDebugInfoDlg->Refresh();

    // 검사결과(Result Dlg Grid) 화면 갱신
    m_visionCommonResultDlg->Refresh();

    long nResult = m_visionUnit.GetInspTotalResult();

    switch (nResult)
    {
        case PASS:
            color = RGB(0, 255, 0);
            strTemp.Format(_T("%d Pane [Pass]"), nPane);
            break;
        case REJECT:
            color = RGB(255, 0, 0);
            strTemp.Format(_T("%d Pane [Reject]"), nPane);
            break;
        case INVALID:
            nResult = INVALID;
            color = RGB(255, 255, 0);
            strTemp.Format(_T("%d Pane [Invalid]"), nPane);
            break;
        case DOUBLEDEVICE: //kircheis_3DEmpty
            color = RGB(150, 255, 255);
            strTemp.Format(_T("%d Pane [Double]"), nPane);
            break;
        case EMPTY:
            color = RGB(255, 255, 255);
            strTemp.Format(_T("%d Pane [Empty]"), nPane);
            break;
        case COUPON:
            color = RGB(255, 128, 0);
            strTemp.Format(_T("%d Pane [Coupon]"), nPane);
            break;
        default:
            color = RGB(180, 180, 180);
            strTemp.Format(_T("%d Pane [Not Measured]"), nPane);
            break;
    }

    CString strTime;
    strTime.Format(_T("%.2f ms"), paneTime);
    m_LabelResult.SetTextColor(Result2Color(nResult));
    m_LabelResult.SetText(Result2String(nResult) + _T(" / ") + strTime);

    // 검사 결과 탭 보여주기
    m_tabResult.SetCurSel(TAB_INSPECTION_RESULT);
    OnTcnSelchangeTabResult(nullptr, nullptr);
}

void CDlgVisionPrimaryUI::OnBnClickedButtonInspectSide()
{
    Ipvm::TimeCheck FovTime;

    CString strSum = _T("");
    CString strTemp;
    COLORREF color;

    static const bool bIsSideVision = (SystemConfig::GetInstance().IsVisionTypeSide() == 1) ? true : false;
    static const int32_t nOverlayNum = bIsSideVision ? 2 : 1;
    std::vector<VisionInspectionOverlayResult> vecOverlayResult(nOverlayNum);

    std::vector<ImageLotView*> vecpImageLotView(nOverlayNum);
    vecpImageLotView[0] = m_imageLotView;
    if (bIsSideVision)
        vecpImageLotView[1] = m_imageLotViewRearSide;

    m_visionCommonResultDlg->ClearVisionInspectionResult();
    m_visionCommonDetailResultDlg->ClearVisionInspectionResult();

    // Updata Spec Data
    m_visionCommonTextResultDlg->SetLogBoxText(_T(""));
    if (bIsSideVision)
    {
        m_visionCommonTextResultDlgRear->SetLogBoxText(_T(""));
        m_visionCommonResultDlgRear->ClearVisionInspectionResult();
        m_visionCommonDetailResultDlgRear->ClearVisionInspectionResult();
    }
    // 선택된 Pane 만 검사하자
    // 모든 Pane을 검사해 봐야 Core가 이전 결과를 버려서 로그이외에 제대로 표시해주지도 못한다.
    long nPane = m_visionUnit.GetCurrentPaneID();

    Ipvm::TimeCheck Panetime;

    strTemp.Format(_T("\r\n<< Inspection Pane Num :%d >>\r\n "), nPane);

    m_visionUnit.SetCurrentPaneID(nPane);

    //mc_Side Vision
    //검사결과 View 따로 담아야하나..
    //Side Vision이면 Front, Rear 검사, 그렇지 않은 Vision들은 무조건 Front규칙에 따른다
    std::vector<long> vecnResult(nOverlayNum);
    long nWorstResult = NOT_MEASURED;

    CString strAccumulatedInspResult = _T("");

    for (long nRepeatidx = enSideVisionModule::SIDE_VISIONMODULE_START; nRepeatidx < nOverlayNum; nRepeatidx++)
    {
        auto& visionCommonTextResultDlg = (nRepeatidx == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            ? m_visionCommonTextResultDlgRear
            : m_visionCommonTextResultDlg;
        auto& visionCommonDebugInfoDlg = (nRepeatidx == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            ? m_visionCommonDebugInfoDlgRear
            : m_visionCommonDebugInfoDlg;
        auto& visionCommonResultDlg = (nRepeatidx == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            ? m_visionCommonResultDlgRear
            : m_visionCommonResultDlg;
        auto& visionCommonDetailResultDlg = (nRepeatidx == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            ? m_visionCommonDetailResultDlgRear
            : m_visionCommonDetailResultDlg;

        strSum = strTemp;
        vecpImageLotView[nRepeatidx]->Overlay_RemoveAll();
        //if (!m_visionUnit.RunInspection(nullptr, false, enSideVisionModule(m_visionMainAgent.GetSideVisionSection()))) //여기는 Main화면에서 Run inspection하면 타는 루프
        if (!m_visionUnit.RunInspection(
                nullptr, false, enSideVisionModule(nRepeatidx))) //여기는 Main화면에서 Run inspection하면 타는 루프
        {
            ;
            ;
        }
        vecOverlayResult[nRepeatidx].AddOverlay(m_visionUnit.GetInspectionOverlayResult());
        vecOverlayResult[nRepeatidx].Apply(vecpImageLotView[nRepeatidx]->GetCoreView());
        vecpImageLotView[nRepeatidx]->ShowCurrentImage();

        strSum += m_visionUnit.GetLastInspection_Text(FALSE);

        visionCommonTextResultDlg->SetLogBoxText(strSum);
        // Dialog 가 있으면 화면 갱신
        visionCommonDebugInfoDlg->Refresh();
        // 검사결과(Result Dlg Grid) 화면 갱신
        visionCommonResultDlg->CollectVisionInspectionResult();
        visionCommonResultDlg->Refresh();
        visionCommonDetailResultDlg->CollectVisionInspectionResult();

        vecnResult[nRepeatidx] = m_visionUnit.GetInspTotalResult();

        nWorstResult = (long)max(nWorstResult, vecnResult[nRepeatidx]);

        strAccumulatedInspResult += Result2String(vecnResult[nRepeatidx]);
        if (nRepeatidx < nOverlayNum - 1)
        {
            strAccumulatedInspResult += _T(", ");
        }
    }

    float paneTime = CAST_FLOAT(Panetime.Elapsed_ms()); // Time Check 끝...

    switch (nWorstResult)
    {
        case PASS:
            color = RGB(0, 255, 0);
            strTemp.Format(_T("%d Pane [Pass]"), nPane);
            break;
        case REJECT:
            color = RGB(255, 0, 0);
            strTemp.Format(_T("%d Pane [Reject]"), nPane);
            break;
        case INVALID:
            color = RGB(255, 255, 0);
            strTemp.Format(_T("%d Pane [Invalid]"), nPane);
            break;
        case DOUBLEDEVICE: //kircheis_3DEmpty
            color = RGB(150, 255, 255);
            strTemp.Format(_T("%d Pane [Double]"), nPane);
            break;
        case EMPTY:
            color = RGB(255, 255, 255);
            strTemp.Format(_T("%d Pane [Empty]"), nPane);
            break;
        case COUPON:
            color = RGB(255, 128, 0);
            strTemp.Format(_T("%d Pane [Coupon]"), nPane);
            break;
        default:
            color = RGB(180, 180, 180);
            strTemp.Format(_T("%d Pane [Not Measured]"), nPane);
            break;
    }

    CString strTime;
    strTime.Format(_T("%.2f ms"), paneTime);
    m_LabelResult.SetTextColor(Result2Color(nWorstResult));
    m_LabelResult.SetText(strAccumulatedInspResult + _T(" / ") + strTime);

    // 검사 결과 탭 보여주기
    m_tabResult.SetCurSel(TAB_INSPECTION_RESULT);
    OnTcnSelchangeTabResult(nullptr, nullptr);
}

void CDlgVisionPrimaryUI::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent)
    {
        case Timer_Send_Host_Live_Image:
        {
            auto& imageLot = m_visionUnit.getImageLot();
            const int& idx_grabber = 0;
            const int& idx_camera = 0;

            if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
            {
                Ipvm::Image8u image;

                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());

                BYTE* imageMem = image.GetMem();

                auto sendHostFrameNumber = SystemConfig::GetInstance().m_nSendHostFrameNumber;
                const long nSendFrameID
                    = (sendHostFrameNumber < imageLot.GetImageFrameCount()) ? sendHostFrameNumber : 0;

                SyncController::GetInstance().TurnOnLight(nSendFrameID, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &imageMem, 1, nullptr);
                SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr);

                long nOriginalImageSizeX = ORIGINAL_NGRV_IMAGE_SIZE_X_SPLIT_COLOR;
                long nOriginalImageSizeY = ORIGINAL_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR;
                long nResizeImageSizeX = RESIZE_NGRV_IMAGE_SIZE_X_SPLIT_COLOR;
                long nResizeImageSizeY = RESIZE_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR;

                std::vector<BYTE> vecbyResizeImage(nResizeImageSizeX * nResizeImageSizeY);

                long nDstX(0), nDstY(0);
                long nIdxY = 0;
                for (long nY = 0; nY < nOriginalImageSizeY; nY += 4)
                {
                    nIdxY = nDstY * nResizeImageSizeX;
                    nDstY++;
                    nDstX = 0;
                    for (long nX = 0; nX < nOriginalImageSizeX; nX += 4)
                    {
                        vecbyResizeImage[nIdxY + nDstX] = image.GetMem(nX, nY)[0];
                        nDstX++;
                    }
                }

                Ipvm::Image8u imageResizeMono(
                    nResizeImageSizeX, nResizeImageSizeY, &vecbyResizeImage[0], nResizeImageSizeX);

                long length;
                DEFMSGPARAM Data;
                Data.P1 = nResizeImageSizeX;
                Data.P2 = nResizeImageSizeY;

                length = sizeof(Data);

                m_visionMainAgent.m_pMessageSocket->Write(MSG_PROBE_IMAGE_SIZE, length, (BYTE*)&Data);

                m_visionMainAgent.m_pMessageSocket->Write(MSG_PROBE_IMAGE_SEND_1,
                    imageResizeMono.GetWidthBytes() * imageResizeMono.GetSizeY(), imageResizeMono.GetMem());

                SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            }
            else if (imageLot.GetImageFrameCount() > 0
                && SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_NGRV_INSP)
            {
                Ipvm::Image8u image(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());

                auto sendHostFrameNumber = SystemConfig::GetInstance().m_nSendHostFrameNumber;
                const long nSendFrameID
                    = (sendHostFrameNumber < imageLot.GetImageFrameCount()) ? sendHostFrameNumber : 0;

                SyncController::GetInstance().TurnOnLight(nSendFrameID, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                BYTE* imageMem = image.GetMem();

                if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &imageMem, 1, nullptr))
                {
                    if (FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
                    {
                        m_visionMainAgent.m_pMessageSocket->Write(
                            MSG_PROBE_IMAGE_SEND_2, image.GetWidthBytes() * image.GetSizeY(), image.GetMem());
                    }
                }

                SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            }
            else
            {
                SetTimerFlag(Timer_Send_Host_Live_Image, 0);
            }
        }
        break;

        case Timer_Check_Installer_Monitor:
            checkInstallerMonitorProgram();
            break;
            //case Timer_AutoFocusing_2DManual_Package:
            //	{
            //		static BOOL bDoingCalc = FALSE;
            //		if(!bDoingCalc)
            //		{
            //			bDoingCalc = TRUE;

            //			//사용할 Image 및 CalcFocusValue에 필요한 변수 선언.
            //			long nPackageType = m_pDlgAutoFocusing->m_nPackageType;
            //			long nFrameID = m_pDlgAutoFocusing->m_nFrameIndex;
            //			long nSrcImgWidth = (*m_pVisionUnit->m_sImageLot.m_pvecImages)[nFrameID].nWidth;
            //			long nSrcImgHeight = (*m_pVisionUnit->m_sImageLot.m_pvecImages)[nFrameID].nHeight;
            //			long nSrcImgSize = nSrcImgWidth * nSrcImgHeight;

            //			//조명 Set
            //			m_pDlgAutoFocusing->Manual2D_AutoFocusing_Package_illuminationSet();

            //			//영상 Get
            //			Get2DMainImage();

            //			//EdgeAlign까지 검사 Check
            //			BOOL bInspItemResult = FALSE;
            //			bInspItemResult = m_pDlgAutoFocusing->InspectionItemResultCheck();
            //			if(!bInspItemResult) //EdgeAlign까지 실패하였다면
            //			{
            //				m_pDlgAutoFocusing->m_b2DManual_AutoFocusing_State = FALSE;
            //				m_pDlgAutoFocusing->Manual_2DAutoFocusing_Initialize();
            //				::SimpleMessage("Check Inspection Item");
            //				bDoingCalc = FALSE;
            //				return;
            //			}

            //			//AlignResult
            //			std::vector<SAlignResult> vecsAlignResult;
            //			vecsAlignResult = m_pDlgAutoFocusing->m_vecsEdgeAlignReuslt;

            //			//영상 설정
            //			BYTE *pbyCalcImg = (*m_pVisionUnit->m_sImageLot.m_pvecImages)[nFrameID].pbyImage;

            //			m_pDlgAutoFocusing->Manual2D_AutoFocusing_Package(nPackageType, pbyCalcImg, nSrcImgWidth, nSrcImgHeight, nSrcImgSize, vecsAlignResult);

            //			//사용하고 있는 Frame Image DisPlay
            //			m_imageView->SetImage(pbyCalcImg, nSrcImgWidth, nSrcImgHeight, nSrcImgWidth);

            //			bDoingCalc = FALSE;

            //		}
            //	}
            //	break;
        case Timer_iGrab_Board_Temperature_Check: //mc_iGrab Board 온도 확인
            check_iGrab_Board_Temperature_Check();
            break;
    }

    CDialog::OnTimer(nIDEvent);
}

void CDlgVisionPrimaryUI::DrawCenterCrossLine(BOOL bIDOverlayCenterLine)
{
    if (!bIDOverlayCenterLine)
        return;

    Ipvm::Point32r2 pt1, pt2, pt3, pt4;

    const auto& pixelToUm = m_visionUnit.getScale().pixelToUm();
    pt1 = Ipvm::Point32r2(pixelToUm.m_x / 2, 0);
    pt2 = Ipvm::Point32r2(pixelToUm.m_x / 2, pixelToUm.m_y);
    pt3 = Ipvm::Point32r2(0, pixelToUm.m_y / 2);
    pt4 = Ipvm::Point32r2(pixelToUm.m_x, pixelToUm.m_y / 2);

    Ipvm::LineSeg32r line1 = Ipvm::LineSeg32r(pt1.m_x, pt1.m_y, pt2.m_x, pt2.m_y);
    Ipvm::LineSeg32r line2 = Ipvm::LineSeg32r(pt3.m_x, pt3.m_y, pt4.m_x, pt4.m_y);

    m_imageView->ImageOverlayAdd(line1, RGB(0, 255, 0));
    m_imageView->ImageOverlayAdd(line2, RGB(0, 255, 0));
    m_imageView->ImageOverlayShow();
}

void CDlgVisionPrimaryUI::OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);

    m_nCurCommonTab = m_tabResult.GetCurSel();

    UpdateCommonTabShow();

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT CDlgVisionPrimaryUI::OnAddLogWarning(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(lparam);
    UNREFERENCED_PARAMETER(wparam);

    // 경고시 Log탭으로 강제 이동
    m_tabResult.SetCurSel(5);
    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0;
}

LRESULT CDlgVisionPrimaryUI::OnShowDetailResult(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(lparam);

    const long itemIndex = long(wparam);

    if (itemIndex < TAB_INSPECTION_SPEC)
    {
        m_tabResult.SetCurSel(TAB_INSPECTION_RESULT);
        m_nCurCommonTab = TAB_INSPECTION_RESULT;
    }
    else
    {
        const long nSideSection = m_visionMainAgent.GetSideVisionSection();
        auto& visionCommonDetailResultDlg = (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            ? m_visionCommonDetailResultDlgRear
            : m_visionCommonDetailResultDlg;
        if (visionCommonDetailResultDlg->Refresh(itemIndex))
        {
            m_tabResult.SetCurSel(2);
            m_nCurCommonTab = TAB_DETAIL_RESULT;
            OnTcnSelchangeTabResult(nullptr, nullptr);
        }
    }

    return 0L;
}

LRESULT CDlgVisionPrimaryUI::OnImageLotViewPaneSelChanged(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(lparam);
    UNREFERENCED_PARAMETER(wparam);

    return 0L;
}

void CDlgVisionPrimaryUI::ShowCurrentImage(bool showRaw)
{
    m_imageLotView->ShowCurrentImage(showRaw);

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
        m_imageLotViewRearSide->ShowCurrentImage(showRaw);
}

void CDlgVisionPrimaryUI::SetLiveMain()
{
    SyncController::GetInstance().SetFramePeriod(
        500.f, enSideVisionModule(m_visionMainAgent.GetSideVisionSection())); //frame period
    SyncController::GetInstance().TurnOnLight(
        m_imageLotView->GetCurrentImageFrame(), enSideVisionModule(m_visionMainAgent.GetSideVisionSection()));
    FrameGrabber::GetInstance().live_on(
        GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enSideVisionModule(m_visionMainAgent.GetSideVisionSection()));
}

void CDlgVisionPrimaryUI::ResetLiveMain()
{
    FrameGrabber::GetInstance().live_off();
    SyncController::GetInstance().TurnOffLight(enSideVisionModule(m_visionMainAgent.GetSideVisionSection()));
    SyncController::GetInstance().SetFramePeriod(
        (float)max(SystemConfig::GetInstance().m_nLastDownloadDuration, SystemConfig::GetInstance().GetGrabDuration()),
        enSideVisionModule(m_visionMainAgent.GetSideVisionSection())); //kircheis_CamTrans
}

void CDlgVisionPrimaryUI::ReviewImageViewer()
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("Show Review Image Viwer"));
    m_pReviewImageViewer->ShowWindow(SW_SHOW);
}

void CDlgVisionPrimaryUI::SystemSetup()
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("Show System Setup"));
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DlgSystemSetup Dlg;
    long nOriginilluminationCalType = ::SystemConfig::GetInstance().m_nIlluminationCalType; //kircheis_IllumCalType
    float fOriginalInterpolationStart = ::SystemConfig::GetInstance().m_fInterpolationStart; //SDY_DualCal
    if (Dlg.DoModal() == IDOK)
    {
        m_visionUnit.m_systemConfig.SaveIni(DynamicSystemPath::get(DefineFile::System));
        if (Dlg.m_needReboot)
        {
            ::exit(0);
        }
        if (nOriginilluminationCalType != ::SystemConfig::GetInstance().m_nIlluminationCalType
            || fOriginalInterpolationStart
                != ::SystemConfig::GetInstance().m_fInterpolationStart) //kircheis_IllumCalType
            m_visionMainAgent.HardwareSetup();
    }
}

// 영훈 : 다른 부분에서 진입할 수 있도록 만든다. ( 지금은 SK에서 진입하려고.. )
void CDlgVisionPrimaryUI::SetTimerFlag(long nFlag, long nTime)
{
    if (nTime > 0)
    {
        SetTimer(nFlag, nTime, NULL);
    }
    else
    {
        KillTimer(nFlag);
    }
}

void CDlgVisionPrimaryUI::SetMarkTeachTotalResultView(const Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtViewArea,
    long i_nMarkCharNum, Ipvm::Rect32s* i_prtEachCharROI) //kircheis_201611XX //TeachResultView
{
    if (m_pDlgMarkTeachTotalResultView == NULL)
        return;

    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("Show Mark Teach Total Result View"));

    CRect rtDlg;
    m_pDlgMarkTeachTotalResultView->GetWindowRect(rtDlg);
    static const long nDlgWidth = rtDlg.Width();
    static const long nDlgHeight = rtDlg.Height();
    m_pDlgMarkTeachTotalResultView->MoveWindow(100, 200, nDlgWidth, nDlgHeight);
    m_pDlgMarkTeachTotalResultView->SetImageData(image, i_rtViewArea, i_nMarkCharNum, i_prtEachCharROI);
    m_pDlgMarkTeachTotalResultView->ShowWindow(SW_SHOW);
    ;
}

void CDlgVisionPrimaryUI::HideMarkTeachTotalResultView() //kircheis_201611XX //TeachResultView
{
    if (m_pDlgMarkTeachTotalResultView == NULL)
        return;

    m_pDlgMarkTeachTotalResultView->ShowWindow(SW_HIDE);
}

// SDY Profile view를 닫기 위한 함수
void CDlgVisionPrimaryUI::CloseProfileView()
{
    m_imageView->CloseProfileView();
}

void CDlgVisionPrimaryUI::UpdateLayout()
{
    UpdateLayoutGeneralVisionMainUI();
}

void CDlgVisionPrimaryUI::UpdateLayoutGeneralVisionMainUI()
{
    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(0, 0, rtParentClient.Width(), rtParentClient.Height(), FALSE);

    CRect rtClient;
    GetClientRect(rtClient);
    rtClient.DeflateRect(5, 5);

    const long labelHeight = 21;
    const long labelGap = 3;

    long offsetY = labelGap;

    auto labelWidth = rtClient.Width() / 2;
    auto labelL = rtClient.left;
    auto labelR = rtClient.left + labelWidth;

    UpdateLayoutLabelAndValueUI(labelL, offsetY, labelWidth, labelHeight, m_Label_JobFile, m_Label_JobFileName);
    UpdateLayoutLabelAndValueUI(labelR, offsetY, labelWidth, labelHeight, m_Label_Connect1, m_Label_Connect2);
    offsetY += labelHeight + labelGap;

    m_LabelResult.MoveWindow(labelL, offsetY, rtClient.Width(), labelHeight);
    offsetY += labelHeight + labelGap;

    CRect rtLotViewRegion;

    rtLotViewRegion.left = rtClient.left;
    rtLotViewRegion.top = offsetY;
    rtLotViewRegion.right = rtClient.right;
    rtLotViewRegion.bottom = offsetY + (rtClient.bottom - offsetY) * 2 / 3;

    ::MoveWindow(m_imageLotView->GetSafeHwnd(), rtLotViewRegion.left, rtLotViewRegion.top, rtLotViewRegion.Width(),
        rtLotViewRegion.Height(), TRUE);

    offsetY += rtLotViewRegion.Height() + labelGap;

    CRect rtTab(rtClient.left, offsetY, rtClient.right, rtClient.bottom);
    m_tabResult.MoveWindow(rtTab);
    m_tabResult.AdjustRect(FALSE, rtTab);

    m_visionCommonSpecDlg->MoveWindow(rtTab);
    m_visionCommonResultDlg->MoveWindow(rtTab);
    m_visionCommonDetailResultDlg->MoveWindow(rtTab);
    m_visionCommonTextResultDlg->MoveWindow(rtTab);
    m_visionCommonDebugInfoDlg->MoveWindow(rtTab);
    m_visionCommonLogDlg->MoveWindow(rtTab);
}

void CDlgVisionPrimaryUI::UpdateLayoutLabelAndValueUI(
    int32_t x, int32_t y, int32_t width, int32_t height, CNewLabel& label, CNewLabel& value)
{
    const long labelGap = 3;
    const long leftLabelWidth = 200;

    label.MoveWindow(x, y, leftLabelWidth, height);
    auto valueX = x + leftLabelWidth + labelGap;
    value.MoveWindow(valueX, y, width - leftLabelWidth - labelGap, height);
}

void CDlgVisionPrimaryUI::checkInstallerMonitorProgram()
{
    // Inline 중에는 체크하지 말자
    if (m_visionMainAgent.isInline())
        return;

    CString monitorExeFileName = _T("IntekPlus.iPIS-500I.Monitor");
    monitorExeFileName.MakeLower();

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("CreateToolhelp32Snapshot error \n"));
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        _tprintf(_T("Process32First error ! \n"));
        CloseHandle(hProcessSnap);
        return;
    }

    do
    {
        CString processExeFile = pe32.szExeFile;
        processExeFile.MakeLower();

        if (processExeFile.Find(monitorExeFileName) >= 0)
        {
            // 이미 실행중이다
            CloseHandle(hProcessSnap);
            return;
        }
    }
    while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    CString valueName = _T("IntekPlus.iPIS-500I.InstallerMonitor");

    HKEY hKey;
    DWORD type = REG_NONE;

    auto lRes = ::RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
        KEY_WOW64_64KEY | KEY_ALL_ACCESS, &hKey);
    if (lRes == ERROR_SUCCESS)
    {
        DWORD cbData = 8192;
        BYTE data[8192];

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, valueName, NULL, &type, data, &cbData))
        {
            CString excuteFilePath((TCHAR*)data);
            STARTUPINFO si = {sizeof(si)};
            PROCESS_INFORMATION pi = {};

            CreateProcess(
                nullptr, LPTSTR(LPCTSTR(excuteFilePath)), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
            //BUG: pi.hThread 핸들을 닫아 주어야 함
            CloseHandle(pi.hProcess);
        }
    }
}

CCriticalSection g_Check_iGrab_Board_Temperature_Lock;

#define IGRAB_BOARD_TEMPERATURE_LOG_WRITE 5

static long g_niGrab_Board_Temperature_LogWrite_Count = 0;

void CDlgVisionPrimaryUI::check_iGrab_Board_Temperature_Check()
{
    //Critical Section 사용
    g_Check_iGrab_Board_Temperature_Lock.Lock();

    float fiGrab_Board_Temperature(-1.f);

    if (m_visionMainAgent.isInline() == true) //Inline 상태면
    {
        fiGrab_Board_Temperature = FrameGrabber::GetInstance().get_grabber_temperature(0);
    }
    else
    {
        //Inline 상태가 아니면 그냥 1분마다 함수를 이용하여 확인한다
        FrameGrabber::GetInstance().set_grabber_temperature(0);
        fiGrab_Board_Temperature = FrameGrabber::GetInstance().get_grabber_temperature(0);
    }

    if (fiGrab_Board_Temperature <= 10.f) //mc_간혹가다가 값이 0이 나오는 경우가 있다 10도미만이면 보내지 말자
        return;

    //Handler로 현재 온도를 알려준다
    if (m_visionMainAgent.m_pMessageSocket != nullptr && m_visionMainAgent.m_pMessageSocket->IsConnected() == true)
        m_visionMainAgent.m_pMessageSocket->Write(MSG_PROBE_SEND_IGRAB_BOARD_TEMPERATURE,
            sizeof(fiGrab_Board_Temperature), (float*)&fiGrab_Board_Temperature);

    //Vision에서 Option화를 하여 저장하며, 5분마다 저장한다.
    if (g_niGrab_Board_Temperature_LogWrite_Count == IGRAB_BOARD_TEMPERATURE_LOG_WRITE
        && SystemConfig::GetInstance().GetiGrab_Board_Temperature_Log_save_Vision() == true)
    {
        CFile File;
        CString strFileName("");

        // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
        strFileName.Format(_T("%s\\VisionTemperatureLog(%s)_%s.txt"),
            (LPCTSTR)DynamicSystemPath::get(DefineFolder::Log), (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo,
            (LPCTSTR)CTime::GetCurrentTime().Format(_T("%Y%m%d")));

        if (!File.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
            return;
        else
        {
            SYSTEMTIME cur_time;
            GetLocalTime(&cur_time);

            // 파일의 맨 마지막을 찾는다.
            File.SeekToEnd();

            CString strWriteTime("");
            CString strWriteData("");
            strWriteTime.Format(_T("[%02d:%02d:%02d:%03ld]\t"), cur_time.wHour, cur_time.wMinute, cur_time.wSecond,
                cur_time.wMilliseconds);

            CString strLog("");
            strLog.Format(_T("%05f"), fiGrab_Board_Temperature);
            CArchive ar(&File, CArchive::store);
            ar.WriteString(strWriteTime);
            ar.WriteString(_T("\t"));
            ar.WriteString(strLog);

            ar.WriteString(_T("\r\n"));
            ar.Close();
            File.Close();
        }

        g_niGrab_Board_Temperature_LogWrite_Count = 0;
    }

    if (SystemConfig::GetInstance().GetiGrab_Board_Temperature_Log_save_Vision() == true)
    {
        g_niGrab_Board_Temperature_LogWrite_Count++;
    }

    g_Check_iGrab_Board_Temperature_Lock.Unlock();
}

void CDlgVisionPrimaryUI::OnStnClickedStaticSideFrontPrimary()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_visionMainAgent.GetSideVisionSection() != enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
        m_Label_Side_Rear_Status.SetBkColor(defColor);

        bool bisLiveMode = FrameGrabber::GetInstance().IsLive(); //mc_Live Mode 중이였으면,

        if (bisLiveMode == true)
            ResetLiveMain();

        m_nSideVisionSection = enSideVisionModule::SIDE_VISIONMODULE_FRONT;

        m_visionMainAgent.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

        if (bisLiveMode == true)
            SetLiveMain();
    }
    else
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
    }
    UpdateCommonTabShow();
}

void CDlgVisionPrimaryUI::OnStnClickedStaticSideRearPrimary()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_visionMainAgent.GetSideVisionSection() != enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_Label_Side_Front_Status.SetBkColor(defColor);
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));

        bool bisLiveMode = FrameGrabber::GetInstance().IsLive(); //mc_Live Mode 중이였으면,

        if (bisLiveMode == true)
            ResetLiveMain();

        m_nSideVisionSection = enSideVisionModule::SIDE_VISIONMODULE_REAR;

        m_visionMainAgent.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_REAR);

        if (bisLiveMode == true)
            SetLiveMain();
    }
    else
    {
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
    }
    UpdateCommonTabShow();
}

void CDlgVisionPrimaryUI::SetPrimaryUISideVisionSection()
{
    // Default Color 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    m_nSideVisionSection = m_visionMainAgent.GetSideVisionSection();

    // 혹시 모를 쓰레기 값을 방지하기 위해 FRONT/REAR 값 이외의 값이 들어오면 무조건 FRONT로 변경시킨다.
    switch (m_nSideVisionSection)
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

void CDlgVisionPrimaryUI::UpdateCommonTabShow()
{
    const long nSideSection = m_visionMainAgent.GetSideVisionSection();

    static const bool bIsSideVision = (m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);

    m_visionCommonSpecDlg->ShowWindow(SW_HIDE);

    m_visionCommonResultDlg->ShowWindow(SW_HIDE);
    m_visionCommonDetailResultDlg->ShowWindow(SW_HIDE);
    m_visionCommonDebugInfoDlg->ShowWindow(SW_HIDE);
    m_visionCommonTextResultDlg->ShowWindow(SW_HIDE);

    m_visionCommonLogDlg->ShowWindow(SW_HIDE);

    if (bIsSideVision == true)
    {
        m_visionCommonResultDlgRear->ShowWindow(SW_HIDE);
        m_visionCommonDetailResultDlgRear->ShowWindow(SW_HIDE);
        m_visionCommonDebugInfoDlgRear->ShowWindow(SW_HIDE);
        m_visionCommonTextResultDlgRear->ShowWindow(SW_HIDE);
    }

    if (m_nCurCommonTab == TAB_INSPECTION_SPEC)
    {
        m_visionCommonSpecDlg->ShowWindow(SW_SHOW);
        return;
    }
    else if (m_nCurCommonTab == TAB_TXT_LOG)
    {
        m_visionCommonLogDlg->ShowWindow(SW_SHOW);
        return;
    }

    if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        switch (m_nCurCommonTab)
        {
            case TAB_INSPECTION_RESULT:
                m_visionCommonResultDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_DETAIL_RESULT:
                m_visionCommonDetailResultDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_DEBUG_INFO:
                m_visionCommonDebugInfoDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_TXT_RESULT:
                m_visionCommonTextResultDlgRear->ShowWindow(SW_SHOW);
                break;
            default:
                m_visionCommonSpecDlg->ShowWindow(SW_SHOW);
                break;
        }
    }
    else
    {
        switch (m_nCurCommonTab)
        {
            case TAB_INSPECTION_RESULT:
                m_visionCommonResultDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_DETAIL_RESULT:
                m_visionCommonDetailResultDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_DEBUG_INFO:
                m_visionCommonDebugInfoDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_TXT_RESULT:
                m_visionCommonTextResultDlg->ShowWindow(SW_SHOW);
                break;
            default:
                m_visionCommonSpecDlg->ShowWindow(SW_SHOW);
                break;
        }
    }
}

void CDlgVisionPrimaryUI::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (m_imageLotView != nullptr)
    {
        UpdateLayout();
    }
}
