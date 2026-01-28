//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "MainFrm.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/semiinfo.h"
#include "../HardwareModules/dPI_Framegrabber/FrameGrabber.h"
#include "../InformationModule/dPI_DataBase/SlitBeam3DParameters.h"
#include "../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../MainUiModules/VisionPrimaryUI/DlgVisionPrimaryUI.h"
#include "../MainUiModules/VisionPrimaryUI/VisionPrimaryUI.h"
#include "../ManagementModules/VisionUnit/VisionUnit.h"
#include "../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../VisionNeedLibrary/VisionCommon/VisionMainMenuAccess.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CMainFrame* g_mainFrame = nullptr;

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
ON_WM_CREATE()
ON_COMMAND(ID_JOB_NEWJOB, &CMainFrame::OnJobNewjob)
ON_COMMAND(ID_JOB_JOBOPEN, &CMainFrame::OnJobJobopen)
ON_COMMAND(ID_JOB_JOBSAVE, &CMainFrame::OnJobJobsave)
ON_COMMAND(ID_IMAGE_IMAGEOPEN, &CMainFrame::OnImageImageopen)
ON_COMMAND(ID_IMAGE_IMAGESAVE, &CMainFrame::OnImageImagesave)
ON_COMMAND(ID_IMAGE_GRAB, &CMainFrame::OnImageGrab)
ON_COMMAND(ID_IMAGE_LIVE, &CMainFrame::OnImageLive)
ON_COMMAND(ID_JOB_BATCHINSPECTION, &CMainFrame::OnJobBatchinspection)
ON_COMMAND(ID_JOB_INSPECTIONITEMSETUP, &CMainFrame::OnJobInspectionitemsetup)
ON_COMMAND(ID_JOB_ILLUMSETUP, &CMainFrame::OnJobIllumsetup)
ON_COMMAND(ID_INSPECTION_RUN, &CMainFrame::OnInspectionRun)
ON_COMMAND(ID_INSPECTION_TEACH, &CMainFrame::OnInspectionTeach)
ON_COMMAND(ID_INSPECTION_QUIT, &CMainFrame::OnInspectionQuit)
ON_COMMAND(ID_MARKTEACH_SKIP, &CMainFrame::OnMarkteachSkip)
ON_COMMAND(ID_MARKTEACH_DONE, &CMainFrame::OnMarkteachDone)
ON_COMMAND(ID_HOST_GOTOHOST, &CMainFrame::OnHostGotohost)
ON_COMMAND(ID_JOB_INLINEMODE, &CMainFrame::OnJobInlinemode)
ON_COMMAND(ID_JOB_LOGIN, &CMainFrame::OnJobLogin)
ON_UPDATE_COMMAND_UI(ID_JOB_NEWJOB, &CMainFrame::OnUpdateJobNewjob)
ON_UPDATE_COMMAND_UI(ID_JOB_JOBOPEN, &CMainFrame::OnUpdateJobJobopen)
ON_UPDATE_COMMAND_UI(ID_JOB_JOBSAVE, &CMainFrame::OnUpdateJobJobsave)
ON_UPDATE_COMMAND_UI(ID_JOB_JOBSAVEAS, &CMainFrame::OnUpdateJobJobsaveas)
ON_UPDATE_COMMAND_UI(ID_IMAGE_IMAGEOPEN, &CMainFrame::OnUpdateImageImageopen)
ON_UPDATE_COMMAND_UI(ID_IMAGE_IMAGESAVE, &CMainFrame::OnUpdateImageImagesave)
ON_UPDATE_COMMAND_UI(ID_IMAGE_GRAB, &CMainFrame::OnUpdateImageGrab)
ON_UPDATE_COMMAND_UI(ID_IMAGE_LIVE, &CMainFrame::OnUpdateImageLive)
ON_UPDATE_COMMAND_UI(ID_JOB_BATCHINSPECTION, &CMainFrame::OnUpdateJobBatchinspection)
ON_UPDATE_COMMAND_UI(ID_JOB_INSPECTIONITEMSETUP, &CMainFrame::OnUpdateJobInspectionitemsetup)
ON_UPDATE_COMMAND_UI(ID_JOB_ILLUMSETUP, &CMainFrame::OnUpdateJobIllumsetup)
ON_UPDATE_COMMAND_UI(ID_INSPECTION_RUN, &CMainFrame::OnUpdateInspectionRun)
ON_UPDATE_COMMAND_UI(ID_INSPECTION_TEACH, &CMainFrame::OnUpdateInspectionTeach)
ON_UPDATE_COMMAND_UI(ID_INSPECTION_QUIT, &CMainFrame::OnUpdateInspectionQuit)
ON_UPDATE_COMMAND_UI(ID_MARKTEACH_SKIP, &CMainFrame::OnUpdateMarkteachSkip)
ON_UPDATE_COMMAND_UI(ID_MARKTEACH_DONE, &CMainFrame::OnUpdateMarkteachDone)
ON_UPDATE_COMMAND_UI(ID_HOST_GOTOHOST, &CMainFrame::OnUpdateHostGotohost)
ON_UPDATE_COMMAND_UI(ID_JOB_INLINEMODE, &CMainFrame::OnUpdateJobInlinemode)
ON_UPDATE_COMMAND_UI(ID_JOB_LOGIN, &CMainFrame::OnUpdateJobLogin)
ON_COMMAND(ID_JOB_SYSTEM_SETUP, &CMainFrame::OnJobSystemSetup)
ON_UPDATE_COMMAND_UI(ID_JOB_SYSTEM_SETUP, &CMainFrame::OnUpdateJobSystemSetup)
ON_COMMAND(ID_JOB_IOCHECK, &CMainFrame::OnJobIocheck)
ON_UPDATE_COMMAND_UI(ID_JOB_IOCHECK, &CMainFrame::OnUpdateJobIocheck)
ON_COMMAND(ID_JOB_REVIEWIMAGEVIEWER, &CMainFrame::OnJobReviewimageviewer)
ON_UPDATE_COMMAND_UI(ID_JOB_REVIEWIMAGEVIEWER, &CMainFrame::OnUpdateJobReviewimageviewer)
ON_COMMAND(ID_JOB_OVERLAY_MANAGER, &CMainFrame::OnJobOverlayManager)
ON_UPDATE_COMMAND_UI(ID_JOB_OVERLAY_MANAGER, &CMainFrame::OnUpdateJobOverlayManager)
ON_COMMAND(ID_JOB_JOBSAVEAS, &CMainFrame::OnJobJobsaveas)
ON_MESSAGE(UM_SHOW_TEACH, &CMainFrame::OnShowTeach)
ON_MESSAGE(UM_SHOW_MAIN, &CMainFrame::OnShowMain)
ON_MESSAGE(UM_SOCKET_DATA_RECEIVED, &CMainFrame::OnDataReceivedFromHost)
ON_MESSAGE(UM_SOCKET_CONNECTION_EVENT, &CMainFrame::OnConnectionUpdatedFromHost)
ON_MESSAGE(UM_TUNING_SOCKET_DATA_RECEIVED, &CMainFrame::OnTuningSocketDataReceivedFromHost)
ON_MESSAGE(UM_TUNING_SOCKET_CONNECTION_EVENT, &CMainFrame::OnTuningSocketConnectionUpdatedFromHost)
ON_MESSAGE(UM_VISION_TUNING_LIVE_2D, &CMainFrame::OnTuningLive2DEvent)
ON_MESSAGE(UM_VISION_TUNING_LIVE_3D, &CMainFrame::OnTuningLive3DEvent)
ON_MESSAGE(UM_AUTOMATION_SOCKET_CONNECTION_EVENT, &CMainFrame::OnConnectionUpdatedFromAutomation)
ON_MESSAGE(UM_AUTOMATION_SOCKET_DATA_RECEIVED, &CMainFrame::OnDataReceivedFromAutomation)
ON_COMMAND(ID_BUTTON_RUN_IDL, &CMainFrame::OnButtonRunIdl)
ON_UPDATE_COMMAND_UI(ID_BUTTON_RUN_IDL, &CMainFrame::OnUpdateButtonRunIdl)
ON_REGISTERED_MESSAGE(MsgSendFinalREsultDataVsToMv, &CMainFrame::OnMsgSendFinalREsultDataVsToMv)
END_MESSAGE_MAP()

