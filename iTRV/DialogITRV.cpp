//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DialogITRV.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../HardwareModules/dPI_SyncController/SyncController.h"
#include "../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
DialogITRV::DialogITRV(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_ITRV, pParent)
    , m_Flag_thread_SyncStart_INPOCKET(::CreateEvent(NULL, TRUE, FALSE, NULL))
    , m_Flag_thread_SyncStart_OTI(::CreateEvent(NULL, TRUE, FALSE, NULL))
    , m_Flag_thread_Inline_INPOCKET(::CreateEvent(NULL, TRUE, FALSE, NULL))
    , m_Flag_thread_Inline_OTI(::CreateEvent(NULL, TRUE, FALSE, NULL))
    , m_Handle_thread_Kill(::CreateEvent(NULL, FALSE, FALSE, NULL))
    , m_Handle_thread(NULL)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    CWinThread* pSyncThread(nullptr);
    pSyncThread = ::AfxBeginThread(ThreadSyncStartFunc, (void*)this, THREAD_PRIORITY_HIGHEST);

    if (pSyncThread == NULL)
    {
        ::SimpleMessage(_T("Failed to create Sync Start Thread\nPlease restart S.W"), MB_OK);
        return;
    }

    HANDLE hNewThread = pSyncThread->m_hThread;
    ::DuplicateHandle(::GetCurrentProcess(), hNewThread, ::GetCurrentProcess(), &m_Handle_thread, 0, FALSE, DUPLICATE_SAME_ACCESS);
}

DialogITRV::~DialogITRV()
{
    ::SetEvent(m_Handle_thread_Kill);

    if (::WaitForSingleObject(m_Handle_thread, 10000) != WAIT_OBJECT_0)
        ::TerminateThread(m_Handle_thread, 555);

    ::CloseHandle(m_Handle_thread_Kill);
    ::CloseHandle(m_Flag_thread_Inline_INPOCKET);
    ::CloseHandle(m_Flag_thread_Inline_OTI);
    ::CloseHandle(m_Flag_thread_SyncStart_INPOCKET);
    ::CloseHandle(m_Flag_thread_SyncStart_OTI);
    ::CloseHandle(m_Handle_thread);
}

UINT DialogITRV::ThreadSyncStartFunc(LPVOID pParam)
{
    DialogITRV* control = (DialogITRV*)pParam;

    return control->ThreadSyncStartFunc();
}

