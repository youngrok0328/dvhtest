//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionPrimaryUI.h"

//CPP_2_________________________________ This project's headers
#include "DlgLogin.h"
#include "DlgVisionPrimaryUI.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../ManagementModules/VisionMain/VisionMain.h"
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CVisionPrimaryUI::CVisionPrimaryUI(VisionMainAgent& visionMainAgent, VisionUnit& visionUnit)
    : m_visionMainAgent(visionMainAgent)
    , m_visionUnit(visionUnit)
{
    m_pDlg = NULL;
}

CVisionPrimaryUI::~CVisionPrimaryUI(void)
{
    if (m_pDlg->GetSafeHwnd())
    {
        m_pDlg->DestroyWindow();
    }

    delete m_pDlg;
}

void CVisionPrimaryUI::OnMessageSocketConnectionUpdated(const Ipvm::SocketMessaging::EVT_INFO& evtInfo)
{
    m_pDlg->OnMessageSocketConnectionUpdated(evtInfo);
}

void CVisionPrimaryUI::OnMessageSocketDataReceived(const Ipvm::SocketMessaging::MSG_INFO& msgInfo)
{
    m_pDlg->OnMessageSocketDataReceived(msgInfo);
}

void CVisionPrimaryUI::SetMarkTeachFlag(BOOL bMarkTeachFlag)
{
    m_pDlg->m_bMarkTeachFlag = bMarkTeachFlag;
}

BOOL CVisionPrimaryUI::GetMarkTeachFlag()
{
    return m_pDlg->m_bMarkTeachFlag;
}

BOOL CVisionPrimaryUI::OnFileImageOpen()
{
    return m_pDlg->ImageFileOpen();
}

void CVisionPrimaryUI::OnJobChanged()
{
    m_pDlg->OnJobChanged();
}

void CVisionPrimaryUI::ShowVisionPrimaryDlg(HWND hwndParent)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pDlg->GetSafeHwnd())
    {
        m_pDlg->ShowWindow(SW_SHOW);
    }
    else
    {
        CWnd* parentWnd = CWnd::FromHandle(hwndParent);

        m_pDlg = new CDlgVisionPrimaryUI(m_visionMainAgent, m_visionUnit, parentWnd);
        m_pDlg->Create(CDlgVisionPrimaryUI::IDD, parentWnd);
    }
}

void CVisionPrimaryUI::HideVisionPrimaryDlg()
{
    if (m_pDlg->GetSafeHwnd())
    {
        m_pDlg->ShowWindow(SW_HIDE);
    }
}

bool CVisionPrimaryUI::IsWindowVisible()
{
    if (m_pDlg->GetSafeHwnd())
    {
        return m_pDlg->IsWindowVisible() ? true : false;
    }

    return false;
}

bool CVisionPrimaryUI::IsJobTeachEnabled()
{
    if (m_pDlg)
    {
        return m_pDlg->m_bJobTeachEnable;
    }

    return false;
}

void CVisionPrimaryUI::ShowCurrentImage(bool showRaw)
{
    m_pDlg->ShowCurrentImage(showRaw);
}

void CVisionPrimaryUI::OnInspect()
{
    m_pDlg->OnBnClickedButtonInspect();
}