// CMainFrame 생성/소멸

CMainFrame::CMainFrame()
    : m_visionMain(SystemConfig::GetInstance())
    , m_tuningSlitBeamParameters(nullptr)
    , m_bNeedToExitOfDetailSetup(FALSE)
{
    g_mainFrame = this;
}

CMainFrame::~CMainFrame()
{
    if (m_tuningSlitBeamParameters != nullptr)
        delete m_tuningSlitBeamParameters;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_wndRibbonBar.Create(this);
    //	m_wndRibbonBar.m_bIsCustomizeMenu = FALSE;
    m_wndRibbonBar.LoadFromResource(IDR_RIBBON);
    m_wndRibbonBar.SetWindows7Look(FALSE);

    CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));

    m_visionMain.m_hwndMainFrame = GetSafeHwnd();
    m_visionMain.m_msgShowTeach = UM_SHOW_TEACH;
    m_visionMain.m_msgShowMain = UM_SHOW_MAIN;

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    // 제목에 Document Title을 없애자.. 보기 안좋다
    cs.style = cs.style & (~FWS_ADDTOTITLE);

    if (!CFrameWndEx::PreCreateWindow(cs))
        return FALSE;
    // TODO: CREATESTRUCT cs를 수정하여 여기에서
    //  Window 클래스 또는 스타일을 수정합니다.

    RECT rtWorkArea;

    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rtWorkArea, 0);

    cs.x = 0;
    cs.y = 0;
    cs.cx = rtWorkArea.right - rtWorkArea.left;
    cs.cy = rtWorkArea.bottom - rtWorkArea.top;

    return TRUE;
}