UINT DialogITRV::ThreadSyncStartFunc()
{
    //사용할 Handle을 용도 별로 각각의 배열로 구성한다.
    HANDLE phInlineEnable[] = {m_Flag_thread_Inline_INPOCKET, m_Flag_thread_Inline_OTI, m_Handle_thread_Kill};
    HANDLE phStartSync[] = {m_Flag_thread_SyncStart_INPOCKET, m_Flag_thread_SyncStart_OTI};

    while (1)
    {
        //Inline 관련 플래그를 확인하며 기다린다.
        const DWORD dwRetEnable
            = ::WaitForMultipleObjects(sizeof(phInlineEnable) / sizeof(HANDLE), phInlineEnable, FALSE, INFINITE);

        //Inline 상황이 유지되고 있으면 Kill Thread 플래그를 놓칠 수 있다. 그래서 다시 한번 확인한다.
        const DWORD dwKillThread = ::WaitForSingleObject(m_Handle_thread_Kill, 0);
        if (dwKillThread == WAIT_OBJECT_0)
        {
            return 0;
        }

        //Inpocket도 Inline set이 아니고 OTI도 Inline set이 아니다. 그러면 문제가 있다는 거니까 Thread는 종료한다.
        if (dwRetEnable != WAIT_OBJECT_0 && dwRetEnable != WAIT_OBJECT_0 + 1)
        {
            if (dwRetEnable != WAIT_OBJECT_0 + 2) //Kill Thread 신호가 아니다..
            {
                //경고 메시지 출력 필요
                ::SimpleMessage(_T("Failed to create Sync Start Thread\nPlease restart S.W"), MB_OK);
            }

            return 0;
        }

        //어느 검사쪽이 시작 플래그를 Set 했는지 확인한다. 왜 이작업을 하냐면 둘다 Set 될 수 있기 때문이다.
        //요래 하면 어느 Vision을 Grab 해야하는지 별도의 확인 절차가 필요 없고 dwEableXXX 값들로 바로 알 수 있다.
        //약간의 시차로 인해 둘 다 그랩인 상황에서 한쪽만 먼저 Set 될 수 있다.
        //하지만 Inline Start를 받은 후 실제 그랩(Vision Start 수신 시점) 전까지의 시차로 인해 그런 상황은 커버될거라 기대한다.
        const DWORD dwEnableInpocket = ::WaitForSingleObject(m_Flag_thread_Inline_INPOCKET, 0);
        const DWORD dwEnableOTI = ::WaitForSingleObject(m_Flag_thread_Inline_OTI, 0);

        const bool enableInpocket = (dwEnableInpocket == WAIT_OBJECT_0) ? true : false;
        const bool enableOTI = (dwEnableOTI == WAIT_OBJECT_0) ? true : false;

        if (enableInpocket == true && enableOTI == true) //둘다 Inline 상황인 동시 그랩이다.
        {
            DWORD dwRetStartSync = ::WaitForMultipleObjects(sizeof(phStartSync) / sizeof(HANDLE), phStartSync, TRUE, 0);
            if (dwRetStartSync == WAIT_OBJECT_0)
            {
                //StartSyncBoard를 Inpocket, OTI 모두 켜게 호출 한다.
                //mc_Daul Grab인경우는 Inpocket or OTI로 날리든 한번에 호출된다.
                SyncController::GetInstance().StartSyncBoard(FALSE, true, enTRVisionmodule::TR_VISIONMODULE_INPOCKET);

                ::ResetEvent(m_Flag_thread_SyncStart_INPOCKET);
                ::ResetEvent(m_Flag_thread_SyncStart_OTI);
            }
        }
        else //둘 중 하나만 그랩이다.
        {
            HANDLE hStartSync = enableInpocket == true ? m_Flag_thread_SyncStart_INPOCKET : m_Flag_thread_SyncStart_OTI;
            const DWORD dwRetStartSync = ::WaitForSingleObject(hStartSync, 0);
            if (dwRetStartSync == WAIT_OBJECT_0)
            {
                //StartSyncBoard를 enableInpocket, enableOTI에 맞게 호출 한다.
                if (enableInpocket == true)
                {
                    SyncController::GetInstance().StartSyncBoard(
                        FALSE, FALSE, enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
                }
                else
                {
                    SyncController::GetInstance().StartSyncBoard(FALSE, FALSE, enTRVisionmodule::TR_VISIONMODULE_OTI);
                }

                ::ResetEvent(hStartSync);
            }
        }
    }

    return 0;
}

void DialogITRV::ReturnToPrimaryUI()
{
    long settingVisionIndex = -1;
    // 현재 설정중이였던 비전 인덱스를 얻어온다
    for (auto vision : m_visions)
    {
        if (vision->IsShowProcessingTeachingUI())
        {
            settingVisionIndex = vision->m_visionIndex;
            break;
        }
    }

    if (settingVisionIndex >= 0)
    {
        // 설정중이였던 비전에서 Package Spec을 얻어와서 모든 비전에 동일하게 설정한다
        CiDataBase db;
        m_visions[settingVisionIndex]->m_visionMain.LinkDataBase_PackageSpec(TRUE, db);

        for (auto vision : m_visions)
        {
            if (vision->m_visionIndex != settingVisionIndex)
            {
                vision->m_visionMain.LinkDataBase_PackageSpec(FALSE, db);
            }
        }
    }

    for (auto vision : m_visions)
    {
        vision->ShowPrimaryUI();
    }

    UpdateLayout();
}

void DialogITRV::Crashed(const wchar_t* dumpPathName)
{
    SystemConfig::GetInstance().SetVisionCondition(eVisionCondition::VisionCondition_Wrong);

    //MSG전송으로 V->H이 알려준다..
    if (m_visions.size() == 0)
    {
        return;
    }

    CTime curTime(CTime::GetCurrentTime());

    auto firstVision = m_visions.front();
    auto visionIndex = 0;

    auto masterFileVersion = firstVision->m_visionMain.GetFileVersion();
    CString strFolderName;
    strFolderName.Format(_T("%s%s_%04d-%02d-%02d-%02d-%02d"), CRASH_DUMP_DIRECTORY, (LPCTSTR)masterFileVersion,
        curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), curTime.GetHour(), curTime.GetMinute());
    ::CreateDirectory(strFolderName, NULL);

    // 생성된 파일을 파일 버전이름으로 변경
    CString strBackupCrashDump;
    strBackupCrashDump.Format(_T("%s\\%s_CrashDump.dmp"), (LPCTSTR)strFolderName, (LPCTSTR)masterFileVersion);
    ::MoveFile(dumpPathName, LPCTSTR(strBackupCrashDump));

    // System 파일 저장
    CString strBackupSystem;
    strBackupSystem.Format(_T("%s\\system.ini"), (LPCTSTR)strFolderName);
    ::CopyFile(DynamicSystemPath::get(DefineFile::System), strBackupSystem, FALSE);

    for (auto vision : m_visions)
    {
        // Vision 별로 수행할 업무
        vision->m_visionMain.iPIS_Send_ErrorMessageForPopup(_T("Crash, Please Check Vision"));

        CString visionVersion = vision->m_visionMain.GetFileVersion();
        visionVersion.AppendFormat(L"%d", visionIndex);

        // 이미지 저장
        vision->m_visionMain.SaveDumpImageFiles(strFolderName, visionVersion);

        // Job 저장
        CString strBackupJob;
        strBackupJob.Format(_T("%s\\%s_Job.vmjob"), (LPCTSTR)strFolderName, (LPCTSTR)visionVersion);
        vision->m_visionMain.SaveJobFile(strBackupJob, VisionMainAgent::SaveJobMode::SaveOnly);

        visionIndex++;
    }
}

