//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "iPackView.h"

//CPP_2_________________________________ This project's headers
#include "FileVersion.h"
#include "MainFrm.h"
#include "iPackDoc.h"

//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../SharedComponent/CrashHandler/CrashHandler.h"
#include "../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
extern CMainFrame* g_mainFrame;
#pragma comment(lib, "version.lib")

IMPLEMENT_DYNCREATE(CiPackView, CFormView)

BEGIN_MESSAGE_MAP(CiPackView, CFormView)
ON_WM_CREATE()
ON_WM_SIZE()
END_MESSAGE_MAP()

// CiPackView 생성/소멸

CiPackView::CiPackView()
    : CFormView(CiPackView::IDD)
{
#ifndef _DEBUG
    CrashHandler::Register(false, CRASH_DUMP_DIRECTORY, _T("MiniDump"), this, CiPackView::UserExceptionHandlerPre,
        CiPackView::UserExceptionHandlerPost);
#endif
}

CiPackView::~CiPackView()
{
}

void CiPackView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}

BOOL CiPackView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: CREATESTRUCT cs를 수정하여 여기에서
    //  Window 클래스 또는 스타일을 수정합니다.

    return CFormView::PreCreateWindow(cs);
}

void CiPackView::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();

    GetParentFrame()->RecalcLayout();

    CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(::AfxGetMainWnd());

    pMainFrame->m_visionMain.InitializeVisionInlineUI(GetSafeHwnd());
    pMainFrame->m_visionMain.ShowVisionPrimaryUI(GetSafeHwnd());

    // 영훈 20131226 :: 파일의 버전을 받아오도록 한다.
    CString strExe = _T("iPack.exe");

    // 영훈 20160802 : Main 화면에 표시횔 Version 부분을 세팅한다.
    FileVersion cFileVer;
    cFileVer.Open(strExe);
    m_strAppVersion = cFileVer.GetFileVersion();
    m_strAppVersion.Remove(' ');
    m_strAppVersion.Replace(',', '.');
    m_strAppDetailVersion = cFileVer.GetFileVersion2();

    // 영훈 20160802 : 화면에 표시하기
    pMainFrame->m_visionMain.SetFileVersion(m_strAppDetailVersion);

    pMainFrame->m_visionMain.OpenJobFile(pMainFrame->m_visionMain.GetPrevJob(), _T(""));

    // 영훈 20160802 : iPack title에 현재 AccessMode를 확인할 수 있도록 한다.
    pMainFrame->SetCurrentFrameString();

    CRect rtClient;
    GetClientRect(rtClient);

    OnSize(0, rtClient.Width(), rtClient.Height());

    // 여기서부터 소켓을 사용할 수 있게 한다.
    pMainFrame->InitializeMessageSocket();
}

// CiPackView 진단

#ifdef _DEBUG
void CiPackView::AssertValid() const
{
    CFormView::AssertValid();
}

void CiPackView::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}

CiPackDoc* CiPackView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CiPackDoc)));
    return (CiPackDoc*)m_pDocument;
}
#endif //_DEBUG

void CiPackView::UserExceptionHandlerPre(void* callbackData)
{
    UNREFERENCED_PARAMETER(callbackData);

    SystemConfig::GetInstance().SetVisionCondition(eVisionCondition::VisionCondition_Wrong);
}