// CMainFrame 진단

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

void CMainFrame::OnJobNewjob()
{
    // TODO: 여기에 명령 처리기 코드를 추가합니다.
    static const bool bHWExist = static_cast<bool>(SystemConfig::GetInstance().IsHardwareExist());

    if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_SHIFT)
        && m_visionMain.m_systemConfig.m_nVisionType == VISIONTYPE_NGRV_INSP && bHWExist == false)
    {
        m_visionMain.m_isNGRVSingleRun = !m_visionMain.m_isNGRVSingleRun;

        return;
    }

    if (m_visionMain.SetPackageType())
    {
        m_visionMain.SaveJobFile(_T("New Job"), VisionMainAgent::SaveJobMode::Normal, TRUE, TRUE);
    }
}

void CMainFrame::OnJobJobopen()
{
    m_visionMain.VisionPrimaryUI_OpenJobFile();
}

void CMainFrame::OnJobJobsave()
{
    m_visionMain.VisionPrimaryUI_SaveJobFile();
}

void CMainFrame::OnImageImageopen()
{
    m_visionMain.VisionPrimaryUI_OpenImageFile();
}

void CMainFrame::OnImageImagesave()
{
    m_visionMain.VisionPrimaryUI_SaveImageFile();
}

void CMainFrame::OnImageGrab()
{
    m_visionMain.OnImageGrab();
}

void CMainFrame::OnImageLive()
{
    if (FrameGrabber::GetInstance().IsLive() == TRUE)
    {
        m_visionMain.ResetLiveMain();
    }
    else
    {
        m_visionMain.SetLiveMain();
    }
}

void CMainFrame::OnJobBatchinspection()
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

    m_visionMain.ShowBatchInspectionUI(GetActiveView()->GetSafeHwnd());

    m_visionMain.HideVisionPrimaryUI();
}

void CMainFrame::OnJobInspectionitemsetup()
{
    m_visionMain.ShowVisionInspSpec();
}