void DialogITRV::AccessModeChanged()
{
    m_viewStatus->UpdateAccessModeText();

    for (auto vision : m_visions)
    {
        vision->UpdateRibbonMenuState();
    }

    SetCurrentFrameString();
}

void DialogITRV::ShowAccessModeDialog()
{
    if (m_visions.empty())
    {
        return;
    }

    for (auto vision : m_visions)
    {
        if (vision->m_visionMain.m_pMessageSocket->IsConnected())
        {
            // Host와 연결되어 있으면 설정 불가능하다
            return;
        }

        if (vision->m_visionMain.IsShowVisionPrimaryUI() == false)
        {
            // Primary UI 상태가 아니라면 설정 불가능하다
            return;
        }
    }

    // 모든 엑세스 모드 설정은 동일하므로 첫 번째 비전의 엑세스 모드 대화 상자를 표시합니다.
    m_visions.front()->ShowAccessModeDialog();

    AccessModeChanged();
}

void DialogITRV::ShowIllumSetup(int32_t visionIndex)
{
    for (auto index = 0; index < m_visions.size(); index++)
    {
        m_visions[index]->ShowWindow(m_visions[index]->m_visionIndex == visionIndex ? SW_SHOW : SW_HIDE);
    }

    UpdateLayout();

    if (visionIndex >= 0 && visionIndex < (int32_t)m_visions.size())
    {
        m_visions[visionIndex]->ShowIlluminationTeachUI();
    }
}

void DialogITRV::ShowTeach(int32_t visionIndex)
{
    for (auto index = 0; index < m_visions.size(); index++)
    {
        m_visions[index]->ShowWindow(m_visions[index]->m_visionIndex == visionIndex ? SW_SHOW : SW_HIDE);
    }

    UpdateLayout();

    if (visionIndex >= 0 && visionIndex < (int32_t)m_visions.size())
    {
        m_visions[visionIndex]->ShowTeach();
    }
}

void DialogITRV::ShowBatchInspection(int32_t visionIndex)
{
    for (auto index = 0; index < m_visions.size(); index++)
    {
        m_visions[index]->ShowWindow(m_visions[index]->m_visionIndex == visionIndex ? SW_SHOW : SW_HIDE);
    }

    UpdateLayout();

    if (visionIndex >= 0 && visionIndex < (int32_t)m_visions.size())
    {
        m_visions[visionIndex]->ShowBatchInspection();
    }
}

