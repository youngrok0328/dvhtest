//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ViewVision.h"

//CPP_2_________________________________ This project's headers
#include "DialogITRV.h"
#include "FileVersion.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../DefineModules/dA_Base/semiinfo.h"
#include "../HardwareModules/dPI_Framegrabber/FrameGrabber.h"
#include "../InformationModule/dPI_DataBase/SlitBeam3DParameters.h"
#include "../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../MainUiModules/VisionPrimaryUITR/DlgVisionPrimaryUI.h"
#include "../MainUiModules/VisionPrimaryUITR/VisionPrimaryUI.h"
#include "../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../VisionNeedLibrary/VisionCommon/VisionMainMenuAccess.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum class enumMenu : int32_t
{
    InlineMode,

    Open,
    Save,
    SaveAs,

    ImageOpen,
    ImageSave,

    Grab,
    Live,

    BatchInspection,
    InspItemSetup,
    IllumItemSetup,
    RunInspection,
    JobTeaching,
    TeachQuit,

    NextPocket,
    JobSendToHost,

    iPackOption,
    IOCheck,
    OverlayManager,
};

IMPLEMENT_DYNAMIC(ViewVision, CDialogEx)

BEGIN_MESSAGE_MAP(ViewVision, CDialogEx)
ON_WM_CREATE()
ON_WM_SIZE()
ON_MESSAGE(UM_SHOW_TEACH, &ViewVision::OnShowTeach)
ON_MESSAGE(UM_SHOW_MAIN, &ViewVision::OnShowMain)
ON_MESSAGE(UM_SOCKET_DATA_RECEIVED, &ViewVision::OnDataReceivedFromHost)
ON_MESSAGE(UM_SOCKET_CONNECTION_EVENT, &ViewVision::OnConnectionUpdatedFromHost)
ON_MESSAGE(UM_ACCESS_MODE_CHANGED, &ViewVision::OnAccessModeChanged)
ON_MESSAGE(UM_TUNING_SOCKET_DATA_RECEIVED, &ViewVision::OnTuningSocketDataReceivedFromHost)
ON_MESSAGE(UM_TUNING_SOCKET_CONNECTION_EVENT, &ViewVision::OnTuningSocketConnectionUpdatedFromHost)
ON_MESSAGE(UM_VISION_TUNING_LIVE_2D, &ViewVision::OnTuningLive2DEvent)
ON_MESSAGE(UM_VISION_TUNING_LIVE_3D, &ViewVision::OnTuningLive3DEvent)
ON_MESSAGE(UM_AUTOMATION_SOCKET_CONNECTION_EVENT, &ViewVision::OnConnectionUpdatedFromAutomation)
ON_MESSAGE(UM_AUTOMATION_SOCKET_DATA_RECEIVED, &ViewVision::OnDataReceivedFromAutomation)
END_MESSAGE_MAP()


ViewVision::ViewVision(DialogITRV* parent, int32_t visionIndex, HANDLE flag_InlineStart, HANDLE flag_SnycStart)
    : CDialogEx(IDD_VIEW_VISION, parent)
    , m_visionIndex(visionIndex)
    , m_visionMain(SystemConfig::GetInstance(), visionIndex, flag_InlineStart, flag_SnycStart)
    , m_parent(parent)
    , m_bNeedToExitOfDetailSetup(FALSE)
    , m_tuningSlitBeamParameters(nullptr)
{
}

ViewVision::~ViewVision()
{
    if (m_tuningSlitBeamParameters != nullptr)
        delete m_tuningSlitBeamParameters;
}

bool ViewVision::IsShowProcessingTeachingUI()
{
    return m_visionMain.IsShowProcessingTeachingUI();
}

void ViewVision::ShowAccessModeDialog()
{
    m_visionMain.OnLogin();
}

void ViewVision::ShowIlluminationTeachUI()
{
    ShowWindow(SW_SHOW);

    m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView
        ->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거

    m_visionMain.ShowIlluminationTeachUI(m_wndBody->GetSafeHwnd());
    m_visionMain.HideVisionPrimaryUI();
}