void CMainFrame::OnJobIllumsetup()
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

    m_visionMain.ShowIlluminationTeachUI(GetActiveView()->GetSafeHwnd());

    m_visionMain.HideVisionPrimaryUI();
}

void CMainFrame::OnInspectionRun()
{
    if (!m_visionMain.DoInspection())
    {
        AfxMessageBox(_T("Fail to inspect!"));
    }
}

void CMainFrame::OnInspectionTeach()
{
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

    m_visionMain.ShowProcessingTeachingUI(GetActiveView()->GetSafeHwnd());

    m_bNeedToExitOfDetailSetup = TRUE;

    m_visionMain.HideVisionPrimaryUI();
}

void CMainFrame::OnInspectionQuit()
{
    m_bNeedToExitOfDetailSetup = FALSE;

    m_visionMain.ReturnToPrimaryUI();
}

void CMainFrame::OnMarkteachSkip()
{
    m_visionMain.iPIS_MarkTeach_Skip();
}

void CMainFrame::OnMarkteachDone()
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

void CMainFrame::OnHostGotohost()
{
    m_visionMain.OnGotoHost();
}

void CMainFrame::OnJobInlinemode()
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

        OnJobLogin();
        return;
    }

    if (m_visionMain.IsInlineMode() == false)
        m_visionMain.SetInlineStart(false);
    else
        m_visionMain.SetInlineStop(false);
}

void CMainFrame::OnJobLogin()
{
    m_visionMain.OnLogin();

    SetCurrentFrameString();
}

// 영훈 [ Main Title Name ] 20130819 : iPack title에 설정한 값을 입력할 수 있도록 한다
void CMainFrame::SetCurrentFrameString()
{
    CString strAccessMode;
    CString strOut;

    if (m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR)
        strAccessMode.Format(_T("[Operator Mode]"));
    else if (m_visionMain.m_systemConfig.m_nCurrentAccessMode == _ENGINEER)
        strAccessMode.Format(_T("[Engineer Mode]"));
    else if (m_visionMain.m_systemConfig.m_nCurrentAccessMode == _INTEKPLUS)
        strAccessMode.Format(_T("[Intekplus Mode]"));

    //{{ //kircheis_TRV //kircheis_100M
    static const long nVisionType = SystemConfig::GetInstance().m_nVisionType;
    static const long nSideVisionID = (long)(SystemConfig::GetInstance().GetSideVisionNumber());
    static const bool isNGRVInspMode = SystemConfig::GetInstance().IsNGRVInspectionMode();

    CString strIPack("iPack");

    if (nVisionType == VISIONTYPE_2D_INSP)
    {
        if (isNGRVInspMode)
            strIPack.Format(_T("iPack for NGRV Inspection Mode"));
        else
            strIPack.Format(_T("iPack for 2D"));
    }
    else if (nVisionType == VISIONTYPE_3D_INSP)
        strIPack.Format(_T("iPack for 3D"));
    else if (nVisionType == VISIONTYPE_NGRV_INSP)
    {
        if (SystemConfig::GetInstance().IsNgrvSwirOptics() == true)
            strIPack.Format(_T("iPack for NGRV to SWIR"));
        else
            strIPack.Format(_T("iPack for NGRV"));
    }
        
    else if (nVisionType == VISIONTYPE_SIDE_INSP)
    {
        if (nSideVisionID == SIDE_VISIONNUMBER_1)
            strIPack.Format(_T("iPack for side #1"));
        else if (nSideVisionID == SIDE_VISIONNUMBER_2)
            strIPack.Format(_T("iPack for side #2"));
        else
            strIPack.Format(_T("iPack for side unknown"));
    }
    else if (nVisionType == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
        strIPack.Format(_T("iPack for SWIR"));
    else
        strIPack.Format(_T("iPack for Unknown"));

    strOut.Format(_T("%s %s"), (LPCTSTR)strIPack, (LPCTSTR)strAccessMode);

    SetWindowText(strOut);
}

void CMainFrame::InitializeMessageSocket()
{
    if (m_visionMain.m_pMessageSocket)
    {
        return;
    }

    if (PersonalConfig::getInstance().isSocketConnectionEnabled())
    {
        m_visionMain.m_pMessageSocket
            = new Ipvm::SocketMessaging({0, 5000, GetSafeHwnd(), UM_SOCKET_CONNECTION_EVENT, UM_SOCKET_DATA_RECEIVED});
        m_visionMain.m_tuningMessageSocket = new Ipvm::SocketMessaging(
            {0, 5001, GetSafeHwnd(), UM_TUNING_SOCKET_CONNECTION_EVENT, UM_TUNING_SOCKET_DATA_RECEIVED});
        if (m_visionMain.m_systemConfig.m_nAutomationTestMode == 1)
        {
            m_visionMain.m_automationMessageSocket = new Ipvm::SocketMessaging(
                {0, 5002, GetSafeHwnd(), UM_AUTOMATION_SOCKET_CONNECTION_EVENT, UM_AUTOMATION_SOCKET_DATA_RECEIVED});
        }

        //mc_NGRV Single Run
        m_visionMain.SetSocketInfo(m_visionMain.m_pMessageSocket);
    }
}

void CMainFrame::OnUpdateJobNewjob(CCmdUI* pCmdUI)
{
    // TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode() == true
        || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.IsJobTeachEnabled() == false)
    {
        pCmdUI->Enable(FALSE);
    }
    else
    {
        pCmdUI->Enable(TRUE);
    }
}

