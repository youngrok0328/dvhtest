//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "iTRV.h"

//CPP_2_________________________________ This project's headers
#include "DialogITRV.h"
#include "LicenseChecker.h"

//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../HardwareModules/dPI_SyncController/SlitIlluminator.h"
#include "../HardwareModules/dPI_SyncController/SyncController.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../SharedComponent/CrashHandler/CrashHandler.h"
#include "../UserInterfaceModules/ImageLotView/ImageLotView.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/Miscellaneous.h>
#include <afxmt.h>
#include <afxwinappex.h>
#include <gdiplus.h>
#include <mmsystem.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#if (IPVMLIB_MAJOR_VERSION == 9) && (IPVMLIB_MINOR_VERSION == 0) && (IPVMLIB_REVISION == 1)
#else
#error "VisionMadang SDK Version Mismatch. Download the right version."
#endif

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "gdiplus.lib")

BEGIN_MESSAGE_MAP(CiTRVApp, CWinApp)
ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CiTRVApp 생성

CiTRVApp::CiTRVApp()
{
    static CMutex mutex(FALSE, _T("iTRV singleton"), nullptr);

    if (mutex.Lock(0) == FALSE)
    {
        ::AfxMessageBox(_T("iTRV.exe is running already!!"), MB_ICONWARNING | MB_OK);
        ::exit(0);
    }

    mutex.Lock();

    // 아래 구문이 없으면 CMFCRibbonInfo::Read() 내에서 크래시되는 경우가 있음. 이현민.
    ::AfxOleInit();

    TCHAR modulePath[MAX_PATH];
    GetModuleFileName(NULL, modulePath, MAX_PATH);
    DynamicSystemPath::setModulePath(modulePath);

    Ipvm::CreateDirectories(DynamicSystemPath::get(DefineFolder::Root));
    Ipvm::CreateDirectories(CRASH_DUMP_DIRECTORY);

    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::Binary), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::Config), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::System), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::Job), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::Job_ChipAlgo), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::Job_PadAlignAlgo), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::Log), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::TimeLog), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::Pacakge_ViewerData), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::MarkImage), nullptr);
    ::CreateDirectory(DynamicSystemPath::get(DefineFolder::Temp), nullptr);
    ::CreateDirectory(REVIEW_IMAGE_DIRECTORY, nullptr);
    ::CreateDirectory(RAW_IMAGE_DIRECTORY, nullptr);

    // 하드웨어 관련된 싱글톤 생성.
    SystemConfig::GetInstance();

    if (SystemConfig::GetInstance().IsVisionTypeTR() == false)
    {
        CString visionType = SystemConfig::GetInstance().GetVisionTypeText();
        CString errorMessage;
        errorMessage.Format(_T("This system option is not for [TR Vision] Type.\nCurrent type is the [%s].\nPlease ")
                            _T("check the system parameter."),
            visionType);
        ::AfxMessageBox(errorMessage, MB_ICONWARNING | MB_OK);

        ::exit(0);
    }

    if (!SystemConfig::GetInstance().m_bHardwareExist && ::GetKeyState(VK_CONTROL) >= 0 && ::GetKeyState(VK_SHIFT) >= 0)
    {
        // 프로그램을 다른 사함들 PC에 무분별하게 설치해주는 것을 막고 싶어 하셔서
        // Nohardware 일때는 일단 빌드후 최대 90일간만 사용할 수 있도록 하였다.
        // 추후 제대로된 라이센스를 체크 방법을 추가하던지 하자.

        LicenseChecker::check();
    }

    SystemConfig::GetInstance().Set_TimeLog_SaveFolderPath(DynamicSystemPath::get(DefineFolder::TimeLog));

    SyncController::GetInstance();
    SlitIlluminator::GetInstance();
    FrameGrabber::GetInstance();

    ::timeBeginPeriod(1);

#ifndef _DEBUG
    CrashHandler::Register(false, CRASH_DUMP_DIRECTORY, _T("MiniDump"), this, CiTRVApp::UserExceptionHandlerPre,
        CiTRVApp::UserExceptionHandlerPost);
#endif

    Gdiplus::GdiplusStartup(&m_gdiToken, &g_gdiInput, nullptr);

    new BYTE[100]; // 메모리 누수 탐지 체크용. 이현민.
}

// 유일한 CiTRVApp 개체입니다.

CiTRVApp theApp;

// CiTRVApp 초기화

void CiTRVApp::UserExceptionHandlerPre(void* callbackData)
{
    UNREFERENCED_PARAMETER(callbackData);

    SystemConfig::GetInstance().SetVisionCondition(eVisionCondition::VisionCondition_Wrong);
}

void CiTRVApp::UserExceptionHandlerPost(
    void* callbackData, bool success, const wchar_t* dumpPathName, const wchar_t* message)
{
    UNREFERENCED_PARAMETER(callbackData);
    UNREFERENCED_PARAMETER(success);
    UNREFERENCED_PARAMETER(message);

    //VisionCondition을 알려준다
    auto* mainWindow = (DialogITRV*)theApp.m_pMainWnd;
    mainWindow->Crashed(dumpPathName);
}

BOOL CiTRVApp::InitInstance()
{
    // Windows XP에서는 InitCommonControlsEx()를 필요로 합니다.
    // 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
    // InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
    // 이 항목을 설정하십시오.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    AfxEnableControlContainer();

    // 대화 상자에 셸 트리 뷰 또는
    // 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
    CShellManager* pShellManager = new CShellManager;

    // MFC 컨트롤의 테마를 사용하기 위해 "Windows 원형" 비주얼 관리자 활성화
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    // 표준 초기화
    // 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
    // 아래에서 필요 없는 특정 초기화
    // 루틴을 제거해야 합니다.
    // 해당 설정이 저장된 레지스트리 키를 변경하십시오.
    // TODO: 이 문자열을 회사 또는 조직의 이름과 같은
    // 적절한 내용으로 수정해야 합니다.
    SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));

    DialogITRV dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
        //  코드를 배치합니다.
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
        //  코드를 배치합니다.
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
        TRACE(traceAppMsg, 0,
            "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 "
            "없습니다.\n");
    }

    // 위에서 만든 셸 관리자를 삭제합니다.
    if (pShellManager != nullptr)
    {
        delete pShellManager;
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    // 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고 응용 프로그램을 끝낼 수 있도록 FALSE를
    // 반환합니다.
    return FALSE;
}

int CiTRVApp::ExitInstance()
{
    Gdiplus::GdiplusShutdown(m_gdiToken);

    // 리본 인터페이스를 사용하는 경우에 이 함수가 DLL에서 먼저 호출되면 그 안에서 프로세스가 강제로 종료된다.
    // 따라서, 메모리 누수 체크를 진행하지 못하게 된다.
    // 이 문제를 회피하기 위해서, 이 함수를 강제로 호출하여, 프로그램이 정상 종료되게 만든다.
    // 이렇게 처리하면 프로그램이 왜 정상 종료되는지 이유를 잘 모르고 있다.
    // 20250428 이현민.
    CMFCToolBarImages::CleanUp();

    return CWinApp::ExitInstance();
}

// CiPackApp 메시지 처리기

BOOL CiTRVApp::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
    {
        if (pMsg->wParam >= VK_F1 && pMsg->wParam <= VK_F10)
        {
            ImageLotView::PressSystemKey(pMsg->wParam);
            return TRUE;
        }
    }

    return CWinApp::PreTranslateMessage(pMsg);
}