void CVisionPrimaryUI::OnLogin()
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("Show Login"));

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CDlgLogin Dlg;
    Dlg.SetAccessMode(m_visionUnit.m_systemConfig.m_nCurrentAccessMode);

    if (Dlg.DoModal() == IDOK)
    {
        m_visionUnit.m_systemConfig.m_nCurrentAccessMode = Dlg.m_nMode;

        if (m_visionUnit.m_systemConfig.m_nCurrentAccessMode == _OPERATOR)
            m_pDlg->m_Label_AccessMode2.SetText(_T("Operator"));
        else if (m_visionUnit.m_systemConfig.m_nCurrentAccessMode == _ENGINEER)
            m_pDlg->m_Label_AccessMode2.SetText(_T("Engineer"));
        else if (m_visionUnit.m_systemConfig.m_nCurrentAccessMode == _INTEKPLUS)
            m_pDlg->m_Label_AccessMode2.SetText(_T("Intekplus"));

        // 영훈 20141124_LoginMode : 현재 구성된 Login Mode를 프로그램이 종료되어도 지속되도록 한다.  계속 로그인 하는 것도 귀찮아서 개발자 모드로 만든다 ㅡ.,ㅡ
        // Login DLG에서 OK버튼을 누를때 Shift랑 Ctrl 버튼을 함께 누르면 영구 저장된다.
        if (Dlg.m_bSystemSaveMode)
        {
            m_visionUnit.m_systemConfig.SaveIni(DynamicSystemPath::get(DefineFile::System));
        }
    }
}

void CVisionPrimaryUI::SetLiveMain()
{
    m_pDlg->SetLiveMain();
}

void CVisionPrimaryUI::ResetLiveMain()
{
    m_pDlg->ResetLiveMain();
}

void CVisionPrimaryUI::ReviewImageViewer()
{
    m_pDlg->ReviewImageViewer();
}

void CVisionPrimaryUI::SystemSetup()
{
    m_pDlg->SystemSetup();
}

// 영훈 20150720 : pDlg는 이제 밖에서 사용하지 못한다.
HWND CVisionPrimaryUI::Dlg_GetsafeHwnd()
{
    return m_pDlg->GetSafeHwnd();
}

BOOL CVisionPrimaryUI::Dlg_GetMarkSipMode()
{
    return m_pDlg->m_bMarkSkipMode;
}

void CVisionPrimaryUI::Dlg_SetMarkSipMode(BOOL bData)
{
    m_pDlg->m_bMarkSkipMode = bData;
}

// 영훈 20160802 : Release Version과 Test Version을 화면에 표시해주도록 한다.
void CVisionPrimaryUI::Dlg_SetLabel_Version2(CString strFileVersion)
{
    CString strR = strFileVersion.Right(1);
    long n4thHeadNum = 4;
    strR.MakeUpper();
    if (strR == _T("R") || strR == _T("D"))
        n4thHeadNum = 5;
    CString strFileVersion_Temp = strFileVersion.Right(n4thHeadNum);
    TCHAR cVersion0 = strFileVersion_Temp.GetAt(0);

    strFileVersion_Temp = strFileVersion;

    if (cVersion0 == _T('8') || cVersion0 == _T('9'))
    {
        long nStrCount = strFileVersion_Temp.GetLength();

        if (n4thHeadNum == 5)
            strFileVersion_Temp.Delete(nStrCount - 1, 1);

        strFileVersion_Temp += _T(" Test Version");
    }
    else
    {
        long nStrCount = strFileVersion_Temp.GetLength();

        if (n4thHeadNum == 5)
            strFileVersion_Temp.Delete(nStrCount - 1, 1);
        strFileVersion_Temp += _T(" Release Version");
    }

    m_pDlg->m_Label_Version2.SetText(strFileVersion_Temp);
    m_pDlg->m_strVersion = strFileVersion;
}

void CVisionPrimaryUI::SetMarkTeachTotalResultView(const Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtViewArea,
    long i_nMarkCharNum, Ipvm::Rect32s* i_prtEachCharROI) //kircheis_201611XX //TeachResultView
{
    m_pDlg->SetMarkTeachTotalResultView(image, i_rtViewArea, i_nMarkCharNum, i_prtEachCharROI);
}

void CVisionPrimaryUI::HideMarkTeachTotalResultView() //kircheis_201611XX //TeachResultView
{
    m_pDlg->HideMarkTeachTotalResultView();
}

void CVisionPrimaryUI::SetPrimaryUISideVisionSection() // VisionPrimaryUI SideVision Selector - JHB_2024.04.15
{
    m_pDlg->SetPrimaryUISideVisionSection();
}