void CMainFrame::OnUpdateJobJobopen(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_menuAccessUI)
    {
        pCmdUI->Enable(m_visionMain.m_menuAccessUI->isJobOpenEnable());
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode() == true
        || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.IsJobTeachEnabled() == false)
    {
        pCmdUI->Enable(FALSE);
    }
    else
    {
        pCmdUI->Enable(TRUE);
    }
}

void CMainFrame::OnUpdateJobJobsave(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_menuAccessUI)
    {
        pCmdUI->Enable(m_visionMain.m_menuAccessUI->isJobSaveEnable());
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.m_pMessageSocket->IsConnected()
        || m_visionMain.IsInlineMode() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateJobJobsaveas(CCmdUI* pCmdUI)
{
    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateImageImageopen(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_menuAccessUI)
    {
        pCmdUI->Enable(m_visionMain.m_menuAccessUI->isImageOpenEnable());
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateImageImagesave(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_menuAccessUI)
    {
        pCmdUI->Enable(m_visionMain.m_menuAccessUI->isImageSaveEnable());
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateImageGrab(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_menuAccessUI)
    {
        pCmdUI->Enable(m_visionMain.m_menuAccessUI->isGrabEnable());
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateImageLive(CCmdUI* pCmdUI)
{
    if (m_visionMain.IsInlineMode() || m_visionMain.IsShowVisionPrimaryUI() == false)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateJobBatchinspection(CCmdUI* pCmdUI)
{
    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false
        || m_visionMain.m_systemConfig.m_nVisionType == VISIONTYPE_NGRV_INSP)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateJobInspectionitemsetup(CCmdUI* pCmdUI)
{
    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.IsJobTeachEnabled() == false)
    {
        pCmdUI->Enable(FALSE);
    }
    else
    {
        pCmdUI->Enable(TRUE);
    }
}

void CMainFrame::OnUpdateJobIllumsetup(CCmdUI* pCmdUI)
{
    if (FrameGrabber::GetInstance().IsLive() || m_visionMain.IsInlineMode()
        || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.IsJobTeachEnabled() == false)
    {
        pCmdUI->Enable(FALSE);
    }
    else
    {
        pCmdUI->Enable(TRUE);
    }
}

void CMainFrame::OnUpdateInspectionRun(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_menuAccessUI)
    {
        pCmdUI->Enable(m_visionMain.m_menuAccessUI->isInspectionEnable());
        return;
    }

    if (FrameGrabber::GetInstance().IsLive() == false && m_visionMain.IsInlineMode() == false
        && (m_visionMain.IsShowVisionPrimaryUI() || m_visionMain.IsShowProcessingTeachingUI())
        && m_visionMain.m_systemConfig.m_nVisionType != VISIONTYPE_NGRV_INSP)
        pCmdUI->Enable(TRUE);
    else
        pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateInspectionTeach(CCmdUI* pCmdUI)
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
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateInspectionQuit(CCmdUI* pCmdUI)
{
    if (m_visionMain.IsShowVisionPrimaryUI() == false && m_visionMain.IsShowVisionInlineUI() == false)
        pCmdUI->Enable(TRUE);
    else
        pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateMarkteachSkip(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_bMarkTeach == TRUE)
        pCmdUI->Enable(TRUE);
    else
        pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateMarkteachDone(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_pMessageSocket->IsConnected() == false || m_visionMain.IsInlineMode())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateHostGotohost(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_pMessageSocket->IsConnected() == true
        && (m_visionMain.IsShowVisionPrimaryUI() == true || m_visionMain.IsShowVisionInlineUI() == true))
        pCmdUI->Enable(TRUE);
    else
        pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateJobInlinemode(CCmdUI* pCmdUI)
{
    if (m_visionMain.IsShowVisionPrimaryUI() == true || m_visionMain.IsShowVisionInlineUI() == true)
        pCmdUI->Enable(TRUE);
    else
        pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateJobLogin(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_pMessageSocket->IsConnected() == true || m_visionMain.IsShowVisionPrimaryUI() == false)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnJobSystemSetup()
{
    m_visionMain.SystemSetup();
}

void CMainFrame::OnUpdateJobSystemSetup(CCmdUI* pCmdUI)
{
    if (m_visionMain.IsInlineMode() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnJobIocheck()
{
    m_visionMain.Iocheck();
}

void CMainFrame::OnUpdateJobIocheck(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnJobReviewimageviewer()
{
    m_visionMain.ReviewImageViewer();
}

void CMainFrame::OnUpdateJobReviewimageviewer(CCmdUI* pCmdUI)
{
    if (m_visionMain.IsInlineMode() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false
        || m_visionMain.m_systemConfig.m_nVisionType == VISIONTYPE_NGRV_INSP)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnJobOverlayManager()
{
    m_visionMain.ShowOverlayManager();
}

void CMainFrame::OnUpdateJobOverlayManager(CCmdUI* pCmdUI)
{
    if (m_visionMain.IsInlineMode() || m_visionMain.m_systemConfig.m_nCurrentAccessMode == _OPERATOR
        || m_visionMain.IsShowVisionPrimaryUI() == false
        || m_visionMain.m_systemConfig.m_nVisionType == VISIONTYPE_NGRV_INSP)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);
}

void CMainFrame::OnJobJobsaveas()
{
    // Shine 20150114 : Job Save AS 기능 추가
    m_visionMain.SaveJobFile();
}

LRESULT CMainFrame::OnShowTeach(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    OnInspectionTeach();

    return 1L;
}

LRESULT CMainFrame::OnShowMain(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    OnInspectionQuit();

    return 1L;
}

void CMainFrame::OnButtonRunIdl()
{
    m_visionMain.RuniDL();
}

void CMainFrame::OnUpdateButtonRunIdl(CCmdUI* pCmdUI)
{
    if (m_visionMain.m_systemConfig.m_bUseAiInspection)
        pCmdUI->Enable(TRUE);
    else
        pCmdUI->Enable(FALSE);
}

LRESULT CMainFrame::OnMsgSendFinalREsultDataVsToMv(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    m_visionMain.m_mutSend.Lock();
    ::SetEvent(m_visionMain.m_Control_SendResult.m_Flag_ThreadRun);
    m_visionMain.m_mutSend.Unlock();
    return 0L;
}

LRESULT CMainFrame::OnMsgSendDataCheck(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    ::SetEvent(m_visionMain.m_Control_SendConfirm.m_Signal_SendResult);
    return 0L;
}