void ViewVision::ShowBatchInspection()
{
    ShowWindow(SW_SHOW);

    m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거

    m_visionMain.ShowBatchInspectionUI(m_wndBody->GetSafeHwnd());
    m_visionMain.HideVisionPrimaryUI();
}

void ViewVision::ShowPrimaryUI()
{
    ShowWindow(SW_SHOW);
    m_bNeedToExitOfDetailSetup = FALSE;
    m_visionMain.ReturnToPrimaryUI();
}

void ViewVision::ShowTeach()
{
    ShowWindow(SW_SHOW);

    // switch case문을 사용하여 VISIONTYPE_SIDE_INSP, VISIONTYPE_NGRV_INSP, 그 외로 구분한다. - MED#6_NGRV
    long nVisionType = SystemConfig::GetInstance().GetVisionType();
    switch (nVisionType)
    {
        case VISIONTYPE_SIDE_INSP:
        {
            m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView
                ->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거
            m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotViewRearSide
                ->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거
        }
        break;

        case VISIONTYPE_NGRV_INSP:
        {
            m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView
                ->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거
        }
        break;

        default:
        {
            m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView
                ->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거
        }
        break;
    }

    m_visionMain.ShowProcessingTeachingUI(m_wndBody->GetSafeHwnd());

    m_bNeedToExitOfDetailSetup = TRUE;

    m_visionMain.HideVisionPrimaryUI();
}

void ViewVision::UpdateRibbonMenuState()
{
    m_wndRibbonBar.CheckButtons();
}

void ViewVision::UpdateLayout()
{
    CRect rtClient;
    GetClientRect(rtClient);

    auto cx = rtClient.Width();
    auto cy = rtClient.Height();

    auto titleHeight = 30;
    auto menuHeight = 100;
    m_title.MoveWindow(0, 0, cx, titleHeight, TRUE);
    m_wndRibbonBar.MoveWindow(0, titleHeight, cx, menuHeight, TRUE);
    m_wndBody->MoveWindow(0, titleHeight + menuHeight, cx, cy - titleHeight - menuHeight, TRUE);
    m_title.Invalidate(FALSE);
}

void ViewVision::OnClickedMenuButton(int32_t buttonId)
{
    auto menu = static_cast<enumMenu>(buttonId);

    switch (menu)
    {
        case enumMenu::InlineMode:
            OnJobInlinemode();
            break;
        case enumMenu::Open:
            m_visionMain.VisionPrimaryUI_OpenJobFile();
            break;
        case enumMenu::Save:
            m_visionMain.VisionPrimaryUI_SaveJobFile();
            break;
        case enumMenu::SaveAs:
            m_visionMain.SaveJobFile();
            break;
        case enumMenu::ImageOpen:
            m_visionMain.VisionPrimaryUI_OpenImageFile();
            break;
        case enumMenu::ImageSave:
            m_visionMain.VisionPrimaryUI_SaveImageFile();
            break;
        case enumMenu::Grab:
            m_visionMain.OnImageGrab();
            break;
        case enumMenu::Live:
            if (FrameGrabber::GetInstance().IsLive() == TRUE)
            {
                m_visionMain.ResetLiveMain();
            }
            else
            {
                m_visionMain.SetLiveMain();
            }
            break;
        case enumMenu::BatchInspection:
            OnJobBatchinspection();
            break;
        case enumMenu::InspItemSetup:
            m_visionMain.ShowVisionInspSpec();
            break;
        case enumMenu::IllumItemSetup:
            OnJobIllumsetup();
            break;

        case enumMenu::RunInspection:
            if (!m_visionMain.DoInspection())
            {
                AfxMessageBox(_T("Fail to inspect!"));
            }
            break;

        case enumMenu::JobTeaching:
            OnInspectionTeach();
            break;
        case enumMenu::TeachQuit:
            OnInspectionQuit();
            break;

        case enumMenu::NextPocket:
            m_visionMain.iPIS_MarkTeach_Skip();
            break;
        case enumMenu::JobSendToHost:
            OnMarkteachDone();
            break;

        case enumMenu::iPackOption:
            m_visionMain.SystemSetup();
            break;
        case enumMenu::IOCheck:
            m_visionMain.Iocheck();
            break;
        case enumMenu::OverlayManager:
            m_visionMain.ShowOverlayManager();
            break;
    }
}

