//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionCommonLogDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_UPDATE_LOG (WM_USER + 100)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(VisionCommonLogDlg, CDialog)

VisionCommonLogDlg::VisionCommonLogDlg(CWnd* pParent, const CRect& rtPositionOnParent)
    : CDialog(IDD_DIALOG_TEXT_LOG, pParent)
    , m_rtInitPosition(rtPositionOnParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    AfxInitRichEdit();

    VERIFY(Create(IDD_DIALOG_TEXT_LOG, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

VisionCommonLogDlg::~VisionCommonLogDlg()
{
}

void VisionCommonLogDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHECK_Warning, m_chkWarning);
    DDX_Control(pDX, IDC_CHECK_NOTICE, m_chkNotice);
    DDX_Control(pDX, IDC_CHECK_Event, m_chkEvent);
    DDX_Control(pDX, IDC_CHECK_Thread, m_chkThread);
    DDX_Control(pDX, IDC_CHECK_TCP, m_chkTCP);
    DDX_Control(pDX, IDC_CHECK_DEVELOP, m_chkDevelop);
    DDX_Control(pDX, IDC_LIST_LOG, m_logBox);
}

BEGIN_MESSAGE_MAP(VisionCommonLogDlg, CDialog)
ON_WM_SIZE()
ON_BN_CLICKED(IDC_CHECK_Warning, &VisionCommonLogDlg::OnBnClickedCheckWarning)
ON_BN_CLICKED(IDC_CHECK_Event, &VisionCommonLogDlg::OnBnClickedCheckEvent)
ON_BN_CLICKED(IDC_CHECK_Thread, &VisionCommonLogDlg::OnBnClickedCheckThread)
ON_BN_CLICKED(IDC_CHECK_TCP, &VisionCommonLogDlg::OnBnClickedCheckTcp)
ON_BN_CLICKED(IDC_CHECK_DEVELOP, &VisionCommonLogDlg::OnBnClickedCheckDevelop)
ON_BN_CLICKED(IDC_CHECK_NOTICE, &VisionCommonLogDlg::OnBnClickedCheckNotice)
ON_MESSAGE(UM_UPDATE_LOG, &VisionCommonLogDlg::OnUpdateLog)
END_MESSAGE_MAP()

// VisionCommonLogDlg 메시지 처리기입니다.

BOOL VisionCommonLogDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_chkWarning.SetCheck(DevelopmentLog::IsTypeEnable(DevelopmentLog::Type::Warning));
    m_chkEvent.SetCheck(DevelopmentLog::IsTypeEnable(DevelopmentLog::Type::Event));
    m_chkThread.SetCheck(DevelopmentLog::IsTypeEnable(DevelopmentLog::Type::Thread));
    m_chkTCP.SetCheck(DevelopmentLog::IsTypeEnable(DevelopmentLog::Type::TCP));
    m_chkDevelop.SetCheck(DevelopmentLog::IsTypeEnable(DevelopmentLog::Type::Develop));
    m_chkNotice.SetCheck(DevelopmentLog::IsTypeEnable(DevelopmentLog::Type::Notice));

    m_chkDevelop.ShowWindow(IPIS500I_BUILD_NUMBER == IPIS500I_DEVELOPER_VERSION_BUILD_NUMBER ? SW_SHOW : SW_HIDE);

    DevelopmentLog::SetMonitoringHwnd(GetSafeHwnd(), UM_UPDATE_LOG);

    MoveWindow(m_rtInitPosition, FALSE);

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void VisionCommonLogDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (m_logBox.GetSafeHwnd())
    {
        CRect oldPos;
        m_logBox.GetWindowRect(oldPos);
        ScreenToClient(oldPos);

        m_logBox.MoveWindow(0, oldPos.top, cx, cy - oldPos.top);
    }
}

void VisionCommonLogDlg::OnBnClickedCheckWarning()
{
    DevelopmentLog::SetTypeEnabled(DevelopmentLog::Type::Warning, m_chkWarning.GetCheck());
}

void VisionCommonLogDlg::OnBnClickedCheckEvent()
{
    DevelopmentLog::SetTypeEnabled(DevelopmentLog::Type::Event, m_chkEvent.GetCheck());
}

void VisionCommonLogDlg::OnBnClickedCheckThread()
{
    DevelopmentLog::SetTypeEnabled(DevelopmentLog::Type::Thread, m_chkThread.GetCheck());
}

void VisionCommonLogDlg::OnBnClickedCheckTcp()
{
    DevelopmentLog::SetTypeEnabled(DevelopmentLog::Type::TCP, m_chkTCP.GetCheck());
}

void VisionCommonLogDlg::OnBnClickedCheckDevelop()
{
    DevelopmentLog::SetTypeEnabled(DevelopmentLog::Type::Develop, m_chkDevelop.GetCheck());
}

void VisionCommonLogDlg::OnBnClickedCheckNotice()
{
    DevelopmentLog::SetTypeEnabled(DevelopmentLog::Type::Notice, m_chkNotice.GetCheck());
}

LRESULT VisionCommonLogDlg::OnUpdateLog(WPARAM, LPARAM)
{
    std::vector<std::pair<DevelopmentLog::Type, CString>> logList;
    DevelopmentLog::PopLog(logList);

    CString tempText;
    bool autoMoveForWarning = false;

    if (logList.size())
    {
        bool autoScroll = (m_logBox.GetCount() - 1 == m_logBox.GetCurSel());

        for (const auto& logLine : logList)
        {
            switch (logLine.first)
            {
                case DevelopmentLog::Type::Warning:
                    tempText.Format(_T("[Warning] %s"), LPCTSTR(logLine.second));
                    m_logBox.InsertString(m_logBox.GetCount(), tempText);
                    autoMoveForWarning = true;
                    break;
                case DevelopmentLog::Type::Event:
                    tempText.Format(_T("[Event] %s"), LPCTSTR(logLine.second));
                    m_logBox.InsertString(m_logBox.GetCount(), tempText);
                    break;
                case DevelopmentLog::Type::Thread:
                    tempText.Format(_T("[Thread] %s"), LPCTSTR(logLine.second));
                    m_logBox.InsertString(m_logBox.GetCount(), tempText);
                    break;
                case DevelopmentLog::Type::TCP:
                    tempText.Format(_T("[TCP] %s"), LPCTSTR(logLine.second));
                    m_logBox.InsertString(m_logBox.GetCount(), tempText);
                    break;
                case DevelopmentLog::Type::Develop:
                    tempText.Format(_T("[DEV] %s"), LPCTSTR(logLine.second));
                    TRACE(tempText);
                    m_logBox.InsertString(m_logBox.GetCount(), tempText);
                    break;
                case DevelopmentLog::Type::Notice:
                    tempText.Format(_T("[Notice] %s"), LPCTSTR(logLine.second));
                    m_logBox.InsertString(m_logBox.GetCount(), tempText);
                    break;
                default:
                    m_logBox.InsertString(m_logBox.GetCount(), logLine.second);
                    break;
            }

            if (m_logBox.GetCount() > 2000)
            {
                m_logBox.DeleteString(0);
            }
        }

        if (autoScroll)
        {
            m_logBox.SetCurSel(m_logBox.GetCount() - 1);
        }
    }

    if (autoMoveForWarning)
    {
        GetParent()->PostMessage(UM_ADD_LOG_WARNING);
    }

    return 0;
}