void DialogITRV::SetCurrentFrameString()
{
    auto& system = SystemConfig::GetInstance();

    CString strAccessMode;
    CString strOut;

    if (system.m_nCurrentAccessMode == _OPERATOR)
        strAccessMode.Format(_T("[Operator Mode]"));
    else if (system.m_nCurrentAccessMode == _ENGINEER)
        strAccessMode.Format(_T("[Engineer Mode]"));
    else if (system.m_nCurrentAccessMode == _INTEKPLUS)
        strAccessMode.Format(_T("[Intekplus Mode]"));

    //{{ //kircheis_TRV //kircheis_100M
    static const long nVisionType = system.m_nVisionType;
    static const long nSideVisionID = (long)(system.GetSideVisionNumber());

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
    }

    strOut.Format(_T("%s %s"), (LPCTSTR)strITRV, (LPCTSTR)strAccessMode);

    SetWindowText(strOut);
}

void DialogITRV::UpdateLayout()
{
    CRect windowRect;
    GetClientRect(&windowRect);
    auto cx = windowRect.Width();
    auto cy = windowRect.Height();

    auto statusHeight = 30;
    auto visibledVisionCount = 0;
    for (auto vision : m_visions)
    {
        if (vision->IsWindowVisible())
        {
            visibledVisionCount++;
        }
    }

    auto visionViewWidth = visibledVisionCount ? cx / visibledVisionCount : 0;
    auto x = 0;

    for (auto vision : m_visions)
    {
        if (!vision->IsWindowVisible())
        {
            continue;
        }
        vision->MoveWindow(x, 0, visionViewWidth, cy - statusHeight, TRUE);
        x += visionViewWidth;
    }

    m_viewStatus->MoveWindow(0, cy - statusHeight, cx, statusHeight, TRUE);
}

void DialogITRV::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DialogITRV, CDialogEx)
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_WM_SIZE()
END_MESSAGE_MAP()

// DialogITRV 메시지 처리기

BOOL DialogITRV::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
    //  프레임워크가 이 작업을 자동으로 수행합니다.
    SetIcon(m_hIcon, TRUE); // 큰 아이콘을 설정합니다.
    SetIcon(m_hIcon, FALSE); // 작은 아이콘을 설정합니다.

    if (SystemConfig::GetInstance().m_tr.m_executeMode == TR_EXECUTE_ALL
        || SystemConfig::GetInstance().m_tr.m_executeMode == TR_EXECUTE_IN_POCKET)
    {
        m_visions.push_back(std::make_shared<ViewVision>(this, TR_EXECUTE_IN_POCKET, m_Flag_thread_Inline_INPOCKET, m_Flag_thread_SyncStart_INPOCKET));
    }
    if (SystemConfig::GetInstance().m_tr.m_executeMode == TR_EXECUTE_ALL
        || SystemConfig::GetInstance().m_tr.m_executeMode == TR_EXECUTE_OTI)
    {
        m_visions.push_back(std::make_shared<ViewVision>(this, TR_EXECUTE_OTI, m_Flag_thread_Inline_OTI, m_Flag_thread_SyncStart_OTI));
    }

    for (auto vision : m_visions)
    {
        if (!vision->Create(IDD_VIEW_VISION, this))
        {
            AfxMessageBox(_T("Failed to create ViewVision dialog!"));
            return FALSE; // 대화 상자 생성 실패
        }
        vision->ShowWindow(SW_SHOW);
    }

    m_viewStatus = std::make_unique<ViewStatus>(this);
    m_viewStatus->Create(IDD_VIEW_STATUS, this);
    m_viewStatus->ShowWindow(SW_SHOW);

    ShowWindow(SW_MAXIMIZE);
    UpdateLayout();
    SetCurrentFrameString();

    return TRUE; // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void DialogITRV::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 아이콘을 그립니다.
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR DialogITRV::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void DialogITRV::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_viewStatus != nullptr)
    {
        UpdateLayout();
    }
}