void CiPackView::UserExceptionHandlerPost(
    void* callbackData, bool success, const wchar_t* dumpPathName, const wchar_t* message)
{
    UNREFERENCED_PARAMETER(callbackData);
    UNREFERENCED_PARAMETER(success);
    UNREFERENCED_PARAMETER(message);

    CMainFrame* pMainFrame = g_mainFrame;

    //VisionCondition을 알려준다
    SystemConfig::GetInstance().SetVisionCondition(eVisionCondition::VisionCondition_Wrong);

    //MSG전송으로 V->H이 알려준다..
    pMainFrame->m_visionMain.iPIS_Send_ErrorMessageForPopup(_T("Crash, Please Check Vision"));

    CTime curTime(CTime::GetCurrentTime());

    CString strFileVersion = pMainFrame->m_visionMain.GetFileVersion();

    CString strFolderName;
    strFolderName.Format(_T("%s%s_%04d-%02d-%02d-%02d-%02d"), CRASH_DUMP_DIRECTORY, (LPCTSTR)strFileVersion,
        curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), curTime.GetHour(), curTime.GetMinute());
    ::CreateDirectory(strFolderName, NULL);

    // 생성된 파일을 파일 버전이름으로 변경
    CString strBackupCrashDump;
    strBackupCrashDump.Format(_T("%s\\%s_CrashDump.dmp"), (LPCTSTR)strFolderName, (LPCTSTR)strFileVersion);
    ::MoveFile(dumpPathName, LPCTSTR(strBackupCrashDump));

    // 이미지 저장
    pMainFrame->m_visionMain.SaveDumpImageFiles(strFolderName, strFileVersion);

    // Job 저장
    CString strBackupJob;
    strBackupJob.Format(_T("%s\\%s_Job.vmjob"), (LPCTSTR)strFolderName, (LPCTSTR)strFileVersion);
    pMainFrame->m_visionMain.SaveJobFile(strBackupJob, VisionMainAgent::SaveJobMode::SaveOnly);

    // System 파일 저장
    CString strBackupSystem;
    strBackupSystem.Format(_T("%s\\system.ini"), (LPCTSTR)strFolderName);
    ::CopyFile(DynamicSystemPath::get(DefineFile::System), strBackupSystem, FALSE);
}

int CiPackView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFormView::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CiPackView::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);

    CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(::AfxGetMainWnd());

    if (pMainFrame == nullptr)
    {
        return;
    }

    // 여기서 Thread UI 를 뿌려준다...
    const CRect rtInlineArea(5, 5, cx - 5, cy - 5);

    static const long nThreadNum
        = pMainFrame->m_visionMain.GetThreadNum(); //이하 4개의 변수는 S/W 실행 시 변경 안되는 녀석들이라  static 처리
    static const bool bIsSingleLine
        = (nThreadNum <= 3); //일단은 Apple에서 Thread 개수를 8개까지만 입력 가능하니까 2행까지 만 가능한걸로
    static const long nRowCount = bIsSingleLine ? 1 : 2; // Thread 3개까지는 1줄, 4개부터는 2줄
    static const long nColCount = bIsSingleLine
        ? nThreadNum
        : (long)((float)nThreadNum * .5f + .5f); //nTN <=3 : 1 // nTN==4 : 2 // nTN==5or6 : 3 // nTN==7or8 : 4
    long nCol = 0;
    long nRow = 0;

    for (long nThread = 0; nThread < nThreadNum; nThread++)
    {
        CWnd* pDlg = pMainFrame->m_visionMain.GetVisionInlineDialog(nThread);

        nCol = nThread % nColCount;
        nRow = (nThread == nCol)
            ? 0
            : 1; // nThread를 nColCount로 나눈 후 나머지가 nThread와 같다는 건 몫이 0이란 이야기.. 즉, 첫 줄이다. 나눗셈 연산하기 싫어서 3항 연산처리

        if (pDlg->GetSafeHwnd())
        {
            //pDlg->MoveWindow(::GetContentSubArea(rtInlineArea, nThread, nThread + 1, nThreadNum, 0, 1, 1), TRUE);//원래 1줄 배열 Code
            pDlg->MoveWindow(
                ::GetContentSubArea(rtInlineArea, nCol, nCol + 1, nColCount, nRow, nRow + 1, nRowCount), TRUE);
        }
    }
}

BOOL CiPackView::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if (::GetKeyState(VK_CONTROL) < 0 && pMsg->wParam == 'C')
        {
            pMsg->message = WM_COPY;
        }
    }

    return CFormView::PreTranslateMessage(pMsg);
}
