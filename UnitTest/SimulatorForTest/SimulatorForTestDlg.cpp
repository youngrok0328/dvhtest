//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SimulatorForTestDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/ArchiveAllType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/BinaryFileData.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../SharedCommunicationModules/VisionHostCommon/iPIS_MsgDefinitions.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_SOCKET_CONNECTION (WM_USER + 100)
#define UM_SOCKET_RECEIVED (WM_USER + 102)

//CPP_7_________________________________ Implementation body
//
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_ABOUTBOX
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation

protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg()
    : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CSimulatorForTestDlg dialog

CSimulatorForTestDlg::CSimulatorForTestDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_SIMULATORFORTEST_DIALOG, pParent)
    , m_socket(nullptr)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CSimulatorForTestDlg::~CSimulatorForTestDlg()
{
    delete m_socket;
}

void CSimulatorForTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSimulatorForTestDlg, CDialogEx)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_BTN_CONNECT, &CSimulatorForTestDlg::OnBnClickedBtnConnect)
ON_BN_CLICKED(IDC_BTN_DISCONNECT, &CSimulatorForTestDlg::OnBnClickedBtnDisconnect)
ON_MESSAGE(UM_SOCKET_CONNECTION, &CSimulatorForTestDlg::OnSocketConnection)
ON_MESSAGE(UM_SOCKET_RECEIVED, &CSimulatorForTestDlg::OnSocketReceived)
ON_BN_CLICKED(IDC_BTN_SET_INLINE_START, &CSimulatorForTestDlg::OnBnClickedBtnSetInlineStart)
ON_BN_CLICKED(IDC_BTN_SET_INLINE_STOP, &CSimulatorForTestDlg::OnBnClickedBtnSetInlineStop)
ON_BN_CLICKED(IDC_BTN_SEND_TEST_JOB, &CSimulatorForTestDlg::OnBnClickedBtnSendTestJob)
END_MESSAGE_MAP()

// CSimulatorForTestDlg message handlers

BOOL CSimulatorForTestDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE); // Set big icon
    SetIcon(m_hIcon, FALSE); // Set small icon

    // TODO: Add extra initialization here

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CSimulatorForTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSimulatorForTestDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSimulatorForTestDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CSimulatorForTestDlg::OnBnClickedBtnConnect()
{
    delete m_socket;
    Ipvm::SocketMessaging::INIT_INFO info = {0, 0, GetSafeHwnd(), UM_SOCKET_CONNECTION, UM_SOCKET_RECEIVED};
    m_socket = new Ipvm::SocketMessaging(info);
    m_socket->Connect(_T("127.0.0.1"), 5000, 100);
}

void CSimulatorForTestDlg::OnBnClickedBtnDisconnect()
{
    delete m_socket;
    m_socket = nullptr;
}

LRESULT CSimulatorForTestDlg::OnSocketConnection(WPARAM wParam, LPARAM lParam)
{
    const auto evtInfo = Ipvm::SocketMessaging::ParseEvt(wParam, lParam);

    if (evtInfo.m_connected)
    {
        GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_DISCONNECT)->EnableWindow(TRUE);
    }
    if (evtInfo.m_disconnected)
    {
        GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_DISCONNECT)->EnableWindow(FALSE);

        delete m_socket;
        m_socket = nullptr;
    }

    return 0;
}

LRESULT CSimulatorForTestDlg::OnSocketReceived(WPARAM wParam, LPARAM lParam)
{
    const auto msgInfo = Ipvm::SocketMessaging::ParseMsg(wParam, lParam);

    const long nMessageID = msgInfo.m_messageIndex;
    const long nMessageLength = msgInfo.m_messageLength;
    const BYTE* pData = (const BYTE*)msgInfo.m_message;

    switch (nMessageID)
    {
        case MSG_PROBE_TEACH_END:
            if (true)
            {
                long nSize;

                CMemFile memFile;
                memFile.Attach((BYTE*)pData, nMessageLength);
                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);

                BinaryFileData binary;
                binary.SerializeForComm(ar);
                binary.Save(_T("C:\\IntekPlus\\iPIS-500I\\Test\\Received.vmjob"));
                ar >> nSize;

                VisionInspectionSpec* visionInspSpecs = new VisionInspectionSpec[nSize];
                for (int n = 0; n < nSize; n++)
                {
                    ar >> visionInspSpecs[n];
                }

                long ballNumber = 0;
                long landNumber = 0;
                long passiveNumber = 0;
                ar >> ballNumber;
                ar >> landNumber;
                ar >> passiveNumber;

                float fSize;
                ar >> fSize;
                float fDeviceSizeX = fSize / 1000; // 비전에서 um로 보낸다.
                ar >> fSize;
                float fDeviceSizeY = fSize / 1000; // 비전에서 um로 보낸다.
                ar >> fSize;
                float fDeviceThickness = fSize / 1000; // 비전에서 um로 보낸다.
                BOOL bNeedMarkTeach;
                BOOL bUseHeightConversioInspection;
                ar >> bNeedMarkTeach;
                ar >> bUseHeightConversioInspection;

                ar.Close();

                delete[] visionInspSpecs;
            }
            break;
    }

    return 0;
}

void CSimulatorForTestDlg::OnBnClickedBtnSetInlineStart()
{
    if (!m_socket->IsConnected())
        return;

    m_socket->Write(MSG_PROBE_INLINE_START, 0, nullptr);
}

void CSimulatorForTestDlg::OnBnClickedBtnSetInlineStop()
{
    if (!m_socket->IsConnected())
        return;

    m_socket->Write(MSG_PROBE_INLINE_STOP, 0, nullptr);
}

void CSimulatorForTestDlg::OnBnClickedBtnSendTestJob()
{
    CString fileName = _T("Test.vmjob");
    CString filePath = _T("C:\\IntekPlus\\iPIS-500I\\Test\\Test.vmjob");

    BinaryFileData binaryData;
    if (!binaryData.Load(filePath))
    {
        MessageBox(_T("Test Job file is not found"));
        return;
    }

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    double scanLength_mm = 0.; // 만들어줘야 하지만 테스트니까 빈깡통을 보내자

    ar << fileName;
    ar << scanLength_mm;

    binaryData.SerializeForComm(ar);

    const BOOL bSendVisionPara = FALSE;
    ar << bSendVisionPara;

    ar.Close();

    long length = ((CFile*)&memFile)->GetLength();
    BYTE* pByte = memFile.Detach();

    m_socket->Write(MSG_PROBE_JOB_DOWNLOAD, length, pByte);

    free(pByte);
}