void ViewVision::OnJobBatchinspection()
{
    m_parent->ShowBatchInspection(m_visionIndex);
}

void ViewVision::OnJobIllumsetup()
{
    m_parent->ShowIllumSetup(m_visionIndex);
}

void ViewVision::OnInspectionTeach()
{
    m_parent->ShowTeach(m_visionIndex);
}

void ViewVision::OnInspectionQuit()
{
    m_parent->ReturnToPrimaryUI();
}

void ViewVision::OnMarkteachDone()
{
    if (m_bNeedToExitOfDetailSetup)
    {
        ::SendMessage(m_visionMain.m_hwndMainFrame, m_visionMain.m_msgShowMain, 0, 0);
    }

    m_visionMain.CloseProcessingTeachingUI(TRUE);
    m_visionMain.OnMarkTeachDone();

    // 영훈 20151214 ; 버튼을 한번 눌렀으면 이제 Operator는 Mark Skip을 사용할 수 없다.
    m_visionMain.m_pVisionPrimaryUI->Dlg_SetMarkSipMode(FALSE);

    if (m_bNeedToExitOfDetailSetup)
    {
        ::SendMessage(m_visionMain.m_hwndMainFrame, m_visionMain.m_msgShowMain, 0, 0);
    }
}

void ViewVision::OnJobInlinemode()
{
    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView
            ->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거
    }
    else
    {
        m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView
            ->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거
        m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotViewRearSide
            ->CloseProfileView(); // SDY Profile view를 다른 상태로 이동할때 제거
    }

    if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_SHIFT))
    {
        //-------------------------------------------------------------
        // Inline 중에는 Login을 위해서
        // Host로 접속해서 권한을 바꾸어야 하는데
        // 이것 저것 세팅을 위해서 왔다갔다 거리기 귀찮아서 임시 조치하였다
        //-------------------------------------------------------------

        m_visionMain.OnLogin();
        return;
    }

    if (m_visionMain.IsInlineMode() == false)
        m_visionMain.SetInlineStart(false);
    else
        m_visionMain.SetInlineStop(false);
}

void ViewVision::OnUpdateMenuButton(int32_t buttonId, bool& enable)
{
    auto menu = static_cast<enumMenu>(buttonId);

    switch (menu)
    {
        case enumMenu::InlineMode:
            OnUpdateJobInlinemode(enable);
            break;
        case enumMenu::Open:
            OnUpdateJobJobopen(enable);
            break;
        case enumMenu::Save:
            OnUpdateJobJobsave(enable);
            break;
        case enumMenu::SaveAs:
            OnUpdateJobJobsaveas(enable);
            break;
        case enumMenu::ImageOpen:
            OnUpdateImageImageopen(enable);
            break;
        case enumMenu::ImageSave:
            OnUpdateImageImagesave(enable);
            break;
        case enumMenu::Grab:
            OnUpdateImageGrab(enable);
            break;
        case enumMenu::Live:
            OnUpdateImageLive(enable);
            break;
        case enumMenu::BatchInspection:
            OnUpdateJobBatchinspection(enable);
            break;
        case enumMenu::InspItemSetup:
        case enumMenu::IllumItemSetup:
            OnUpdateJobIllumOrInspItemSetup(enable);
            break;

        case enumMenu::RunInspection:
            OnUpdateInspectionRun(enable);
            break;

        case enumMenu::JobTeaching:
            OnUpdateInspectionTeach(enable);
            break;
        case enumMenu::TeachQuit:
            OnUpdateInspectionQuit(enable);
            break;

        case enumMenu::NextPocket:
            OnUpdateMarkteachSkip(enable);
            break;
        case enumMenu::JobSendToHost:
            OnUpdateMarkteachDone(enable);
            break;

        case enumMenu::iPackOption:
            OnUpdateJobSystemSetup(enable);
            break;
        case enumMenu::IOCheck:
            OnUpdateJobIocheck(enable);
            break;
        case enumMenu::OverlayManager:
            OnUpdateJobOverlayManager(enable);
            break;
    }
}

void ViewVision::OnUpdateJobInlinemode(bool& enable)
{
    if (m_visionMain.IsShowVisionPrimaryUI() == true || m_visionMain.IsShowVisionInlineUI() == true)
        enable = true;
    else
        enable = false;
}

void ViewVision::OnUpdateJobJobopen(bool& enable)
{
    if (m_visionMain.m_menuAccessUI)
    {
        enable = m_visionMain.m_menuAccessUI->isJobOpenEnable();
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode() == true
        || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.IsJobTeachEnabled() == false)
    {
        enable = false;
    }
    else
    {
        enable = true;
    }
}

void ViewVision::OnUpdateJobJobsave(bool& enable)
{
    if (m_visionMain.m_menuAccessUI)
    {
        enable = m_visionMain.m_menuAccessUI->isJobSaveEnable();
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.m_pMessageSocket->IsConnected()
        || m_visionMain.IsInlineMode() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateJobJobsaveas(bool& enable)
{
    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateImageImageopen(bool& enable)
{
    if (m_visionMain.m_menuAccessUI)
    {
        enable = m_visionMain.m_menuAccessUI->isImageOpenEnable();
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateImageImagesave(bool& enable)
{
    if (m_visionMain.m_menuAccessUI)
    {
        enable = m_visionMain.m_menuAccessUI->isImageSaveEnable();
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateImageGrab(bool& enable)
{
    if (m_visionMain.m_menuAccessUI)
    {
        enable = m_visionMain.m_menuAccessUI->isGrabEnable();
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateImageLive(bool& enable)
{
    if (m_visionMain.IsInlineMode() || m_visionMain.IsShowVisionPrimaryUI() == false)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateJobBatchinspection(bool& enable)
{
    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false
        || m_visionMain.m_systemConfig.m_nVisionType == VISIONTYPE_NGRV_INSP)
        enable = false;
    else
        enable = true;
}
void ViewVision::OnUpdateJobIllumOrInspItemSetup(bool& enable)
{
    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.IsJobTeachEnabled() == false)
    {
        enable = false;
    }
    else
    {
        enable = true;
    }
}

void ViewVision::OnUpdateInspectionRun(bool& enable)
{
    if (m_visionMain.m_menuAccessUI)
    {
        enable = m_visionMain.m_menuAccessUI->isInspectionEnable();
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() == false && m_visionMain.IsInlineMode() == false
        && (m_visionMain.IsShowVisionPrimaryUI() || m_visionMain.IsShowProcessingTeachingUI())
        && m_visionMain.m_systemConfig.m_nVisionType != VISIONTYPE_NGRV_INSP)
        enable = true;
    else
        enable = false;
}

void ViewVision::OnUpdateInspectionTeach(bool& enable)
{
    // 20140725_Operator_Mark_Teach_Button : Operator 기능에선 Mark Teaching만 사용할 수 있도록하고 Mark가 켜져있지 않다면 비활성화 시킨다.
    BOOL bExistMark = FALSE;

    if (m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR)
    {
        bExistMark = m_visionMain.m_pVisionPrimaryUI->m_visionUnit.IsMarkExist();
    }
    else
    {
        bExistMark = TRUE;
    }

    // NGRV Mode이면서 SingleRun Mode면 true, 아니면 false
    if (FrameGrabber::GetInstance().IsLive() || bExistMark == FALSE || m_visionMain.IsInlineMode()
        || m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.IsJobTeachEnabled() == false
        || (m_visionMain.m_systemConfig.m_nVisionType == VISIONTYPE_NGRV_INSP
            && m_visionMain.m_isNGRVSingleRun == FALSE))
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateInspectionQuit(bool& enable)
{
    if (m_visionMain.IsShowVisionPrimaryUI() == false && m_visionMain.IsShowVisionInlineUI() == false)
        enable = true;
    else
        enable = false;
}

void ViewVision::OnUpdateMarkteachSkip(bool& enable)
{
    if (m_visionMain.m_bMarkTeach == TRUE)
        enable = true;
    else
        enable = false;
}

void ViewVision::OnUpdateMarkteachDone(bool& enable)
{
    if (m_visionMain.m_pMessageSocket->IsConnected() == false || m_visionMain.IsInlineMode())
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateJobSystemSetup(bool& enable)
{
    if (m_visionMain.IsInlineMode() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateJobIocheck(bool& enable)
{
    if (m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnUpdateJobOverlayManager(bool& enable)
{
    if (m_visionMain.IsInlineMode() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false
        || m_visionMain.m_systemConfig.m_nVisionType == VISIONTYPE_NGRV_INSP)
        enable = false;
    else
        enable = true;
}

void ViewVision::OnOK()
{
}

void ViewVision::OnCancel()
{
}

void ViewVision::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

// ViewVision 메시지 처리기

BOOL ViewVision::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_title.Create(L"", WS_BORDER | WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, CRect(0, 0, 1, 1), this);
    switch (m_visionIndex)
    {
        case 0:
            m_title.SetBkColor(RGB(0, 0, 255), RGB(0, 0, 128), CLabel::BackFillMode::Gradient);
            break;
        case 1:
            m_title.SetBkColor(RGB(0, 0, 128), RGB(0, 0, 255), CLabel::BackFillMode::Gradient);
            break;
    }
    m_title.SetTextColor(RGB(255, 255, 255));

    m_wndBody = std::make_shared<ViewVisionBody>(m_visionMain, this);
    m_wndBody->Create(IDD_VIEW_VISION_BODY, this);

    // 영훈 20131226 :: 파일의 버전을 받아오도록 한다.
    CString strExe = _T("iTRV.exe");

    // 영훈 20160802 : Main 화면에 표시횔 Version 부분을 세팅한다.
    FileVersion cFileVer;
    cFileVer.Open(strExe);
    auto appDetailVersion = cFileVer.GetFileVersion2();

    // 영훈 20160802 : 화면에 표시하기
    m_visionMain.SetFileVersion(appDetailVersion);
    m_visionMain.OpenJobFile(m_visionMain.GetPrevJob(), _T(""));

    // 영훈 20160802 : iPack title에 현재 AccessMode를 확인할 수 있도록 한다.
    SetCurrentFrameString();

    UpdateLayout();

    m_wndBody->ShowWindow(SW_SHOW);

    // 여기서부터 소켓을 사용할 수 있게 한다.
    InitializeMessageSocket();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL ViewVision::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if (::GetKeyState(VK_CONTROL) < 0 && pMsg->wParam == 'C')
        {
            pMsg->message = WM_COPY;
        }
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}

int ViewVision::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialogEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_wndRibbonBar.Create(this);
    m_wndRibbonBar.LoadResourceBitmap(IDB_RIBBONMAINLARGE);
    m_wndRibbonBar.SetCallbackButtonCheck(
        [this](int32_t buttonId, bool& enable)
        {
            OnUpdateMenuButton(buttonId, enable);
        });

    m_wndRibbonBar.SetCallbackButtonClicked(
        [this](int32_t buttonId)
        {
            OnClickedMenuButton(buttonId);
        });

    // 메뉴 만들기
    if (auto groupID = m_wndRibbonBar.AddGroup(L"Main"))
    {
        m_wndRibbonBar.AddButton(groupID, 8, L"Inline\nMode", 50, (int32_t)enumMenu::InlineMode);
    }

    if (auto groupId = m_wndRibbonBar.AddGroup(L"Recipe"))
    {
        m_wndRibbonBar.AddButton(groupId, 23, L"Open", 40, (int32_t)enumMenu::Open);
        m_wndRibbonBar.AddButton(groupId, 24, L"Save", 40, (int32_t)enumMenu::Save);
        m_wndRibbonBar.AddButton(groupId, 25, L"Save\nAs", 40, (int32_t)enumMenu::SaveAs);
    }
    if (auto groupId = m_wndRibbonBar.AddGroup(L"Image"))
    {
        m_wndRibbonBar.AddButton(groupId, 15, L"Image\nOpen", 40, (int32_t)enumMenu::ImageOpen);
        m_wndRibbonBar.AddButton(groupId, 16, L"Image\nSave", 40, (int32_t)enumMenu::ImageSave);
        m_wndRibbonBar.AddButton(groupId, 9, L"Grab", 40, (int32_t)enumMenu::Grab);
        m_wndRibbonBar.AddButton(groupId, 13, L"Live", 40, (int32_t)enumMenu::Live);
    }
    if (auto groupId = m_wndRibbonBar.AddGroup(L"Setup"))
    {
        m_wndRibbonBar.AddButton(groupId, 12, L"Batch\nInsp", 40, (int32_t)enumMenu::BatchInspection);
        m_wndRibbonBar.AddButton(groupId, 38, L"Insp\nSetup", 40, (int32_t)enumMenu::InspItemSetup);
        m_wndRibbonBar.AddButton(groupId, 30, L"Illum\nSetup", 40, (int32_t)enumMenu::IllumItemSetup);
        m_wndRibbonBar.AddButton(groupId, 1, L"Run", 40, (int32_t)enumMenu::RunInspection);
        m_wndRibbonBar.AddButton(groupId, 0, L"Teach", 40, (int32_t)enumMenu::JobTeaching);
        m_wndRibbonBar.AddButton(groupId, 4, L"Quit", 40, (int32_t)enumMenu::TeachQuit);
        m_wndRibbonBar.AddButton(groupId, 12, L"Next\nPocket", 50, (int32_t)enumMenu::NextPocket);
        m_wndRibbonBar.AddButton(groupId, 19, L"Job Send\nto Host", 50, (int32_t)enumMenu::JobSendToHost);
    }
    if (auto groupId = m_wndRibbonBar.AddGroup(L"Function"))
    {
        //m_wndRibbonBar.AddButton(groupId, 15, L"Image\nOpen", 40, (int32_t)enumMenu::ImageOpen);
        //m_wndRibbonBar.AddButton(groupId, 16, L"Image\nSave", 40, (int32_t)enumMenu::ImageSave);
        //m_wndRibbonBar.AddButton(groupId, 9, L"Grab", 40, (int32_t)enumMenu::Grab);
        //m_wndRibbonBar.AddButton(groupId, 13, L"Live", 40, (int32_t)enumMenu::Live);
        m_wndRibbonBar.AddButton(groupId, 7, L"Option", 50, (int32_t)enumMenu::iPackOption);
        m_wndRibbonBar.AddButton(groupId, 8, L"IO", 30, (int32_t)enumMenu::IOCheck);
        m_wndRibbonBar.AddButton(groupId, 30, L"Overlay\nManager", 50, (int32_t)enumMenu::OverlayManager);
    }

    m_wndRibbonBar.StartCheckButtonTimer(500);

    CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

    m_visionMain.m_hwndMainFrame = GetSafeHwnd();
    m_visionMain.m_msgShowTeach = UM_SHOW_TEACH;
    m_visionMain.m_msgShowMain = UM_SHOW_MAIN;
    m_visionMain.m_msgAccessModeChanged = UM_ACCESS_MODE_CHANGED;

    return 0;
}

void ViewVision::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_wndBody != nullptr)
    {
        UpdateLayout();
    }
}

// 영훈 [ Main Title Name ] 20130819 : iPack title에 설정한 값을 입력할 수 있도록 한다
void ViewVision::SetCurrentFrameString()
{
    static const long nVisionType = SystemConfig::GetInstance().m_nVisionType;
    static const long nSideVisionID = (long)(SystemConfig::GetInstance().GetSideVisionNumber());

    CString strITRV("iTRV");

    if (nVisionType == VISIONTYPE_2D_INSP)
        strITRV.Format(_T("iTRV for 2D"));
    else if (nVisionType == VISIONTYPE_3D_INSP)
        strITRV.Format(_T("iTRV for 3D"));
    else if (nVisionType == VISIONTYPE_NGRV_INSP)
        strITRV.Format(_T("iTRV for NGRV"));
    else if (nVisionType == VISIONTYPE_SIDE_INSP)
    {
        if (nSideVisionID == SIDE_VISIONNUMBER_1)
            strITRV.Format(_T("iTRV for side #1"));
        else if (nSideVisionID == SIDE_VISIONNUMBER_2)
            strITRV.Format(_T("iTRV for side #2"));
        else
            strITRV.Format(_T("iTRV for side unknown"));
    }
    else if (nVisionType == VISIONTYPE_TR)
    {
        strITRV.Format(_T("iTRV for TR"));
        switch (m_visionIndex)
        {
            case 0:
                strITRV.AppendFormat(L" In Pocket");
                break;

            case 1:
                strITRV.AppendFormat(L" OTI");
                break;
        }
    }

    m_title.SetText(strITRV);
}

void ViewVision::InitializeMessageSocket()
{
    if (m_visionMain.m_pMessageSocket)
    {
        return;
    }

    if (PersonalConfig::getInstance().isSocketConnectionEnabled())
    {
        const uint16_t defaultPort = m_visionIndex == 0 ? 5000 : 6000;
        const uint16_t tuningPort = defaultPort + 1;
        const uint16_t automationPort = defaultPort + 2;

        m_visionMain.m_pMessageSocket
            = new Ipvm::SocketMessaging({0, defaultPort, GetSafeHwnd(), UM_SOCKET_CONNECTION_EVENT, UM_SOCKET_DATA_RECEIVED});
        m_visionMain.m_tuningMessageSocket = new Ipvm::SocketMessaging(
            {0, tuningPort, GetSafeHwnd(), UM_TUNING_SOCKET_CONNECTION_EVENT, UM_TUNING_SOCKET_DATA_RECEIVED});
        if (m_visionMain.m_systemConfig.m_nAutomationTestMode == 1)
        {
            m_visionMain.m_automationMessageSocket = new Ipvm::SocketMessaging(
                {0, automationPort, GetSafeHwnd(), UM_AUTOMATION_SOCKET_CONNECTION_EVENT, UM_AUTOMATION_SOCKET_DATA_RECEIVED});
        }

        //mc_NGRV Single Run
        m_visionMain.SetSocketInfo(m_visionMain.m_pMessageSocket);
    }
}

LRESULT ViewVision::OnShowTeach(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    OnInspectionTeach();

    return 1L;
}

LRESULT ViewVision::OnShowMain(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    OnInspectionQuit();

    return 1L;
}

LRESULT ViewVision::OnMsgSendFinalREsultDataVsToMv(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    m_visionMain.m_mutSend.Lock();
    ::SetEvent(m_visionMain.m_Control_SendResult.m_Flag_ThreadRun);
    m_visionMain.m_mutSend.Unlock();
    return 0L;
}

LRESULT ViewVision::OnMsgSendDataCheck(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    ::SetEvent(m_visionMain.m_Control_SendConfirm.m_Signal_SendResult);
    return 0L;
}